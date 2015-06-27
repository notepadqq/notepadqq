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

#ifndef NOTEPADQQ_H
#define NOTEPADQQ_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QProcessEnvironment>
#include <QApplication>
#include <QUrl>
#include <QCommandLineParser>
#include <QList>
#include "include/mainwindow.h"

#define POINTVERSION "0.50.1" // major.minor.revision

#define MIB_UTF_8 106

/**
 * @brief Global information and utility functions.
 */
class Notepadqq : public QObject
{
    Q_OBJECT
public:

    static Notepadqq& getInstance()
    {
        static Notepadqq instance;
        return instance;
    }

    static const QString version;
    static const QString contributorsUrl;
    static const QString website;
    static QString copyright();
    static QString appDataPath(QString fileName = QString());
    static QString editorPath();
    static QString extensionToolsPath();
    static QString nodejsPath();
    static QString npmPath();
    static QString fileNameFromUrl(const QUrl &url);
    static QSharedPointer<QCommandLineParser> getCommandLineArgumentsParser(const QStringList &arguments);

    static bool oldQt();
    static void setOldQt(bool oldQt);

    static void showQtVersionWarning(bool showCheckBox, QWidget *parent = 0);

    static QString extensionsPath();

signals:
    void newWindow(MainWindow *window);

private:
    Notepadqq() {}
    Notepadqq(Notepadqq const&);      // Don't implement
    void operator=(Notepadqq const&); // Don't implement

    static bool m_oldQt;
};

#endif // NOTEPADQQ_H
