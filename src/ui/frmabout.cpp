#include "include/frmabout.h"

#include "include/iconprovider.h"
#include "include/notepadqq.h"
#include "ui_frmabout.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>

frmAbout::frmAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmAbout)
{
    ui->setupUi(this);

    ui->lblIcon->setPixmap(IconProvider::fromTheme("notepadqq")
                           .pixmap(ui->lblIcon->width(),
                                   ui->lblIcon->height()));

    ui->lblVersion->setText("v" + QApplication::applicationVersion());
    ui->lblCopyright->setText(Notepadqq::copyright());

    QString linkStyle = "text-decoration: none; color:#606060;";
    ui->lblContributors->setText(tr("Contributors:") + " <a href=\"" + Notepadqq::contributorsUrl + "\"><span style=\"" + linkStyle + "\">" + tr("GitHub Contributors") + "</span></a>");
    ui->lblWebsite->setText("<a href=\"" + Notepadqq::website + "\"><span style=\"" + linkStyle + "\">" + Notepadqq::website + "</span></a>");

    ui->btnLicense->setStyleSheet("QPushButton {color: black;}");
    ui->pushButton->setStyleSheet("QPushButton {color: black;}");

    setFixedSize(this->width(), this->height());
    setWindowFlags( (windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);
}

frmAbout::~frmAbout()
{
    delete ui;
}

void frmAbout::on_lblContributors_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link, QUrl::TolerantMode));
}

void frmAbout::on_pushButton_clicked()
{
    this->close();
}

void frmAbout::on_btnLicense_clicked()
{
    QMessageBox license;
    license.setText(R"DELIM(<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
                    <html><head><style type="text/css">
                    p, li { white-space: pre-wrap; }
                    </style></head><body>
                    <p>This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.</p>
                    <p>This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</p>
                    <p>You should have received a copy of the GNU General Public License along with this program. If not, see &lt;http://www.gnu.org/licenses/&gt;.</p>
                    </body></html>)DELIM");
    license.exec();
}
