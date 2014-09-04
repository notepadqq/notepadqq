#include "include/frmsearchreplace.h"
#include "ui_frmsearchreplace.h"
#include <QLineEdit>
#include <QMessageBox>

frmSearchReplace::frmSearchReplace(TopEditorContainer *topEditorContainer, Tabs defaultTab, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmSearchReplace), m_topEditorContainer(topEditorContainer)
{
    ui->setupUi(this);

    setFixedSize(this->width(), this->height());
    setWindowFlags( (windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);

    if (defaultTab == TabSearch) {
        ui->tabWidget->setCurrentWidget(ui->tabSearch);
    } else if (defaultTab == TabReplace) {
        ui->tabWidget->setCurrentWidget(ui->tabReplace);
    }

    ui->tabWidget->currentChanged(ui->tabWidget->currentIndex());

    connect(ui->cmbSearch->lineEdit(), &QLineEdit::returnPressed, this, &frmSearchReplace::on_btnFindNext_clicked);
    connect(ui->cmbSearch_2->lineEdit(), &QLineEdit::returnPressed, this, &frmSearchReplace::on_btnFindNext_3_clicked);
    connect(ui->cmbReplace->lineEdit(), &QLineEdit::returnPressed, this, &frmSearchReplace::on_btnReplaceNext_clicked);
}

frmSearchReplace::~frmSearchReplace()
{
    delete ui;
}

Editor *frmSearchReplace::currentEditor()
{
    return this->m_topEditorContainer->currentTabWidget()->currentEditor();
}

QString frmSearchReplace::plainTextToRegex(QString text)
{
    // Transform it into a regex, but make sure to escape special chars
    QString regex = QRegExp::escape(text);
    return regex;
}

void frmSearchReplace::search(QString string, bool isRegex, bool forward) {
    QString rawSearch;

    if (isRegex) {
        rawSearch = string;
    } else {
        rawSearch = plainTextToRegex(string);
    }

    QList<QVariant> data = QList<QVariant>();
    data.append(rawSearch);
    data.append(forward);
    currentEditor()->sendMessage("C_FUN_SEARCH", QVariant::fromValue(data));
}

void frmSearchReplace::replace(QString string, QString replacement, bool isRegex, bool forward) {
    QString rawSearch;

    if (isRegex) {
        rawSearch = string;
    } else {
        rawSearch = plainTextToRegex(string);
    }

    QList<QVariant> data = QList<QVariant>();
    data.append(rawSearch);
    data.append(replacement);
    data.append(forward);
    currentEditor()->sendMessage("C_FUN_REPLACE", QVariant::fromValue(data));
}

int frmSearchReplace::replaceAll(QString string, QString replacement, bool isRegex) {
    QString rawSearch;

    if (isRegex) {
        rawSearch = string;
    } else {
        rawSearch = plainTextToRegex(string);
    }

    QList<QVariant> data = QList<QVariant>();
    data.append(rawSearch);
    data.append(replacement);
    QVariant count = currentEditor()->sendMessageWithResult("C_FUN_REPLACE_ALL", QVariant::fromValue(data));
    return count.toInt();
}

int frmSearchReplace::selectAll(QString string, bool isRegex) {
    QString rawSearch;

    if (isRegex) {
        rawSearch = string;
    } else {
        rawSearch = plainTextToRegex(string);
    }

    QList<QVariant> data = QList<QVariant>();
    data.append(rawSearch);
    QVariant count = currentEditor()->sendMessageWithResult("C_FUN_SEARCH_SELECT_ALL", QVariant::fromValue(data));
    return count.toInt();
}

void frmSearchReplace::on_btnFindNext_clicked()
{
    this->search(ui->cmbSearch->currentText(), false, true);
}

void frmSearchReplace::on_btnFindPrev_clicked()
{
    this->search(ui->cmbSearch->currentText(), false, false);
}

void frmSearchReplace::on_btnFindNext_3_clicked()
{
    this->search(ui->cmbSearch->currentText(), false, true);
}

void frmSearchReplace::on_btnFindPrev_3_clicked()
{
    this->search(ui->cmbSearch->currentText(), false, false);
}

void frmSearchReplace::on_tabWidget_currentChanged(int index)
{
    if (index == ui->tabWidget->indexOf(ui->tabSearch)) {
        ui->cmbSearch->setFocus();
    } else if (index == ui->tabWidget->indexOf(ui->tabReplace)) {
        ui->cmbSearch_2->setFocus();
    }
}

void frmSearchReplace::on_btnReplaceNext_clicked()
{
    this->replace(ui->cmbSearch_2->currentText(),
                  ui->cmbReplace->currentText(),
                  false,
                  true);
}

void frmSearchReplace::on_btnReplacePrev_clicked()
{
    this->replace(ui->cmbSearch_2->currentText(),
                  ui->cmbReplace->currentText(),
                  false,
                  false);
}

void frmSearchReplace::on_btnReplaceAll_clicked()
{
    int n = this->replaceAll(ui->cmbSearch_2->currentText(),
                             ui->cmbReplace->currentText(),
                             false);
    QMessageBox::information(this, tr("Replace all"), tr("%1 occurrences have been replaced.").arg(n));
}

void frmSearchReplace::on_btnSelectAll_clicked()
{
    int count = this->selectAll(ui->cmbSearch->currentText(), false);
    if (count == 0) {
        QMessageBox::information(this, tr("Select all"), tr("No results found"));
    } else {
        // Focus on main window
        this->m_topEditorContainer->activateWindow();
    }
}
