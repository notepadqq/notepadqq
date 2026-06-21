#include "include/frmindentationmode.h"

#include "include/EditorNS/editor_properties.h"
#include "ui_frmindentationmode.h"

frmIndentationMode::frmIndentationMode(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmIndentationMode)
{
    ui->setupUi(this);
}

frmIndentationMode::~frmIndentationMode()
{
    delete ui;
}

void frmIndentationMode::populateWidgets(EditorNS::IndentationMode indentationMode)
{
    ui->radTabs->setChecked(indentationMode.useTabs);
    ui->radSpaces->setChecked(!indentationMode.useTabs);
    ui->txtLanguages_TabSize->setValue(indentationMode.size);
}

EditorNS::IndentationMode frmIndentationMode::indentationMode()
{
    EditorNS::IndentationMode indent;
    indent.useTabs = ui->radTabs->isChecked();
    indent.size = ui->txtLanguages_TabSize->value();
    return indent;
}

void frmIndentationMode::on_buttonBox_accepted()
{
    accept();
}

void frmIndentationMode::on_buttonBox_rejected()
{
    reject();
}
