#include "include/mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Remove ugly borders from statusbar
    //a.setStyleSheet("QStatusBar::item { border: none; }; ");

    MainWindow w;
    w.show();

    return a.exec();
}
