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

#ifndef FRMABOUT_H
#define FRMABOUT_H

#include <QDesktopServices>
#include <QDialog>
#include <QUrl>

namespace Ui {
    class frmAbout;
}

class frmAbout : public QDialog
{
    Q_OBJECT

public:
    explicit frmAbout(QWidget *parent = 0);
    ~frmAbout();

private:
    Ui::frmAbout *ui;

private slots:
    void on_pushButton_clicked();
    void on_lblUrl_linkActivated();
    void on_lblThanks_linkActivated();
};

#endif // FRMABOUT_H
