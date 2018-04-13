#include <set>

#include "include/Sessions/autosave.h"
#include "include/Sessions/persistentcache.h"
#include "include/Sessions/sessions.h"

#include "include/mainwindow.h"

QTimer Autosave::s_autosaveTimer;
std::vector<Autosave::WindowData> Autosave::s_autosaveData;

void Autosave::executeAutosave() {
    const auto& autosavePath = PersistentCache::autosaveDirPath();

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
    std::set_union(s_autosaveData.begin(), s_autosaveData.end(),
                   newData.begin(), newData.end(),
                   std::back_inserter(unionOfData));

    qDebug() << QString("Running Autosave. %1 old windows, %2 new windows, %3 union size.")
                .arg(s_autosaveData.size())
                .arg(newData.size())
                .arg(unionOfData.size());

    for (const auto& item : unionOfData) {
        const auto oldIter = std::find(s_autosaveData.begin(), s_autosaveData.end(), item);
        const auto newIter = std::find(newData.begin(), newData.end(), item);

        const bool isInOld = oldIter != s_autosaveData.end();
        const bool isInNew = newIter != newData.end();

        if (!isInNew) {
            // These windows have been closed by the user. Remove their session caches.
            const auto ptrToInt = reinterpret_cast<uintptr_t>(item.ptr);
            const QString cachePath = autosavePath + QString("/window_%1").arg(ptrToInt);
            QDir(cachePath).removeRecursively();

            qDebug() << "One item refers to a closed window. Cache removed.";
            continue;
        }

        if (isInOld && oldIter->isFullyEqual(*newIter)) {
            // These windows are unchanged. Don't save them
            qDebug() << item.ptr << "is unchanged. Not doing anything.";
            savedData.push_back(*newIter);
            continue;
        }

        if (isInOld)
            qDebug() << item.ptr << "has changed. Saving.";
        else
            qDebug() << item.ptr << "is newly opened. Saving.";

        // Save this MainWindow as a session inside the autosave path.
        // MainWindow's address is used to have a unique path name.
        MainWindow* wnd = item.ptr;
        const auto ptrToInt = reinterpret_cast<uintptr_t>(wnd);
        const QString cachePath = autosavePath + QString("/window_%1").arg(ptrToInt);
        const QString sessPath = autosavePath + QString("/window_%1/window.xml").arg(ptrToInt);

        bool success = Sessions::saveSession(wnd->getDocEngine(), wnd->topEditorContainer(), sessPath, cachePath);

        // If the session couldn't be saved we won't add this to the saved windows, since it hasn't been.
        // Another save attempt will be made when the autosave timer times out next time.
        if(success)
            savedData.push_back(*newIter);
    }

    s_autosaveData = savedData;
}

bool Autosave::restoreFromAutosave()
{
    const auto& autosavePath = PersistentCache::autosaveDirPath();

    // Each window is saved as a separate session inside a subdirectory.
    // Grab all subdirs and load the session files inside.
    QDir autosaveDir(autosavePath);
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

bool Autosave::detectImproperShutdown()
{
    return QDir(PersistentCache::autosaveDirPath()).exists();
}

void Autosave::enableAutosave(int intervalInSeconds)
{
    if (s_autosaveTimer.isActive())
        return;

    clearAutosaveData();

    static bool initializer = false;
    if (!initializer) {
        // Only create this connection once. Since we're connecting to a plain old function we can't use
        // Qt::UniqueConnection or QObject::disconnect() to make things easier.
        initializer = true;
        QObject::connect(&Autosave::s_autosaveTimer, &QTimer::timeout, &Autosave::executeAutosave);
    }

    s_autosaveTimer.setInterval(intervalInSeconds * 1000);
    s_autosaveTimer.start(intervalInSeconds * 1000);
}

void Autosave::disableAutosave()
{
    if (!s_autosaveTimer.isActive())
        return;

    s_autosaveTimer.stop();
    clearAutosaveData();
}

void Autosave::clearAutosaveData()
{
    const auto& autosavePath = PersistentCache::autosaveDirPath();
    QDir autosaveDir(autosavePath);

    if (autosaveDir.exists())
        autosaveDir.removeRecursively();

    s_autosaveData.clear();
}
