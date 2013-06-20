#include "frmsrchreplace.h"
#include "ui_frmsrchreplace.h"
#include <QMessageBox>
#include "mainwindow.h"
#include "qsciscintillaqq.h"
#include "searchengine.h"

frmsrchreplace::frmsrchreplace(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmsrchreplace)
{
    ui->setupUi(this);
    //Pretty button signals
    connect(ui->btn_countInstances,SIGNAL(clicked()),this,SLOT(buttonCountInstances_clicked()));
    connect(ui->btn_findNext,SIGNAL(clicked()),this,SLOT(buttonFindNext_clicked()));
    connect(ui->btn_findNext_r,SIGNAL(clicked()),this,SLOT(buttonFindNext_clicked()));
    connect(ui->btn_replace,SIGNAL(clicked()),this,SLOT(buttonReplace_clicked()));
    connect(ui->btn_replaceAll,SIGNAL(clicked()),this,SLOT(buttonReplaceAll_clicked()));

    //Signals to let the search engine know we changed parameters
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(updateMode(int)));
    connect(ui->cb_optMatchCase,SIGNAL(clicked()),this, SLOT(setNewSearch()));
    connect(ui->cb_optMatchWhole,SIGNAL(toggled(bool)),this, SLOT(setNewSearch()));
    connect(ui->cb_optWrap,SIGNAL(toggled(bool)),this,SLOT(setNewSearch()));
    connect(ui->rb_up,SIGNAL(toggled(bool)),this,SLOT(setNewSearch()));
    connect(ui->rb_up,SIGNAL(toggled(bool)),this,SLOT(setNewSearch()));
    connect(ui->rb_srch_ext,SIGNAL(toggled(bool)),this,SLOT(setNewSearch()));
    connect(ui->rb_srch_normal,SIGNAL(toggled(bool)),this,SLOT(setNewSearch()));
    connect(ui->rb_srch_regexp,SIGNAL(toggled(bool)),this,SLOT(setNewSearch()));
    connect(ui->edt_findWhat,SIGNAL(textEdited(QString)),this,SLOT(setNewSearch()));

    ui->edt_findWhat->setFocus();
    //Important variable to pass if search parameters are to change.
    newsearch = false;
}

frmsrchreplace::~frmsrchreplace()
{
    delete ui;
}

searchengine* frmsrchreplace::se()
{
    MainWindow* mref = MainWindow::instance();
    if(QString(mref->metaObject()->className()).compare("MainWindow") == 0) {
        return mref->getSearchEngine();
    }
    return 0;
}

void frmsrchreplace::setNewSearch(bool isnew)
{
    se()->setNewSearch(isnew);
}

void frmsrchreplace::closeEvent(QCloseEvent *e)
{
    MainWindow::instance()->clearSearchDialog();
    QDialog::closeEvent(e);
}

void frmsrchreplace::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
}

// Keep persistent options synced across the user interface, may need to find a better way to do this later on.
void frmsrchreplace::updateMode(int newIndex)
{
    ui->opts->setParent(ui->tabWidget->widget(newIndex));
    ui->findParam->setParent(ui->tabWidget->widget(newIndex));
    ui->opts->show();
    ui->findParam->show();

    if(newIndex == 2) {
        ui->cb_optWrap->setEnabled(false);
    }else {
        ui->cb_optWrap->setEnabled(true);
    }
}

void frmsrchreplace::buttonCountInstances_clicked()
{
    QMessageBox mb;

    updateParameters();

    if(se()->pattern().length() == 0) {
        ui->edt_findWhat->setFocus();
        return;
    }
    mb.setText(QString("Found %1 occurrences of %2").arg(se()->countOccurrences()).arg(se()->pattern()));
    mb.exec();
}

//Need a cleaner way to do this later
void frmsrchreplace::buttonFindNext_clicked()
{
    QsciScintillaqq *sci = MainWindow::instance()->focused_editor();
    updateParameters();
    se()->setContext(sci);
    se()->findString();
}

void frmsrchreplace::buttonReplace_clicked()
{
    updateParameters();
    se()->setContext(MainWindow::instance()->focused_editor());
    se()->replace(ui->edt_replaceWith_r->text());
}

void frmsrchreplace::buttonReplaceAll_clicked()
{
    QMessageBox mb;

    updateParameters();
    se()->setContext(MainWindow::instance()->container->focusQTabWidgetqq()->focusQSciScintillaqq());
    int occurrences = se()->replace(ui->edt_replaceWith_r->text(),true);

    if(occurrences > 0) {
        mb.setText(QString("Replaced %1 occurrences of %2").arg(occurrences).arg(se()->pattern()));
        mb.setWindowTitle(QCoreApplication::applicationName());
        mb.setStandardButtons(QMessageBox::Ok);
        mb.exec();
    }
}

void frmsrchreplace::updateParameters()
{
    QsciScintillaqq *sci = MainWindow::instance()->focused_editor();

    se()->setCaseSensitive(ui->cb_optMatchCase->isChecked());
    se()->setWholeWord(ui->cb_optMatchWhole->isChecked());
    se()->setForward(ui->rb_down->isChecked());
    se()->setRegExp(ui->rb_srch_regexp->isChecked());
    se()->setWrap(ui->cb_optWrap->isChecked());
    se()->setPattern(ui->edt_findWhat->text());
    se()->setHaystack(sci->text());
}
