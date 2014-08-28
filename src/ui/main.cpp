#include "include/mainwindow.h"
#include "include/notepadqq.h"
#include <QFile>
#include <QApplication>

bool shouldStartApp(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    if (!shouldStartApp(argc, argv)) {
      return 0;
    }

    QApplication a(argc, argv);

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

    return a.exec();
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
