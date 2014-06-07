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

#include "userlexer.h"
#include <Qsci/qscilexercustom.h>
#include <Qsci/qsciscintilla.h>

userLexer::userLexer(QObject *parent) :
    QsciLexerCustom(parent)
{

}

const char* userLexer::language() const
{
        return "AsciiDoc";
}

QString userLexer::description(int) const
{
        return QString();
}

//void userLexer::styleText(int start, int end)
void userLexer::styleText(int, int)
{
    //editor()->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE,start, end, chars);
}

QColor userLexer::defaultColor(int)
{
        return QColor(0xe0, 0x0, 0x0);
}

QFont  userLexer::defaultFont(int)
{
        return QFont("Courier New", 10);
}

QColor userLexer::defaultPaper(int style)
{
        return QsciLexer::defaultPaper(style);
}
