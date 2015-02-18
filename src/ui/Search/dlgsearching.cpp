#include "include/Search/dlgsearching.h"
#include "ui_dlgsearching.h"

dlgSearching::dlgSearching(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgSearching)
{
    ui->setupUi(this);

    setFixedSize(this->width(), this->height());
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMinMaxButtonsHint);

    QFont f = ui->lblTitle->font();
    f.setPointSizeF(f.pointSizeF() * 1.2);
    ui->lblTitle->setFont(f);
}

dlgSearching::~dlgSearching()
{
    delete ui;
}

void dlgSearching::setTitle(const QString &title)
{
    ui->lblTitle->setText(title);
}

QString dlgSearching::title() const
{
    return ui->lblTitle->text();
}

void dlgSearching::setText(const QString &content)
{
    ui->lblContent->setText(content);
}

QString dlgSearching::text() const
{
    return ui->lblContent->text();
}

void dlgSearching::on_btnCancel_clicked()
{
    reject();
}
