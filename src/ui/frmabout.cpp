#include "include/frmabout.h"
#include "ui_frmabout.h"
#include "include/notepadqq.h"
#include <QDesktopServices>
#include <QUrl>

frmAbout::frmAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmAbout)
{
    ui->setupUi(this);

    ui->lblVersion->setText("v" + Notepadqq::version);
    ui->lblCopyright->setText(Notepadqq::copyright());

    ui->lblContributors->setText(tr("Authors:") + " <a href=\"" + Notepadqq::contributorsUrl + "\"><span style=\"text-decoration: underline; color:#0000ff;\">" + tr("GitHub Contributors") + "</span></a>");

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
