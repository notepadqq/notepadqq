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

#include "frmabout.h"
#include "ui_frmabout.h"
#include "constants.h"

frmAbout::frmAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmAbout)
{
    ui->setupUi(this);

    ui->lblVersion->setText("v" + VERSION);
    ui->lblCopyright->setText(COPYRIGHT);
    ui->lblUrl->setText("<a href=\"" + URL + "\">" + URL + "</a>");
    ui->lblThanks->setText(ui->lblThanks->text().replace("{thanks to}", tr("thanks to...")));

    setFixedSize(this->width(), this->height());
    setWindowFlags( (windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);
}

frmAbout::~frmAbout()
{
    delete ui;
}

void frmAbout::on_lblUrl_linkActivated()
{
    QDesktopServices::openUrl(QUrl(URL, QUrl::TolerantMode));
}

void frmAbout::on_lblThanks_linkActivated()
{
    ui->txtGpl->setText(tr("Thanks to:") + "\n\n" +
                        "Don HO (notepad++)\n" +
                        "Everaldo.com (crystal icons)\n" +
                        "");
}

void frmAbout::on_pushButton_clicked()
{
    this->close();
}
