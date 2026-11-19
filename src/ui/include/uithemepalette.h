#ifndef UITHEMEPALETTE_H
#define UITHEMEPALETTE_H

#include <QColor>
#include <QPalette>
#include <QString>

namespace UiThemePalette {

/**
 * @brief Colors an editor theme reports for coloring the rest of the user interface.
 *
 * Only background and foreground are required. The editor leaves selection or link invalid when its theme does not
 * define them; fromEditorColors() then derives them from background and foreground.
 */
struct EditorColors {
    /// Background of the text area; becomes the window and base color unchanged.
    QColor background;
    /// Base text color of the theme; corrected for contrast before it is used as UI text.
    QColor foreground;
    /// Background of selected text, already composed onto @ref background when the theme uses transparency.
    QColor selection;
    /// Color of links in the text.
    QColor link;
};

/**
 * @brief Derive a complete application palette from the colors of the editor theme.
 *
 * Every color role is set, so nothing falls back to the desktop palette: the user interface looks the same on every
 * desktop. Text roles are corrected to WCAG AAA (7:1), disabled and placeholder text to 3:1, and the selection is
 * kept apart from the background (1.5:1), changing the theme colors no more than necessary.
 *
 * @param colors Colors of the current editor theme; background and foreground must be valid.
 */
QPalette fromEditorColors(const EditorColors& colors);

/**
 * @brief WCAG 2 contrast ratio between two colors.
 *
 * @return Between 1.0 (identical luminance) and 21.0 (black on white); the order of the colors does not matter.
 */
double contrastRatio(const QColor& first, const QColor& second);

/**
 * @brief Describe @p palette as a KDE color scheme (the format of *.colors files).
 *
 * KDE styles such as Breeze draw hover and focus decorations from the color scheme instead of the Qt palette. An
 * application can hand them its own scheme through the KDE_COLOR_SCHEME_PATH property, which applyToApplication()
 * does with this text.
 *
 * @return Scheme with the Window, View, Button, Selection, Tooltip, Header and Complementary color sets.
 */
QString toKdeColorScheme(const QPalette& palette);

/**
 * @brief Make @p palette the application palette, including the widgets Qt does not update on its own.
 *
 * The editor tab widgets carry a style sheet and therefore miss application palette changes, and styles such as Breeze
 * pin the menu bar and tool bar colors to the desktop's header colors. Both are reapplied from @p palette.
 */
void applyToApplication(const QPalette& palette);

/**
 * @brief Return to the palette of the desktop after applyToApplication(), handing the menu bar and tool bars back
 * to the style.
 */
void restoreApplication();

} // namespace UiThemePalette

#endif // UITHEMEPALETTE_H
