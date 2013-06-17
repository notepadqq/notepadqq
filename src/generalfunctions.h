/*
 *
 * This file is part of the Notepadqq text editor.
 *
 * Copyright(c) Notepadqq team.
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

#ifndef GENERALFUNCTIONS_H
#define GENERALFUNCTIONS_H

#include <QObject>
#include <QString>

class generalFunctions
{
public:
    generalFunctions();
    static QString getFileMime(QString file);
    static QString getFileEncoding(QString file);
    static QString readDConfKey(QString schema, QString key);
    static QString getUserFilePath(QString relativePath);
private:
    static QString getOutputFromFileMimeCmd(QString file, QString mimeArg);
};

#endif // GENERALFUNCTIONS_H
