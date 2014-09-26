#include "include/mainwindow.h"
#include "include/notepadqq.h"
#include "include/EditorNS/editor.h"
#include <QObject>
#include <QFile>
#include <QApplication>
#include <QSettings>

#ifdef QT_DEBUG
#include <QElapsedTimer>
#endif

void checkQtVersion();

int main(int argc, char *argv[])
{
#ifdef QT_DEBUG
    QElapsedTimer __aet_timer;
    __aet_timer.start();
    qDebug() << "Start-time benchmark started.";
#endif

    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Notepadqq");
    QCoreApplication::setApplicationName("Notepadqq");
    QCoreApplication::setApplicationVersion(Notepadqq::version);

    Notepadqq::parseCommandLineParameters();

    Editor::addEditorToBuffer();

    QFile file(Notepadqq::editorPath());
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Can't open file: " + file.fileName();
        return EXIT_FAILURE;
    }
    file.close();

    checkQtVersion();

    MainWindow w;
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

void displayHelp()
{
    printf("\n"
           "notepadqq    a Notepad++ clone\n\n"
           "Text editor with support for multiple programming languages,\n"
           "multiple encodings and plugin support.\n\n"
           "Usage:\n"
           "  notepadqq\n"
           "  notepadqq [-h|--help]\n"
           "  notepadqq [-v|--version]\n"
           "  notepadqq [file1 file2 ...]\n\n"
          );
}

void displayVersion()
{
    printf("%s\n", Notepadqq::version.toStdString().c_str());
}
