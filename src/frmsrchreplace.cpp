#include "frmsrchreplace.h"
#include "ui_frmsrchreplace.h"
#include <QMessageBox>
#include "mainwindow.h"
#include "qsciscintillaqq.h"

frmsrchreplace::frmsrchreplace(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmsrchreplace)
{
    ui->setupUi(this);
    //Pretty button signals
    connect(ui->btn_countInstances,SIGNAL(clicked()),this,SLOT(buttonCountInstances_clicked()));
    connect(ui->btn_findNext,SIGNAL(clicked()),this,SLOT(buttonFindNext_clicked()));

    //Signals to let the search engine know we changed parameters
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(updateMode(int)));
    connect(ui->cb_optMatchCase,SIGNAL(clicked()),this, SLOT(searchChanged()));
    connect(ui->cb_optMatchWhole,SIGNAL(toggled(bool)),this, SLOT(searchChanged()));
    connect(ui->cb_optWrap,SIGNAL(toggled(bool)),this,SLOT(searchChanged()));
    connect(ui->rb_up,SIGNAL(toggled(bool)),this,SLOT(searchChanged()));
    connect(ui->rb_up,SIGNAL(toggled(bool)),this,SLOT(searchChanged()));
    connect(ui->rb_srch_ext,SIGNAL(toggled(bool)),this,SLOT(searchChanged()));
    connect(ui->rb_srch_normal,SIGNAL(toggled(bool)),this,SLOT(searchChanged()));
    connect(ui->rb_srch_regexp,SIGNAL(toggled(bool)),this,SLOT(searchChanged()));
    connect(ui->edt_findWhat,SIGNAL(textEdited(QString)),this,SLOT(searchChanged()));

    //Important variable to pass if search parameters are to change.
    newsearch = false;
}

frmsrchreplace::~frmsrchreplace()
{
    delete ui;
}

void frmsrchreplace::searchChanged()
{
    newsearch = true;
}

// Keep persistent options synced across the user interface, may need to find a better way to do this later on.
void frmsrchreplace::updateMode(int newIndex)
{
    ui->opts->setParent(ui->tabWidget->widget(newIndex));
    ui->opts->show();
    if(newIndex == 2) {
        ui->cb_optWrap->setEnabled(false);
    }else {
        ui->cb_optWrap->setEnabled(true);
    }
}

Qt::CaseSensitivity frmsrchreplace::matchCase()
{
    return (ui->cb_optMatchCase->checkState() == Qt::Checked) ? Qt::CaseSensitive : Qt::CaseInsensitive;
}

void frmsrchreplace::buttonCountInstances_clicked()
{
    MainWindow *mref = (MainWindow*)this->parent();
    QsciScintillaqq *sci = mref->container->focusQTabWidgetqq()->focusQSciScintillaqq();
    if(ui->edt_findWhat->text().length() == 0) {
        ui->edt_findWhat->setFocus();
        return;
    }
    sci->countFinds(ui->edt_findWhat->text(),matchCase());
}

void frmsrchreplace::buttonFindNext_clicked()
{
    bool regexp    = ui->rb_srch_regexp->isChecked();
    bool casesense = ui->cb_optMatchCase->isChecked();
    bool wholeword = ui->cb_optMatchWhole->isChecked();
    bool wrap      = ui->cb_optWrap->isChecked();
    bool forward   = ui->rb_down->isChecked();
    MainWindow *mref = (MainWindow*)this->parent();
    QsciScintillaqq *sci = mref->container->focusQTabWidgetqq()->focusQSciScintillaqq();
    qDebug() << newsearch;
    sci->findString(ui->edt_findWhat->text(), regexp,casesense,wholeword,wrap,forward,newsearch);
    newsearch = false;
}
