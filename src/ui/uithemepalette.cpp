#include "include/uithemepalette.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMenuBar>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTabBar>
#include <QTabWidget>
#include <QToolBar>

#include <cmath>

namespace UiThemePalette {

namespace {

constexpr int kAlternateFactor = 108;
constexpr int kButtonFactor = 118;
constexpr int kMidlightFactor = 125;
constexpr int kLightFactor = 150;
constexpr int kMidFactor = 130;
constexpr int kDarkFactor = 160;
constexpr int kShadowFactor = 300;
constexpr double kDarkLightnessThreshold = 0.5;

constexpr double kSrgbLinearThreshold = 0.03928;
constexpr double kSrgbLinearDivisor = 12.92;
constexpr double kSrgbOffset = 0.055;
constexpr double kSrgbScale = 1.055;
constexpr double kSrgbGamma = 2.4;
constexpr double kRedWeight = 0.2126;
constexpr double kGreenWeight = 0.7152;
constexpr double kBlueWeight = 0.0722;
constexpr double kContrastFlare = 0.05;

constexpr double kMinimumTextContrast = 7.0;
constexpr double kMinimumMutedContrast = 3.0;
constexpr double kMutedShare = 0.5;
constexpr double kMinimumHighlightContrast = 1.5;
constexpr double kFallbackSelectionShare = 0.25;
constexpr int kContrastSteps = 40;

constexpr const char* kColorSchemeProperty = "KDE_COLOR_SCHEME_PATH";
constexpr const char* kColorSchemePattern = "ui-theme-*.colors";
const QString kColorSchemeFileName = QStringLiteral("ui-theme-%1.colors");

/// @p color lifted off a dark or pushed down on a light background by @p factor percent, rounded to 8 bits per
/// channel like blend().
QColor towardsContrast(const QColor& color, int factor, bool isDark)
{ return QColor((isDark ? color.lighter(factor) : color.darker(factor)).rgb()); }

/**
 * Color @p share of the way from @p from to @p to; 0.0 returns @p from, 1.0 returns @p to.
 *
 * The result is rounded to 8 bits per channel, the precision the palette is drawn with, so a contrast measured on it
 * still holds on screen.
 */
QColor blend(const QColor& from, const QColor& to, double share)
{
    auto channel = [share](double start, double end) { return start + (end - start) * share; };

    const QColor exact = QColor::fromRgbF(
        channel(from.redF(), to.redF()), channel(from.greenF(), to.greenF()), channel(from.blueF(), to.blueF()));

    return QColor(exact.rgb());
}

/// WCAG 2 relative luminance of @p color, from 0.0 (black) to 1.0 (white).
double relativeLuminance(const QColor& color)
{
    auto linear = [](double channel) {
        return channel <= kSrgbLinearThreshold ? channel / kSrgbLinearDivisor
                                               : std::pow((channel + kSrgbOffset) / kSrgbScale, kSrgbGamma);
    };

    return kRedWeight * linear(color.redF()) + kGreenWeight * linear(color.greenF()) +
           kBlueWeight * linear(color.blueF());
}

/**
 * Pushes @p color towards black or white, whichever stands out more on @p surface, until it reaches @p minimum.
 *
 * Several bundled themes give the editor a deliberately faint base text color, because the code itself is drawn in
 * token colors. Used as UI text that color would be unreadable, so it is corrected as little as possible.
 *
 * @param color Text color to correct; returned unchanged when it already reaches @p minimum.
 * @param surface Background the text is drawn on.
 * @param minimum Required WCAG contrast ratio, e.g. 7.0 (AAA) for normal text.
 * @return The least shifted color that reaches @p minimum, or black/white when even that falls short.
 */
QColor withMinimumContrast(const QColor& color, const QColor& surface, double minimum)
{
    const QColor black(Qt::black);
    const QColor white(Qt::white);
    const QColor extreme = contrastRatio(white, surface) > contrastRatio(black, surface) ? white : black;

    for (int step = 0; step <= kContrastSteps; ++step) {
        const QColor candidate = blend(color, extreme, static_cast<double>(step) / kContrastSteps);
        if (contrastRatio(candidate, surface) >= minimum) {
            return candidate;
        }
    }

    return extreme;
}

/// Sets the text color of every text role, using @p muted for disabled text and placeholders.
void setTextRoles(QPalette& palette, const QColor& text, const QColor& muted)
{
    for (const QPalette::ColorRole role :
        {QPalette::WindowText, QPalette::Text, QPalette::ButtonText, QPalette::ToolTipText, QPalette::BrightText}) {
        palette.setColor(QPalette::Active, role, text);
        palette.setColor(QPalette::Inactive, role, text);
        palette.setColor(QPalette::Disabled, role, muted);
    }
    palette.setColor(QPalette::PlaceholderText, muted);
}

/**
 * Sets the selection, accent and link roles from the theme instead of leaving the desktop's accent color in place.
 *
 * @param palette Palette whose window and base colors are already set.
 * @param colors Theme colors; a missing selection is derived from text and background, a missing link uses @p text.
 * @param text Text color already corrected for contrast.
 */
void setSelectionAndLinkRoles(QPalette& palette, const EditorColors& colors, const QColor& text)
{
    const QColor& background = colors.background;
    const QColor selection =
        colors.selection.isValid() ? colors.selection : blend(background, text, kFallbackSelectionShare);
    QColor highlight = withMinimumContrast(selection, background, kMinimumHighlightContrast);
    const QColor highlightedText = withMinimumContrast(text, highlight, kMinimumTextContrast);
    if (contrastRatio(highlightedText, highlight) < kMinimumTextContrast) {
        // On a mid-tone selection not even black or white text reaches the minimum, so the selection gives way.
        highlight = withMinimumContrast(highlight, highlightedText, kMinimumTextContrast);
    }
    const QColor link =
        withMinimumContrast(colors.link.isValid() ? colors.link : text, background, kMinimumTextContrast);

    palette.setColor(QPalette::Highlight, highlight);
    palette.setColor(QPalette::HighlightedText, highlightedText);
    palette.setColor(QPalette::Link, link);
    palette.setColor(QPalette::LinkVisited, link);
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    palette.setColor(QPalette::Accent, highlight);
#endif
}

/// Whether @p widget belongs to the tools area whose colors styles such as Breeze set themselves.
bool isToolsArea(const QWidget* widget)
{ return qobject_cast<const QMenuBar*>(widget) != nullptr || qobject_cast<const QToolBar*>(widget) != nullptr; }

/**
 * Whether @p widget is a tab widget, its tab bar or one of the tab bar's buttons (close, scroll). They share the
 * style sheet of the editor tab widgets. The editors inside are left out: re-setting the palette of a web view
 * stops it from rendering.
 */
bool isTabPart(const QWidget* widget)
{
    return qobject_cast<const QTabWidget*>(widget) != nullptr || qobject_cast<const QTabBar*>(widget) != nullptr ||
           qobject_cast<const QTabBar*>(widget->parentWidget()) != nullptr;
}

/// Makes every tab widget and tab bar pick up the current application palette while keeping the colors it set itself.
void refreshTabWidgets()
{
    for (QWidget* widget : QApplication::allWidgets()) {
        if (isTabPart(widget)) {
            // Re-setting the own palette resolves every role the widget did not set itself against the new
            // application palette, which Qt skips for the editor tab widgets because they carry a style sheet.
            widget->setPalette(widget->palette());
        }
    }
}

/// @p color in the "r,g,b" notation of KDE color schemes.
QString schemeColor(const QColor& color)
{ return QString("%1,%2,%3").arg(color.red()).arg(color.green()).arg(color.blue()); }

/**
 * One color set of a KDE color scheme. Hover and focus decorations use the highlight, so styles like Breeze mark
 * hovered and focused items in the theme's selection color.
 *
 * @param name Color set as KDE names it, e.g. "View".
 * @param surface Palette role of the set's background.
 * @param text Palette role of the set's text.
 */
QString colorSchemeSet(
    const QString& name, const QPalette& palette, QPalette::ColorRole surface, QPalette::ColorRole text)
{
    return QString(
        "[Colors:%1]\nBackgroundNormal=%2\nBackgroundAlternate=%3\nForegroundNormal=%4\n"
        "ForegroundInactive=%5\nForegroundLink=%6\nForegroundVisited=%6\nDecorationFocus=%7\n"
        "DecorationHover=%7\n\n")
        .arg(name,
            schemeColor(palette.color(surface)),
            schemeColor(palette.color(QPalette::AlternateBase)),
            schemeColor(palette.color(text)),
            schemeColor(palette.color(QPalette::Disabled, text)),
            schemeColor(palette.color(QPalette::Link)),
            schemeColor(palette.color(QPalette::Highlight)));
}

/// Deletes the color schemes earlier calls of writeColorScheme() left in @p directory.
void removeColorSchemes(const QDir& directory)
{
    for (const QString& fileName : directory.entryList({kColorSchemePattern}, QDir::Files)) {
        QFile::remove(directory.filePath(fileName));
    }
}

/**
 * Writes @p palette as a KDE color scheme into the cache directory.
 *
 * Each palette gets its own file name: KDE caches a color scheme by its path, so rewriting one file would leave
 * styles drawing with the colors of the previous theme.
 *
 * @return Path of the written scheme, or an empty string when it could not be written.
 */
QString writeColorScheme(const QPalette& palette)
{
    const QString scheme = toKdeColorScheme(palette);
    const QDir directory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    if (!directory.mkpath(".")) {
        qWarning() << "Cannot create the cache directory for the UI color scheme:" << directory.path();
        return QString();
    }
    removeColorSchemes(directory);

    QSaveFile file(directory.filePath(kColorSchemeFileName.arg(qHash(scheme), 0, 16)));
    if (!file.open(QIODevice::WriteOnly) || file.write(scheme.toUtf8()) < 0 || !file.commit()) {
        qWarning() << "Cannot write the UI color scheme:" << file.fileName() << file.errorString();
        return QString();
    }

    return file.fileName();
}

} // namespace

QString toKdeColorScheme(const QPalette& palette)
{
    return QString("[General]\nName=Notepadqq\n\n") +
           colorSchemeSet("Window", palette, QPalette::Window, QPalette::WindowText) +
           colorSchemeSet("View", palette, QPalette::Base, QPalette::Text) +
           colorSchemeSet("Button", palette, QPalette::Button, QPalette::ButtonText) +
           colorSchemeSet("Selection", palette, QPalette::Highlight, QPalette::HighlightedText) +
           colorSchemeSet("Tooltip", palette, QPalette::ToolTipBase, QPalette::ToolTipText) +
           colorSchemeSet("Header", palette, QPalette::Window, QPalette::WindowText) +
           colorSchemeSet("Complementary", palette, QPalette::Window, QPalette::WindowText);
}

void applyToApplication(const QPalette& palette)
{
    // Set before the palette: KDE styles reload their colors from this scheme when the palette changes.
    qApp->setProperty(kColorSchemeProperty, writeColorScheme(palette));
    QApplication::setPalette(palette);

    for (QWidget* widget : QApplication::allWidgets()) {
        if (isToolsArea(widget)) {
            widget->setPalette(QApplication::palette(widget));
        }
    }
    refreshTabWidgets();
}

void restoreApplication()
{
    for (QWidget* widget : QApplication::allWidgets()) {
        if (isToolsArea(widget)) {
            widget->setPalette(QPalette());
        }
    }

    qApp->setProperty(kColorSchemeProperty, QVariant());
    removeColorSchemes(QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)));
    QApplication::setPalette(QPalette());
    refreshTabWidgets();
}

