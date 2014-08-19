#include "include/mainwindow.h"
#include "include/constants.h"
#include <QFile>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile file(ApplicationEditorPath());
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
