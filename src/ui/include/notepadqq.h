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

#define POINTVERSION "0.30.0" // major.minor.revision

class Notepadqq
{
public:
    static const QString version;
    static const QString membersUrl;
    static QString copyright();
    static QString editorPath();
private:
    static QString m_cachedEditorPath;
};

#endif // NOTEPADQQ_H
