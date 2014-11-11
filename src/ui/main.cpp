#include "include/mainwindow.h"
#include "include/notepadqq.h"
#include "include/EditorNS/editor.h"
#include "include/singleapplication.h"
#include <QObject>
#include <QFile>
#include <QSettings>

#ifdef QT_DEBUG
#include <QElapsedTimer>
#endif

void checkQtVersion();
void forceDefaultSettings();

int main(int argc, char *argv[])
{
#ifdef QT_DEBUG
    QElapsedTimer __aet_timer;
    __aet_timer.start();
    qDebug() << "Start-time benchmark started.";
#endif

    SingleApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Notepadqq");
    QCoreApplication::setApplicationName("Notepadqq");
    QCoreApplication::setApplicationVersion(Notepadqq::version);

    forceDefaultSettings();

    Notepadqq::parseCommandLineParameters();

    if (a.attachToOtherInstance()) {
        return EXIT_SUCCESS;
    }

    // There are no other instances.
    a.startServer();

    Editor::addEditorToBuffer();

    QFile file(Notepadqq::editorPath());
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Can't open file: " + file.fileName();
        return EXIT_FAILURE;
    }
    file.close();

    checkQtVersion();

    MainWindow w(true, 0);
    w.show();

#ifdef QT_DEBUG
    qint64 __aet_elapsed = __aet_timer.nsecsElapsed();
    qDebug() << QString("Started in " + QString::number(__aet_elapsed / 1000 / 1000) + "msec").toStdString().c_str();
#endif

    QSettings settings;
    if (Notepadqq::oldQt() && settings.value("checkQtVersionAtStartup", true).toBool()) {
        Notepadqq::showQtVersionWarning(true, &w);
    }

    return a.exec();
}

void checkQtVersion()
{
    QString runtimeVersion = qVersion();
    if (runtimeVersion.startsWith("5.0") ||
            runtimeVersion.startsWith("5.1") ||
            runtimeVersion.startsWith("5.2")) {

        Notepadqq::setOldQt(true);
    }
}

void forceDefaultSettings()
{
    QSettings settings;

    // Use tabs to indent makefile by default
    if (!settings.contains("Languages/makefile/useDefaultSettings")) {
        settings.setValue("Languages/makefile/useDefaultSettings", false);
        settings.setValue("Languages/makefile/indentWithSpaces", false);
    }
}
