#include "frmpreferences.h"
#include "ui_frmpreferences.h"
#include "mainwindow.h"
#include "appwidesettings.h"
frmpreferences::frmpreferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmpreferences)
{
    ui->setupUi(this);
}

frmpreferences::~frmpreferences()
{
    delete ui;
}

//void frmpreferences::on_toggle_tabbar_hide()
//{

//}
