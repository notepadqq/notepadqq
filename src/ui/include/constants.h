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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QProcessEnvironment>
#include <QApplication>


//#define SINGLEINSTANCE_EXPERIMENTAL true // When true, enable the experimental function for the "single instance" system


#define POINTVERSION "0.30.0" // major.minor.revision
const QString VERSION = POINTVERSION;
const QString COPYRIGHT = QObject::trUtf8("Copyright © 2010-2014, Daniele Di Sarli");
const QString MEMBERS_URL = "https://github.com/notepadqq/notepadqq/network/members";


/*inline QString ApplicationL10nDir() {
#ifdef Q_OS_WIN1DOWS
    QString def = "NOT IMPLEMENTED";
#else
    //QString def = "/usr/share/notepadqq/L10n";
    QString def = QString("%1/../share/%2/L10n").arg(qApp->applicationDirPath()).arg(qApp->applicationName().toLower());
#endif
    //if(!QDir(def).exists()) def = QCoreApplication::applicationDirPath() + "/L10n";
    if(!QDir(def).exists()) def = qApp->applicationDirPath() + "/L10n";
    return def;
}*/

inline QString ApplicationEditorPath() {
    QString def = QString("%1/editor/index.html").arg(qApp->applicationDirPath());
    return def;
}

#endif // CONSTANTS_H
