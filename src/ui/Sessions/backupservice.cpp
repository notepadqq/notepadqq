#include "include/Sessions/backupservice.h"

#include "include/Sessions/persistentcache.h"
#include "include/Sessions/sessions.h"
#include "include/mainwindow.h"

#include <QApplication>

#include <set>

QTimer BackupService::s_autosaveTimer;
bool BackupService::s_autosaveEnabled = false;
std::set<BackupService::WindowData> BackupService::s_backupWindowData;

void BackupService::executeBackup() {
    const auto& backupPath = PersistentCache::backupDirPath();

    std::set<WindowData> newData, savedData, temp;

    // Fill newData with up-to-date window data
    for (const auto& wnd : MainWindow::instances()) {
        WindowData wd;
        wd.ptr = wnd;
        wnd->topEditorContainer()->forEachEditor([&wd](int,int,EditorTabWidget*,QSharedPointer<Editor> ed) {
            int gen = -1;
            ed->getHistoryGeneration().wait().tap([&](int value){gen = value;});
            wd.editors.push_back( std::make_pair(ed, gen) );
            return true;
        });
        newData.insert(std::move(wd));
    }

    // Find all closed windows and remove their backups
    std::set_difference(s_backupWindowData.begin(), s_backupWindowData.end(),
                        newData.begin(), newData.end(),
                        std::inserter(temp, temp.end()));

    for (const auto& item : temp) {
        const auto ptrToInt = reinterpret_cast<uintptr_t>(item.ptr);
        const QString cachePath = backupPath + QString("/window_%1").arg(ptrToInt);
        QDir(cachePath).removeRecursively();
    }

    // Find all newly created windows and create their backups
    temp.clear();
    std::set_difference(newData.begin(), newData.end(),
                        s_backupWindowData.begin(), s_backupWindowData.end(),
                        std::inserter(temp, temp.end()));

    for (const auto& item : temp) {
        // If writeBackup() fails we don't mark this window as saved. Another attempt at saving will be made
        // next time executeBackup() runs.
        if (writeBackup(item.ptr))
            savedData.insert(item);
    }

    // Find all persisting windows and re-check whether to save them
    temp.clear();
    std::set_intersection(s_backupWindowData.begin(), s_backupWindowData.end(),
                          newData.begin(), newData.end(),
                          std::inserter(temp, temp.end()));

    for (const auto& oldItem : temp) { // oldItem is always from the first set (s_backupWindowData)
        const auto& newItem = *newData.find(oldItem);

        // If oldItem and newItem are fully equal, their contents haven't changed and need not be backed up...
        if (oldItem.isFullyEqual(newItem)) {
            savedData.insert(newItem);
            continue;
        }

        // ...otherwise we attempt saving the backup
        if (writeBackup(newItem.ptr))
            savedData.insert(newItem);
    }

    s_backupWindowData = savedData;
}

bool BackupService::writeBackup(MainWindow* wnd)
{
    // Save this MainWindow as a session inside the autosave path.
    // MainWindow's address is used to have a unique path name.
    const auto& backupPath = PersistentCache::backupDirPath();
    const auto ptrToInt = reinterpret_cast<uintptr_t>(wnd);
    const QString cachePath = backupPath + QString("/window_%1").arg(ptrToInt);
    const QString sessPath = backupPath + QString("/window_%1/window.xml").arg(ptrToInt);

    return Sessions::saveSession(wnd->getDocEngine(), wnd->topEditorContainer(), sessPath, cachePath);
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
                          QObject::tr("Notepadqq was not closed properly. Do you want to recover unsaved changes?"),
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
    if (s_autosaveEnabled)
        return;

    static bool initializer = false;
    if (!initializer) {
        // Only create this connection once. Since we're connecting to a plain old function we can't use
        // Qt::UniqueConnection or QObject::disconnect() to make things easier.
        initializer = true;
        QObject::connect(&BackupService::s_autosaveTimer, &QTimer::timeout, &BackupService::executeBackup);

        // Disable the autosave timer when the application goes out of focus.
        QObject::connect(qApp, &QGuiApplication::applicationStateChanged, [](Qt::ApplicationState state) {
            if (!s_autosaveEnabled) return;

            switch (state) {
            case Qt::ApplicationInactive: s_autosaveTimer.stop(); break;
            case Qt::ApplicationActive: s_autosaveTimer.start(); break;
            default: break;
            }
        });
    }

    s_autosaveEnabled = true;
    s_autosaveTimer.setTimerType(Qt::VeryCoarseTimer);
    s_autosaveTimer.setInterval(intervalInSeconds * 1000);
    s_autosaveTimer.start();
}

void BackupService::disableAutosave()
{
    if (!s_autosaveEnabled)
        return;

    s_autosaveEnabled = false;
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

void BackupService::pause()
{
    s_autosaveTimer.stop();
}

void BackupService::resume()
{
    if (s_autosaveEnabled) {
        // The previous interval is persisted in s_autosaveTimer.interval()
        s_autosaveTimer.start();
    }
}
