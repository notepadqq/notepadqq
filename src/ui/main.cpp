#include "include/mainwindow.h"
#include "include/notepadqq.h"
#include <QObject>
#include <QFile>
#include <QDir>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>

#ifdef QT_DEBUG
#include <QElapsedTimer>
#endif

bool shouldStartApp(int argc, char *argv[]);
void checkQtVersion(MainWindow *w);

int main(int argc, char *argv[])
{
#ifdef QT_DEBUG
    QElapsedTimer __aet_timer;
    __aet_timer.start();
    qDebug() << "Start-time benchmark started.";
#endif

    if (!shouldStartApp(argc, argv)) {
      return 0;
    }

    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Notepadqq");
    QCoreApplication::setApplicationName("Notepadqq");

    QFile file(Notepadqq::editorPath());
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Can't open file: " + file.fileName();
        return EXIT_FAILURE;
    }
    file.close();

    // Remove ugly borders from statusbar
    //a.setStyleSheet("QStatusBar::item { border: none; }; ");

    MainWindow w;
    w.show();

#ifdef QT_DEBUG
    qint64 __aet_elapsed = __aet_timer.nsecsElapsed();
    qDebug() << QString("Started in " + QString::number(__aet_elapsed / 1000 / 1000) + "msec").toStdString().c_str();
#endif

    QSettings *settings = new QSettings();
    if (settings->value("checkQtVersionAtStartup", true).toBool())
        checkQtVersion(&w);

    return a.exec();
}

void checkQtVersion(MainWindow *w)
{
    QString runtimeVersion = qVersion();
    if (runtimeVersion.startsWith("5.0") ||
            runtimeVersion.startsWith("5.1") ||
            runtimeVersion.startsWith("5.2")) {

        QString dir = QDir::toNativeSeparators(QDir::homePath() + "/Qt");

        QMessageBox msgBox(w);
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("<h3>" + QObject::tr("You're using an old version of Qt (%1)").arg(qVersion()) + "</h3>");
        msgBox.setInformativeText("<html><body>"
            "<p>" + QObject::tr("Notepadqq will try to do its best, but some things might not work properly.") + "</p>" +
            QObject::tr(
                "Install a newer Qt version (&ge; %1) from the official repositories "
                "of your distribution.<br><br>"
                "If it's not available, download Qt (&ge; %1) from %2 and install it to %3.").
                      arg("5.3").
                      arg("<nobr><a href=\"http://qt-project.org/\">http://qt-project.org/</a></nobr>").
                      arg("<nobr>" + dir + "</nobr>") +
            "</body></html>");

        msgBox.exec();
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

inline
bool shouldStartApp(int argc, char* argv[])
{
#define MATCHES_OPT(str, short, long) \
    strcmp(str, short)==0 || strcmp(str, long)==0

    if (argc > 1) {
        const char* const firstArg = argv[1];
        if (MATCHES_OPT(firstArg, "-h", "--help")) {
          displayHelp();
          return false;
        }

        if (MATCHES_OPT(firstArg, "-v", "--version")) {
          displayVersion();
          return false;
        }

    }
    return true;

#undef MATCHES_OPT
}
