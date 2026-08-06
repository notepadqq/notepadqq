#include "include/nqqfiledialog.h"

#include "include/nqqsettings.h"

#include <QFileInfo>

namespace NqqFileDialog {

namespace {

// Reproduces what the static QFileDialog functions do with their "dir" argument: a path pointing
// at a file selects that file inside its own directory, anything else is a starting directory.
void applyInitialLocation(QFileDialog& dialog, const QUrl& location)
{
    if (location.isEmpty())
        return;

    if (!location.isLocalFile()) {
        dialog.setDirectoryUrl(location);
        return;
    }

    const QFileInfo info(location.toLocalFile());
    if (info.isDir()) {
        dialog.setDirectory(info.absoluteFilePath());
        return;
    }

    dialog.setDirectory(info.absolutePath());
    if (!info.fileName().isEmpty())
        dialog.selectFile(info.fileName());
}

bool showHiddenFiles()
{ return NqqSettings::getInstance().General.getShowHiddenFiles(); }

} // namespace

void applySettings(QFileDialog& dialog)
{
    const bool showHidden = showHiddenFiles();

    // A platform dialog silently ignores the QDir::Hidden filter below, so honouring the
    // setting means giving up the native one.
    const bool useNative = NqqSettings::getInstance().General.getUseNativeFilePicker() && !showHidden;
    dialog.setOption(QFileDialog::DontUseNativeDialog, !useNative);

    if (showHidden)
        dialog.setFilter(dialog.filter() | QDir::Hidden);
}

QDir::Filters entryFilters(QDir::Filters baseFilters)
{ return showHiddenFiles() ? (baseFilters | QDir::Hidden) : baseFilters; }

QList<QUrl> getOpenFileUrls(QWidget* parent, const QString& caption, const QUrl& dir, const QString& filter)
{
    QFileDialog dialog(parent, caption);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    if (!filter.isEmpty())
        dialog.setNameFilter(filter);
    applyInitialLocation(dialog, dir);
    applySettings(dialog);

    if (dialog.exec() != QDialog::Accepted)
        return {};

    return dialog.selectedUrls();
}

QString getOpenFileName(QWidget* parent, const QString& caption, const QString& dir, const QString& filter)
{
    QFileDialog dialog(parent, caption);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    if (!filter.isEmpty())
        dialog.setNameFilter(filter);
    applyInitialLocation(dialog, QUrl::fromLocalFile(dir));
    applySettings(dialog);

    if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty())
        return QString();

    return dialog.selectedFiles().first();
}

QString getSaveFileName(QWidget* parent, const QString& caption, const QString& dir, const QString& filter)
{
    QFileDialog dialog(parent, caption);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    if (!filter.isEmpty())
        dialog.setNameFilter(filter);
    applyInitialLocation(dialog, QUrl::fromLocalFile(dir));
    applySettings(dialog);

    if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty())
        return QString();

    return dialog.selectedFiles().first();
}

QString getExistingDirectory(QWidget* parent, const QString& caption, const QString& dir, QFileDialog::Options options)
{
    QFileDialog dialog(parent, caption);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOptions(options);
    applyInitialLocation(dialog, QUrl::fromLocalFile(dir));
    applySettings(dialog);

    if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty())
        return QString();

    return dialog.selectedFiles().first();
}

} // namespace NqqFileDialog
