#include "include/frmsearchreplace.h"
#include "include/iconprovider.h"
#include "ui_frmsearchreplace.h"
#include <QLineEdit>
#include <QMessageBox>

frmSearchReplace::frmSearchReplace(TopEditorContainer *topEditorContainer, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmSearchReplace), m_topEditorContainer(topEditorContainer)
{
    ui->setupUi(this);

    //setFixedSize(this->width(), this->height());
    setWindowFlags( (windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);

    move(
        parentWidget()->window()->frameGeometry().topLeft() +
        parentWidget()->window()->rect().center() -
        rect().center());

    connect(ui->cmbSearch->lineEdit(), &QLineEdit::returnPressed, this, &frmSearchReplace::on_btnFindNext_clicked);
    connect(ui->cmbReplace->lineEdit(), &QLineEdit::returnPressed, this, &frmSearchReplace::on_btnReplaceNext_clicked);

    ui->actionFind->setIcon(IconProvider::fromTheme("edit-find"));
    ui->actionReplace->setIcon(IconProvider::fromTheme("edit-find-replace"));

    ui->actionFind->setChecked(true);
    ui->actionReplace->toggled(false);
    ui->actionFind->toggled(true);
    ui->chkShowAdvanced->toggled(ui->chkShowAdvanced->isChecked());
}

frmSearchReplace::~frmSearchReplace()
{
    delete ui;
}

void frmSearchReplace::show(Tabs defaultTab)
{
    setCurrentTab(defaultTab);
    QMainWindow::show();
}

void frmSearchReplace::setCurrentTab(Tabs tab)
{
    if (tab == TabSearch) {
        ui->actionFind->setChecked(true);
    } else if (tab == TabReplace) {
        ui->actionReplace->setChecked(true);
    }
}

Editor *frmSearchReplace::currentEditor()
{
    return this->m_topEditorContainer->currentTabWidget()->currentEditor();
}

QString frmSearchReplace::plainTextToRegex(QString text, bool matchWholeWord)
{
    // Transform it into a regex, but make sure to escape special chars
    QString regex = QRegExp::escape(text);

    if (matchWholeWord)
        regex = "\\b" + regex + "\\b";

    return regex;
}

QString frmSearchReplace::rawSearchString(QString search, SearchMode searchMode, SearchOptions searchOptions)
{
    QString rawSearch;

    if (searchMode == SearchMode::Regex) {
        rawSearch = search;
    } else if (searchMode == SearchMode::SpecialChars) {
        bool wholeWord = searchOptions.MatchWholeWord;
        rawSearch = plainTextToRegex(search, wholeWord);
        // Replace '\\' with '\' (basically, we unescape escaped slashes)
        rawSearch = rawSearch.replace("\\\\", "\\");
    } else {
        bool wholeWord = searchOptions.MatchWholeWord;
        rawSearch = plainTextToRegex(search, wholeWord);
    }

    return rawSearch;
}

QString frmSearchReplace::regexModifiersFromSearchOptions(SearchOptions searchOptions)
{
    QString modifiers = "m";
    if (!searchOptions.MatchCase)
        modifiers.append("i");

    return modifiers;
}

void frmSearchReplace::search(QString string, SearchMode searchMode, bool forward, SearchOptions searchOptions) {
    QString rawSearch = rawSearchString(string, searchMode, searchOptions);

    QList<QVariant> data = QList<QVariant>();
    data.append(rawSearch);
    data.append(regexModifiersFromSearchOptions(searchOptions));
    data.append(forward);
    currentEditor()->sendMessage("C_FUN_SEARCH", QVariant::fromValue(data));
}

void frmSearchReplace::replace(QString string, QString replacement, SearchMode searchMode, bool forward, SearchOptions searchOptions) {
    QString rawSearch = rawSearchString(string, searchMode, searchOptions);

    QList<QVariant> data = QList<QVariant>();
    data.append(rawSearch);
    data.append(regexModifiersFromSearchOptions(searchOptions));
    data.append(forward);
    data.append(replacement);
    currentEditor()->sendMessage("C_FUN_REPLACE", QVariant::fromValue(data));
}

int frmSearchReplace::replaceAll(QString string, QString replacement, SearchMode searchMode, SearchOptions searchOptions) {
    QString rawSearch = rawSearchString(string, searchMode, searchOptions);

    QList<QVariant> data = QList<QVariant>();
    data.append(rawSearch);
    data.append(regexModifiersFromSearchOptions(searchOptions));
    data.append(replacement);
    QVariant count = currentEditor()->sendMessageWithResult("C_FUN_REPLACE_ALL", QVariant::fromValue(data));
    return count.toInt();
}

int frmSearchReplace::selectAll(QString string, SearchMode searchMode, SearchOptions searchOptions) {
    QString rawSearch = rawSearchString(string, searchMode, searchOptions);

    QList<QVariant> data = QList<QVariant>();
    data.append(rawSearch);
    data.append(regexModifiersFromSearchOptions(searchOptions));
    QVariant count = currentEditor()->sendMessageWithResult("C_FUN_SEARCH_SELECT_ALL", QVariant::fromValue(data));
    return count.toInt();
}

frmSearchReplace::SearchMode frmSearchReplace::searchModeFromUI()
{
    SearchMode searchMode;

    if (ui->radSearchPlainText->isChecked())
        searchMode = SearchMode::PlainText;
    else if (ui->radSearchWithSpecialChars->isChecked())
        searchMode = SearchMode::SpecialChars;
    else if (ui->radSearchWithRegex->isChecked())
        searchMode = SearchMode::Regex;

    return searchMode;
}

frmSearchReplace::SearchOptions frmSearchReplace::searchOptionsFromUI()
{
    SearchOptions searchOptions;

    if (ui->chkMatchCase->isChecked())
        searchOptions.MatchCase = true;
    if (ui->chkMatchWholeWord->isChecked())
        searchOptions.MatchWholeWord = true;

    return searchOptions;
}

void frmSearchReplace::findFromUI(bool forward)
{
    this->search(ui->cmbSearch->currentText(),
                 searchModeFromUI(),
                 forward,
                 searchOptionsFromUI());
}

void frmSearchReplace::on_btnFindNext_clicked()
{
    this->findFromUI(true);
}

void frmSearchReplace::on_btnFindPrev_clicked()
{
    this->findFromUI(false);
}

void frmSearchReplace::on_btnReplaceNext_clicked()
{
    this->replace(ui->cmbSearch->currentText(),
                  ui->cmbReplace->currentText(),
                  searchModeFromUI(),
                  true,
                  searchOptionsFromUI());
}

void frmSearchReplace::on_btnReplacePrev_clicked()
{
    this->replace(ui->cmbSearch->currentText(),
                  ui->cmbReplace->currentText(),
                  searchModeFromUI(),
                  false,
                  searchOptionsFromUI());
}

void frmSearchReplace::on_btnReplaceAll_clicked()
{
    int n = this->replaceAll(ui->cmbSearch->currentText(),
                             ui->cmbReplace->currentText(),
                             searchModeFromUI(),
                             searchOptionsFromUI());
    QMessageBox::information(this, tr("Replace all"), tr("%1 occurrences have been replaced.").arg(n));
}

void frmSearchReplace::on_btnSelectAll_clicked()
{
    int count = this->selectAll(ui->cmbSearch->currentText(),
                                searchModeFromUI(),
                                searchOptionsFromUI());
    if (count == 0) {
        QMessageBox::information(this, tr("Select all"), tr("No results found"));
    } else {
        // Focus on main window
        this->m_topEditorContainer->activateWindow();
    }
}

void frmSearchReplace::on_actionReplace_toggled(bool on)
{
    ui->actionFind->setChecked(!on);

    ui->btnReplaceAll->setVisible(on);
    ui->btnReplaceNext->setVisible(on);
    ui->btnReplacePrev->setVisible(on);
    ui->cmbReplace->setVisible(on);
    ui->lblReplace->setVisible(on);

    manualSizeAdjust();
}

void frmSearchReplace::on_actionFind_toggled(bool on)
{
    ui->actionReplace->setChecked(!on);

    manualSizeAdjust();
}

void frmSearchReplace::manualSizeAdjust()
{
    int curX = geometry().x();
    int curY = geometry().y();
    //setFixedSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    QApplication::processEvents();
    QApplication::processEvents();
    setGeometry(curX, curY, width(), 0);
    /*QApplication::processEvents();
    QApplication::processEvents();
    setFixedSize(width(), height());
    QApplication::processEvents();
    QApplication::processEvents();
    setGeometry(curX, curY, width(), height());*/
}

void frmSearchReplace::on_chkShowAdvanced_toggled(bool checked)
{
    if (checked)
        ui->groupAdvanced->show();
    else
        ui->groupAdvanced->hide();

    manualSizeAdjust();
}

void frmSearchReplace::on_radSearchWithRegex_toggled(bool checked)
{
    if (checked) {
        ui->chkMatchWholeWord->setChecked(false);
        ui->chkMatchWholeWord->setEnabled(false);

        manualSizeAdjust();
    }
}

void frmSearchReplace::on_radSearchPlainText_toggled(bool checked)
{
    if (checked) {
        ui->chkMatchWholeWord->setEnabled(true);

        manualSizeAdjust();
    }
}

void frmSearchReplace::on_radSearchWithSpecialChars_toggled(bool checked)
{
    if (checked) {
        ui->chkMatchWholeWord->setChecked(false);
        ui->chkMatchWholeWord->setEnabled(false);

        manualSizeAdjust();
    }
}
