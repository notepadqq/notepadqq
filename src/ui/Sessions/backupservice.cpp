#include <set>

#include "include/Sessions/backupservice.h"
#include "include/Sessions/persistentcache.h"
#include "include/Sessions/sessions.h"

#include "include/mainwindow.h"

QTimer BackupService::s_autosaveTimer;
std::vector<BackupService::WindowData> BackupService::s_backupWindowData;

void BackupService::executeBackup() {
    const auto& backupPath = PersistentCache::backupDirPath();

    std::vector<WindowData> newData, unionOfData, savedData;

    // Fill newData with up-to-date window data
    for (const auto& wnd : MainWindow::instances()) {
        WindowData wd;
        wd.ptr = wnd;
        wnd->topEditorContainer()->forEachEditor([&wd](int,int,EditorTabWidget*,Editor* ed) {
            wd.editors.push_back( std::make_pair(ed, ed->getHistoryGeneration()) );
            return true;
        });
        newData.push_back(std::move(wd));
    }

    // Set unionOfData to set union of old and new window data
    std::set_union(s_backupWindowData.begin(), s_backupWindowData.end(),
                   newData.begin(), newData.end(),
                   std::back_inserter(unionOfData));

    for (const auto& item : unionOfData) {
        const auto oldIter = std::find(s_backupWindowData.begin(), s_backupWindowData.end(), item);
        const auto newIter = std::find(newData.begin(), newData.end(), item);

        const bool isInOld = oldIter != s_backupWindowData.end();
        const bool isInNew = newIter != newData.end();

        if (!isInNew) {
            // These windows have been closed by the user. Remove their session caches.
            const auto ptrToInt = reinterpret_cast<uintptr_t>(item.ptr);
            const QString cachePath = backupPath + QString("/window_%1").arg(ptrToInt);
            QDir(cachePath).removeRecursively();
            continue;
        }

        if (isInOld && oldIter->isFullyEqual(*newIter)) {
            // These windows are unchanged. Don't save them
            savedData.push_back(*newIter);
            continue;
        }

        // At this point the window is either new (!isInOld && isInNew) or changed (isInOld && !isFullyEqual(new,old)).

        // Save this MainWindow as a session inside the autosave path.
        // MainWindow's address is used to have a unique path name.
        MainWindow* wnd = item.ptr;
        const auto ptrToInt = reinterpret_cast<uintptr_t>(wnd);
        const QString cachePath = backupPath + QString("/window_%1").arg(ptrToInt);
        const QString sessPath = backupPath + QString("/window_%1/window.xml").arg(ptrToInt);

        bool success = Sessions::saveSession(wnd->getDocEngine(), wnd->topEditorContainer(), sessPath, cachePath);

        // If the session couldn't be saved we won't add this to the saved windows, since it hasn't been.
        // Another save attempt will be made when the autosave timer times out next time.
        if(success)
            savedData.push_back(*newIter);
    }

    s_backupWindowData = savedData;
}

bool BackupService::restoreFromBackup()
{
    const auto& backupPath = PersistentCache::backupDirPath();

    // Each window is saved as a separate session inside a subdirectory.
    // Grab all subdirs and load the session files inside.
    QDir autosaveDir(backupPath);
    autosaveDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    const auto& dirs = autosaveDir.entryInfoList();

    if (dirs.isEmpty())
        return false;

    auto ret = QMessageBox::question(nullptr,
                          "",
                          QObject::tr("It appears Notepadqq crashed. Do you want to restore the latest backup?"),
                          QMessageBox::Yes | QMessageBox::No,
                          QMessageBox::Yes);

    if (ret == QMessageBox::No)
        return false;

    for (const auto& dirInfo : dirs) {
        const auto sessPath = dirInfo.filePath() + "/window.xml";

        MainWindow* wnd = new MainWindow(QStringList(), nullptr);
        Sessions::loadSession(wnd->getDocEngine(), wnd->topEditorContainer(), sessPath);
        wnd->show();
    }

    return true;
}

bool BackupService::detectImproperShutdown()
{
    return QDir(PersistentCache::backupDirPath()).exists();
}

void BackupService::enableAutosave(int intervalInSeconds)
{
    if (s_autosaveTimer.isActive())
        return;

    clearBackupData();

    static bool initializer = false;
    if (!initializer) {
        // Only create this connection once. Since we're connecting to a plain old function we can't use
        // Qt::UniqueConnection or QObject::disconnect() to make things easier.
        initializer = true;
        QObject::connect(&BackupService::s_autosaveTimer, &QTimer::timeout, &BackupService::executeBackup);
    }

    s_autosaveTimer.setInterval(intervalInSeconds * 1000);
    s_autosaveTimer.start(intervalInSeconds * 1000);
}

void BackupService::disableAutosave()
{
    if (!s_autosaveTimer.isActive())
        return;

    s_autosaveTimer.stop();
    clearBackupData();
}

void BackupService::clearBackupData()
{
    const auto& backupPath = PersistentCache::backupDirPath();
    QDir backupDir(backupPath);

    if (backupDir.exists())
        backupDir.removeRecursively();

    s_backupWindowData.clear();
}
