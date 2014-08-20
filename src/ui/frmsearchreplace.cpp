#include "include/frmsearchreplace.h"
#include "ui_frmsearchreplace.h"

frmSearchReplace::frmSearchReplace(TopEditorContainer *topEditorContainer, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmSearchReplace), m_topEditorContainer(topEditorContainer)
{
    ui->setupUi(this);
}

frmSearchReplace::~frmSearchReplace()
{
    delete ui;
}

Editor *frmSearchReplace::currentEditor()
{
    return this->m_topEditorContainer->currentTabWidget()->currentEditor();
}

void frmSearchReplace::search(QString string, bool isRegex, bool forward) {
    QString rawSearch;

    if(isRegex) {
       rawSearch = string;
    } else {
        // TODO Transform it in a regex
        rawSearch = string;
    }


    QList<QVariant> data = QList<QVariant>();
    data.append(rawSearch);
    data.append(forward);
    currentEditor()->sendMessage("C_FUN_SEARCH", QVariant::fromValue(data));
}

void frmSearchReplace::on_btnFindNext_clicked()
{
    this->search(ui->cmbSearch->currentText(), true, true);
}

void frmSearchReplace::on_btnFindPrev_clicked()
{
    this->search(ui->cmbSearch->currentText(), true, false);
}
