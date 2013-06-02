#include "frmsrchreplace.h"
#include "ui_frmsrchreplace.h"

frmsrchreplace::frmsrchreplace(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmsrchreplace)
{
    ui->setupUi(this);
    connect(ui->btnCount,SIGNAL(clicked()),this,SLOT(buttonCount_triggered()));

}

frmsrchreplace::~frmsrchreplace()
{
    delete ui;
}

void frmsrchreplace::buttonCount_triggered()
{
    return;
}
