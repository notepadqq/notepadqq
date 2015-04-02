#include "include/frmlinenumberchooser.h"
#include "ui_frmlinenumberchooser.h"

frmLineNumberChooser::frmLineNumberChooser(int min, int max, int defaultValue, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmLineNumberChooser)
{
    ui->setupUi(this);

    QFont f = ui->lblTitle->font();
    f.setPointSizeF(f.pointSizeF() * 1.2);
    ui->lblTitle->setFont(f);

    setFixedSize(this->width(), this->height());
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMinMaxButtonsHint);

    ui->spinLine->setMinimum(min);
    ui->spinLine->setMaximum(max);
    ui->spinLine->setValue(defaultValue);
    ui->spinLine->selectAll();
}

frmLineNumberChooser::~frmLineNumberChooser()
{
    delete ui;
}

int frmLineNumberChooser::value()
{
    return ui->spinLine->value();
}