double contrastRatio(const QColor& first, const QColor& second)
{
    const double firstLuminance = relativeLuminance(first);
    const double secondLuminance = relativeLuminance(second);

    return (std::max(firstLuminance, secondLuminance) + kContrastFlare) /
           (std::min(firstLuminance, secondLuminance) + kContrastFlare);
}

QPalette fromEditorColors(const EditorColors& colors)
{
    const QColor& background = colors.background;
    const bool isDark = background.lightnessF() < kDarkLightnessThreshold;
    const QColor button = towardsContrast(background, kButtonFactor, isDark);
    const QColor text = withMinimumContrast(colors.foreground, button, kMinimumTextContrast);

    QPalette palette;
    palette.setColor(QPalette::Window, background);
    palette.setColor(QPalette::Base, background);
    palette.setColor(QPalette::AlternateBase, towardsContrast(background, kAlternateFactor, isDark));
    palette.setColor(QPalette::Button, button);
    palette.setColor(QPalette::ToolTipBase, button);
    palette.setColor(QPalette::Light, background.lighter(kLightFactor));
    palette.setColor(QPalette::Midlight, background.lighter(kMidlightFactor));
    palette.setColor(QPalette::Mid, background.darker(kMidFactor));
    palette.setColor(QPalette::Dark, background.darker(kDarkFactor));
    palette.setColor(QPalette::Shadow, background.darker(kShadowFactor));

    setTextRoles(
        palette, text, withMinimumContrast(blend(text, background, kMutedShare), button, kMinimumMutedContrast));
    setSelectionAndLinkRoles(palette, colors, text);

    return palette;
}

} // namespace UiThemePalette
