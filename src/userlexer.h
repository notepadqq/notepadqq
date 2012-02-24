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

#ifndef USERLEXER_H
#define USERLEXER_H

#include <QObject>
#include <Qsci/qscilexercustom.h>

class userLexer : public QsciLexerCustom
{
    Q_OBJECT
public:
    explicit userLexer(QObject *parent = 0);
    const char *language() const;
    QString description(int) const;
    void styleText(int start, int end);
    QColor defaultColor(int);
    QFont  defaultFont(int);
    QColor defaultPaper(int);
signals:

public slots:

};

#endif // USERLEXER_H
