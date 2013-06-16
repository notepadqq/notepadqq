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
#include <QCoreApplication>


#define SINGLEINSTANCE_EXPERIMENTAL true // When true, enable the experimental function for the "single instance" system


const QString VERSION = "0.20.0"; // major.minor.revision
const QString COPYRIGHT = QObject::trUtf8("Copyright © 2010-2012, the Notepadqq team");
const QString URL = "http://notepadqq.sourceforge.net/";


inline QString ApplicationL10nDir() {
#ifdef Q_OS_WIN1DOWS
    QString def = "NOT IMPLEMENTED";
#else
    QString def = "/usr/share/notepadqq/L10n";
#endif
    if(!QDir(def).exists()) def = QCoreApplication::applicationDirPath() + "/L10n";
    return def;
}





/*** DON'T TOUCH ANYTHING AFTER HERE. You are likely to be eaten by a grue if you do. ***/
const int SELECTOR_DefaultSelectionHighlight = 8;
const QString INSTANCESERVER_ID = "notepadqq-{38fe96c0-030a-11e0-a976-0800200c9a66}"
                                  + QProcessEnvironment::systemEnvironment().value("USER", "")
                                  + QProcessEnvironment::systemEnvironment().value("USERNAME", "");


#endif // CONSTANTS_H
