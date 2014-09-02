#include "include/frmpreferences.h"
#include "ui_frmpreferences.h"
#include <QSettings>

frmPreferences::frmPreferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmPreferences)
{
    ui->setupUi(this);

    QSettings s;

    ui->chkCheckQtVersionAtStartup->setChecked(s.value("checkQtVersionAtStartup", true).toBool());
}

frmPreferences::~frmPreferences()
{
    delete ui;
}

void frmPreferences::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem * /*previous*/)
{
    int index = ui->treeWidget->indexOfTopLevelItem(current);

    if (index != -1) {
        ui->stackedWidget->setCurrentIndex(index);
    }
}

void frmPreferences::on_buttonBox_accepted()
{
    QSettings s;
    s.setValue("checkQtVersionAtStartup", ui->chkCheckQtVersionAtStartup->isChecked());

    accept();
}

void frmPreferences::on_buttonBox_rejected()
{
    reject();
}
