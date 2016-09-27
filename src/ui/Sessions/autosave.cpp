#include "include/Sessions/autosave.h"
#include "include/Sessions/persistentcache.h"
#include "include/Sessions/sessions.h"

#include <QTimer>
#include <QDebug>

#include "include/mainwindow.h"

static QTimer g_autosaveTimer;

// Interval between autosaves, in milliseconds.
static const int AUTOSAVE_INTERVAL = 5000;

using namespace Sessions;

namespace Autosave {

static void executeAutosave() {
    const auto& autosavePath = PersistentCache::autosaveDirPath();

    QDir autosaveDir(autosavePath);

    if (autosaveDir.exists())
        autosaveDir.removeRecursively();

    int i = 0;
    for (const auto& window : MainWindow::instances()) {
        qDebug() << "autosave window #" << i;

        const QString cachePath = autosavePath + QString("/window_%1").arg(i);
        const QString sessPath = autosavePath + QString("/window_%1/window.xml").arg(i);

        saveSession(window, sessPath, cachePath);

        i++;
    }
}

void restoreFromAutosave()
{
    qDebug() << "restore from autosave";

    const auto& autosavePath = PersistentCache::autosaveDirPath();

    QDir autosaveDir(autosavePath);
    autosaveDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    const auto& dirs = autosaveDir.entryInfoList();

    for (const auto& dirInfo : dirs) {
        qDebug() << "Dir: " << dirInfo.filePath();

        const auto sessPath = dirInfo.filePath() + "/window.xml";

        MainWindow *b = new MainWindow(QStringList(), 0);

        loadSession(b, sessPath);

        b->show();
    }


}

void enableAutosave()
{
    if (g_autosaveTimer.isActive())
        return;

    qDebug() << "enable";

    QObject::connect(&g_autosaveTimer, &QTimer::timeout, &executeAutosave);

    g_autosaveTimer.setInterval(AUTOSAVE_INTERVAL);
    g_autosaveTimer.start(AUTOSAVE_INTERVAL);
}

void disableAutosave()
{
    qDebug() << "disable";
    g_autosaveTimer.stop();
}

} // namespace Autosave
