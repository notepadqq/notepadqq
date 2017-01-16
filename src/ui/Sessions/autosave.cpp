#include "include/Sessions/autosave.h"
#include "include/Sessions/persistentcache.h"
#include "include/Sessions/sessions.h"

#include "include/mainwindow.h"

QTimer Autosave::s_autosaveTimer;
std::vector<Autosave::WindowData> Autosave::s_autosaveData;

void Autosave::executeAutosave() {
    // s_autosaveData contains pointers to all MainWindows (and their editors) that were saved during the last autosave. Since
    // windows could have been opened or closed in the meantime the following steps are taken to update s_autosaveData:
    // 1) Walk through all WindowData items in s_autosaveData and remove all that point to a MainWindow that isn't open anymore.
    // 2) Walk through all MainWindows and add all to s_autosaveData that don't already have a corresponding WindowData item.
    // 3) Walk through all WindowData items in s_autosaveData and see if the window needs to be saved.

    const auto& autosavePath = PersistentCache::autosaveDirPath();
    const auto& windows = MainWindow::instances();

    // Step 1: Remove window data of windows that don't exist anymore.
    s_autosaveData.erase(std::remove_if(s_autosaveData.begin(), s_autosaveData.end(), [&](const WindowData& wndItem){
        auto it = std::find(windows.begin(), windows.end(), wndItem.ptr);

        if (it != windows.end())
            return false;

        // Remove the folder that contains the saved session for this window.
        // MainWindow's address is used to have a unique path name.
        const auto ptrToInt = reinterpret_cast<uintptr_t>(wndItem.ptr);
        const QString cachePath = autosavePath + QString("/window_%1").arg(ptrToInt);
        QDir(cachePath).removeRecursively();

        qDebug() << "Removing" << cachePath;

        return true;
    }), s_autosaveData.end());

    qDebug() << "Num wndItems: " << s_autosaveData.size();

    // Step 2: Add window data of all newly created windows
    for (const auto& wnd : windows) {
        auto it = std::find_if(s_autosaveData.begin(), s_autosaveData.end(), [wnd](const WindowData& w) { return w.ptr == wnd; });

        if (it != s_autosaveData.end())
            continue;

        // Add a WindowData object with this window's pointer data.
        // The editor vector is left empty to indicate that this MainWindow still needs to be autosaved.
        WindowData w{ w.ptr, {} };
        w.ptr = wnd;

        qDebug() << "Adding window" << w.ptr;

        s_autosaveData.push_back( std::move(w) );
    }

    // Step 3: Walk through all window data and create a session if necessary.
    for (auto& wndItem : s_autosaveData) {
        MainWindow* wnd = wndItem.ptr;

        // This lambda iterates through each Editor of a given TopEditorContainer* and searches through the
        // std::vector<Editor*> in wndItem to see if the Editor is in that vector and whether its history generation
        // matches. If yes to both, the Editor has already been saved in a previous autosave. If no to either, the Editor
        // needs to be autosaved.
        const auto needsSaving = [wndItem](TopEditorContainer* top){
            bool success = true;

            top->forEachEditor([wndItem, &ok](int,int,EditorTabWidget*,Editor* ed) {
                const auto& editors = wndItem.editors;
                auto eit = std::find_if(editors.begin(), editors.end(), [&](const std::pair<Editor*, int> p) {
                    qDebug() << "Found" << p.first << p.second << ed->getHistoryGeneration();

                    return p.first == ed && p.second == ed->getHistoryGeneration();
                });

                success = eit != editors.end();

                // If success==false, an Editor has been found that needs saving. In this case we can stop iterating
                // because this MainWindow definitely needs to be autosaved.
                return success;
            });

            return !success;
        };

        // An autosave only needs to be done for this MainWindow if any Editors were added/removed or the user made changes
        // to one of the editors.
        if (wndItem.editors.size() != wnd->topEditorContainer()->getNumEditors() || needsSaving(wnd->topEditorContainer())) {

            auto sz  =wndItem.editors.size() != wnd->topEditorContainer()->getNumEditors();
            auto sa = sadf(wnd->topEditorContainer());
            qDebug() << "Saving session" << wndItem.ptr << ". Reason:" << sz <<  sa;

            // Save this MainWindow as a session inside the autosave path.
            // MainWindow's address is used to have a unique path name.
            const auto ptrToInt = reinterpret_cast<uintptr_t>(wnd);
            const QString cachePath = autosavePath + QString("/window_%1").arg(ptrToInt);
            const QString sessPath = autosavePath + QString("/window_%1/window.xml").arg(ptrToInt);

            bool success = Sessions::saveSession(wnd->getDocEngine(), wnd->topEditorContainer(), sessPath, cachePath);

            // The autosave dir is inside the Nqq config folder and should *always* be accessible. However, if for some reason
            // it isn't, we don't want to continue as if the window has been saved properly. executeAutosave() will attempt another
            // autosave after the autosave timer period has passed.
            if(!success)
                continue;

            // Now that the MainWindow has been saved its data inside the wndItem needs to be updated.
            // Specifically we save the list of all Editors and their history generation.
            wndItem.editors.clear();

            wnd->topEditorContainer()->forEachEditor([&wndItem](int,int,EditorTabWidget*,Editor* ed) {
                wndItem.editors.push_back( std::make_pair(ed, ed->getHistoryGeneration()) );
                return true;
            });

            qDebug() << "Updated window" << wndItem.ptr << "with" << wndItem.editors.size() << "editors";
        }

    }

}

void Autosave::restoreFromAutosave()
{
    const auto& autosavePath = PersistentCache::autosaveDirPath();

    // Each window is saved as a separate session inside a subdirectory.
    // Grab all subdirs and load the session files inside.
    QDir autosaveDir(autosavePath);
    autosaveDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    const auto& dirs = autosaveDir.entryInfoList();

    for (const auto& dirInfo : dirs) {
        const auto sessPath = dirInfo.filePath() + "/window.xml";

        MainWindow* wnd = new MainWindow(QStringList(), 0);
        Sessions::loadSession(wnd->getDocEngine(), wnd->topEditorContainer(), sessPath);
        wnd->show();
    }


}

void Autosave::enableAutosave()
{
    if (s_autosaveTimer.isActive())
        return;

    // Last shutdown was either proper or all windows are already restored in case of a crash. Either way, the autosave directory needs to be cleared so Autosave can start anew.
    const auto& autosavePath = PersistentCache::autosaveDirPath();
    QDir autosaveDir(autosavePath);

    if (autosaveDir.exists())
        autosaveDir.removeRecursively();

    QObject::connect(&Autosave::s_autosaveTimer, &QTimer::timeout, &Autosave::executeAutosave);

    s_autosaveTimer.setInterval(AUTOSAVE_INTERVAL);
    s_autosaveTimer.start(AUTOSAVE_INTERVAL);
}

void Autosave::disableAutosave()
{
    s_autosaveTimer.stop();

    // disableAutosave() is only called explicitely by the user if they want to disable the feature. In this case we'll clear the autosave dir to clear up disk space.
    const auto& autosavePath = PersistentCache::autosaveDirPath();
    QDir autosaveDir(autosavePath);

    if (autosaveDir.exists())
        autosaveDir.removeRecursively();
}
