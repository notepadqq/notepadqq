/*
 *
 * This file is part of the Notepadqq text editor.
 *
 * Copyright(c) 2010 Notepadqq team.
 * http://notepadqq.sourceforge.net/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QtGui/QApplication>
#include "mainwindow.h"
#include <QTextCodec>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QSharedMemory>
#include <QtNetwork/QLocalSocket>
#include <QDesktopWidget>
#include "constants.h"

// package libgtk3.0-0
// required for build: libgtk3.0-dev

void processOtherInstances();
int numberOfFilesInArgs(QStringList arguments);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.processEvents();

#if SINGLEINSTANCE_EXPERIMENTAL
    /* Attach to an existing instance
     * only if there is at least
     * one file in the arguments.
     */
    if(numberOfFilesInArgs(QApplication::arguments()) > 0) {
        processOtherInstances();
    }
#endif

    // Load current locale
    QTranslator qtTranslator;
    QTranslator appTranslator;

    //QString translationDir = QCoreApplication::applicationDirPath() + "/L10n";

    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    appTranslator.load(QString("notepadqq_") + QLocale::system().name(), ApplicationL10nDir());

    a.installTranslator(&qtTranslator);
    a.installTranslator(&appTranslator);

    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    QCoreApplication::setOrganizationName("notepadqq");
    QCoreApplication::setOrganizationDomain("http://notepadqq.sourceforge.net/");
    QCoreApplication::setApplicationName("Notepadqq");
    //QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, true);
    QCoreApplication::setAttribute(Qt::AA_NativeWindows, true);
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar, false);

    MainWindow* w = MainWindow::instance();
    w->showMaximized();

    //gdk_notify_startup_complete();

    return a.exec();
}

int numberOfFilesInArgs(QStringList arguments) {
    return arguments.count() - 1;
}

void processOtherInstances()
{
    //QApplication::x11ProcessEvent()
    //QApplication::desktop()
//    QSharedMemory shared(INSTANCESERVER_ID);

//    // Ok, if the shared memory is already used, another instance of the application is already running...
//    if( !shared.create( 512, QSharedMemory::ReadWrite) )
//    {
        /* Hmm... if the previous instance of notepadqq crashed, the shared memory is seen as used but the software isn't running.
           So, the only way to start a new instance is to explicitly delete the shared memory with a system command or reboot the computer.
           And that's not good.
           Let's see instead if someone greets us...
        */

        QLocalSocket * app_socket = new QLocalSocket();
        app_socket->connectToServer(INSTANCESERVER_ID);
        if(app_socket->waitForConnected(2000))
        {
            app_socket->write(QString("NEW_CLIENT").toUtf8());
            app_socket->waitForBytesWritten(2000);

            if(app_socket->waitForReadyRead(2000))
            {
                QString greet = "";

                while (app_socket->bytesAvailable()) {
                    greet += QString::fromUtf8(app_socket->readAll());
                }

                if(greet == "HELLO")
                {
                    // WHOA! There is someone out there!!
                    QByteArray ar;
                    QDataStream args(&ar, QIODevice::WriteOnly);
                    args << QApplication::arguments();
                    app_socket->write(ar);
                    app_socket->waitForBytesWritten(5000);
                    /*
                    if(QApplication::argc() <= 1)
                    {
                        app_socket->write(QString("NEW").toUtf8());
                        app_socket->waitForBytesWritten(5000);
                    } else
                    {
                        for(int i = 1; i < QApplication::argc(); i++)
                        {
                            QString msg = QString(QString("OPEN ") + QString(QApplication::argv()[i]));
                            app_socket->write(msg.toUtf8());
                            app_socket->waitForBytesWritten(2000);
                        }
                    }
                    */
                    app_socket->disconnectFromServer();
                    exit(0);
                }
            }
            app_socket->disconnectFromServer();
        }
    //}
}
