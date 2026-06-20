#include "include/frmcolumneditor.h"
#include "include/EditorNS/editor.h"
#include "ui_frmcolumneditor.h"

frmColumnEditor::frmColumnEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmColumnEditor)
{
    ui->setupUi(this);
    ui->lineEdit->setFocus();
}

QString frmColumnEditor::insTxt() const
{
    return ui->lineEdit->text();
}

frmColumnEditor::~frmColumnEditor()
{
    delete ui;
}

void frmColumnEditor::on_buttonBox_accepted()
{
    accept();
}

void frmColumnEditor::on_buttonBox_rejected()
{
    reject();
}
