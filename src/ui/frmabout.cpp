#include "include/frmabout.h"
#include "ui_frmabout.h"
#include "include/constants.h"
#include <QDesktopServices>

frmAbout::frmAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmAbout)
{
    ui->setupUi(this);

    ui->lblVersion->setText("v" + VERSION);
    ui->lblCopyright->setText(COPYRIGHT);

    ui->lblContributors->setText(tr("Authors:") + " <a href=\"" + MEMBERS_URL + "\"><span style=\"text-decoration: underline; color:#0000ff;\">" + tr("GitHub Contributors") + "</span></a>");

    setFixedSize(this->width(), this->height());
    setWindowFlags( (windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);
}

frmAbout::~frmAbout()
{
    delete ui;
}

void frmAbout::on_lblContributors_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(MEMBERS_URL, QUrl::TolerantMode));
}

void frmAbout::on_pushButton_clicked()
{
    this->close();
}
