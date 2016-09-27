#include "include/Sessions/autosave.h"
#include "include/Sessions/persistentcache.h"
#include "include/Sessions/sessions.h"

#include <QTimer>

#include "include/mainwindow.h"

static QTimer g_autosaveTimer;

// Interval between autosaves, in milliseconds.
static const int AUTOSAVE_INTERVAL = 5000;

using namespace Sessions;

namespace Autosave {

static void executeAutosave() {
    const auto& autosavePath = PersistentCache::autosaveDirPath();

    QDir autosaveDir(autosavePath);

    // Since saveSession only clears the window_# subdirectories we'll manually delete
    // the whole directory. saveSession will recreate the necessary subdirectories.
    if (autosaveDir.exists())
        autosaveDir.removeRecursively();

    int i = 0;
    for (const auto& window : MainWindow::instances()) {
        const QString cachePath = autosavePath + QString("/window_%1").arg(i);
        const QString sessPath = autosavePath + QString("/window_%1/window.xml").arg(i);

        saveSession(window, sessPath, cachePath);

        i++;
    }
}

void restoreFromAutosave()
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
        loadSession(wnd, sessPath);
        wnd->show();
    }


}

void enableAutosave()
{
    if (g_autosaveTimer.isActive())
        return;

    QObject::connect(&g_autosaveTimer, &QTimer::timeout, &executeAutosave);

    g_autosaveTimer.setInterval(AUTOSAVE_INTERVAL);
    g_autosaveTimer.start(AUTOSAVE_INTERVAL);
}

void disableAutosave()
{
    g_autosaveTimer.stop();
}

} // namespace Autosave
