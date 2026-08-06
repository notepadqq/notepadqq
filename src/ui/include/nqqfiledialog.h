#ifndef NQQFILEDIALOG_H
#define NQQFILEDIALOG_H

#include <QDir>
#include <QFileDialog>
#include <QList>
#include <QString>
#include <QUrl>

class QWidget;

/**
 * @brief File dialogs that consistently apply Notepadqq's dialog settings.
 *
 * Every dialog is driven by two settings: General/UseNativeFilePicker and
 * General/ShowHiddenFiles.
 *
 * Qt cannot ask a platform dialog (GTK, KDE, xdg-desktop-portal) to reveal hidden entries:
 * QFileDialog::setFilter() only ever reaches the widget-based dialog, and the GTK theme
 * plugin never forwards QDir::Hidden to gtk_file_chooser_set_show_hidden(). So when the user
 * asks for hidden files we fall back to Qt's own dialog, the only one we can control.
 *
 * The getXXX() functions mirror the QFileDialog static functions of the same name, with those
 * settings already applied. Prefer them over the Qt ones: the static functions give no access
 * to the dialog they build, and therefore no way to set the hidden files filter.
 */
namespace NqqFileDialog {

/**
 * @brief Applies the UseNativeFilePicker and ShowHiddenFiles settings to a dialog.
 * @param dialog A dialog whose file mode and options have already been set. Calling this
 *        earlier would let those setters discard the hidden files filter.
 */
void applySettings(QFileDialog& dialog);

/**
 * @brief Adds QDir::Hidden to the given filters when the user asked to see hidden files.
 *
 * For the places where we enumerate a directory ourselves instead of going through a dialog.
 *
 * @param baseFilters The filters the caller would have used.
 * @return The filters to pass to QDir.
 */
QDir::Filters entryFilters(QDir::Filters baseFilters);

/**
 * @brief Asks the user for one or more existing files.
 * @param dir Starting location. As in Qt, a URL pointing at a file opens its parent directory
 *        and preselects the file.
 * @return The selected URLs, empty if the user cancelled.
 */
QList<QUrl> getOpenFileUrls(QWidget* parent, const QString& caption, const QUrl& dir, const QString& filter);

/**
 * @brief Asks the user for a single existing file.
 * @return The selected path, a null string if the user cancelled.
 */
QString getOpenFileName(QWidget* parent,
    const QString& caption,
    const QString& dir = QString(),
    const QString& filter = QString());

/**
 * @brief Asks the user where to save a file.
 * @param dir Starting location, whose file name part becomes the proposed name.
 * @return The chosen path, a null string if the user cancelled.
 */
QString getSaveFileName(QWidget* parent, const QString& caption, const QString& dir, const QString& filter);

/**
 * @brief Asks the user for an existing directory.
 * @return The selected path, a null string if the user cancelled.
 */
QString getExistingDirectory(
    QWidget* parent, const QString& caption, const QString& dir, QFileDialog::Options options = QFileDialog::Options());

} // namespace NqqFileDialog

#endif // NQQFILEDIALOG_H
