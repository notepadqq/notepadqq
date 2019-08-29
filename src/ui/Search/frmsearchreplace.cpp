#include "include/Search/frmsearchreplace.h"

#include "include/Search/searchstring.h"
#include "include/iconprovider.h"
#include "include/nqqsettings.h"
#include "ui_frmsearchreplace.h"

#include <QCompleter>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QThread>

frmSearchReplace::frmSearchReplace(TopEditorContainer *topEditorContainer, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmSearchReplace),
    m_topEditorContainer(topEditorContainer)
{
    ui->setupUi(this);

    setWindowFlags( (windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);

    move(
        parentWidget()->window()->frameGeometry().topLeft() +
        parentWidget()->window()->rect().center() -
        rect().center());

    ui->cmbSearch->completer()->setCaseSensitivity(Qt::CaseSensitive);
    ui->cmbReplace->completer()->setCaseSensitivity(Qt::CaseSensitive);

    NqqSettings& s = NqqSettings::getInstance();

    ui->cmbSearch->addItems(s.Search.getSearchHistory());
    ui->cmbSearch->setCurrentText("");
    ui->cmbReplace->addItems(s.Search.getReplaceHistory());
    ui->cmbReplace->setCurrentText("");

    connect(ui->cmbSearch->lineEdit(), &QLineEdit::textEdited, this, &frmSearchReplace::on_searchStringEdited);
    connect(ui->cmbSearch->lineEdit(), &QLineEdit::returnPressed, this, &frmSearchReplace::on_btnFindNext_clicked);
    connect(ui->cmbReplace->lineEdit(), &QLineEdit::returnPressed, this, &frmSearchReplace::on_btnFindNext_clicked);

    connect(ui->actionAdvancedSearch, &QAction::triggered, this, &frmSearchReplace::toggleAdvancedSearch);

    ui->actionFind->setIcon(IconProvider::fromTheme("edit-find"));
    ui->actionReplace->setIcon(IconProvider::fromTheme("edit-find-replace"));

    QActionGroup *tabGroup = new QActionGroup(this);
    tabGroup->addAction(ui->actionFind);
    tabGroup->addAction(ui->actionReplace);
    tabGroup->addAction(ui->actionAdvancedSearch);
    tabGroup->setExclusive(true);

    // Initialize all the tabs
    ui->actionFind->setChecked(true);
    ui->actionReplace->setChecked(true);

    ui->chkShowAdvanced->toggled(ui->chkShowAdvanced->isChecked());

    setCurrentTab(TabSearch);
}

frmSearchReplace::~frmSearchReplace()
{
    delete ui;
}

void frmSearchReplace::keyPressEvent(QKeyEvent *evt)
{
    switch (evt->key())
    {
    case Qt::Key_Escape:
        close();
        break;
    default:
        QMainWindow::keyPressEvent(evt);
    }
}

void frmSearchReplace::show(Tabs defaultTab)
{
    setCurrentTab(defaultTab);
    ui->cmbSearch->setFocus();
    ui->cmbSearch->lineEdit()->selectAll();
    QMainWindow::show();
    manualSizeAdjust();
}

void frmSearchReplace::setSearchText(QString string)
{
    ui->cmbSearch->setCurrentText(string);

    /*
      Workaround for: https://bugreports.qt.io/browse/QTBUG-49165
      There's a bug where the combobox's current index is changed to 0 when
      the lineedit contains selected text that isn't yet in the box's history.
      That happens when we call setSearchText() followed by show().
      Workaround is to disable auto complete until the search box was manually edited
      which prevents the bug. Auto complete is enabled again in on_searchStringEdited.
    */
    ui->cmbSearch->setAutoCompletion(false);
}

void frmSearchReplace::setCurrentTab(Tabs tab)
{
    if (tab == TabSearch) {
        ui->actionFind->setChecked(true);
    } else if (tab == TabReplace) {
        ui->actionReplace->setChecked(true);
    }
}

QSharedPointer<Editor> frmSearchReplace::currentEditor()
{
    return this->m_topEditorContainer->currentTabWidget()->currentEditor();
}

QString frmSearchReplace::regexModifiersFromSearchOptions(SearchHelpers::SearchOptions searchOptions)
{
    QString modifiers = "m";
    if (!searchOptions.MatchCase)
        modifiers.append("i");

    return modifiers;
}

void frmSearchReplace::search(QString string, SearchHelpers::SearchMode searchMode, bool forward, SearchHelpers::SearchOptions searchOptions) {
    if (!string.isEmpty()) {
        QString rawSearch = SearchString::format(string, searchMode, searchOptions);

        auto editor = currentEditor();

        if (searchOptions.SearchFromStart) {
            editor->setCursorPosition(0, 0);
        }

        QList<QVariant> data = QList<QVariant>();
        data.append(rawSearch);
        data.append(regexModifiersFromSearchOptions(searchOptions));
        data.append(forward);
        editor->sendMessage("C_FUN_SEARCH", QVariant::fromValue(data));
    }
}

void frmSearchReplace::replace(QString string, QString replacement, SearchHelpers::SearchMode searchMode, bool forward, SearchHelpers::SearchOptions searchOptions) {
    if (!string.isEmpty()) {
        QString rawSearch = SearchString::format(string, searchMode, searchOptions);
        if (searchMode == SearchHelpers::SearchMode::SpecialChars) {
            replacement = SearchString::unescape(replacement);
        }

        auto editor = currentEditor();

        if (searchOptions.SearchFromStart) {
            editor->setCursorPosition(0, 0);
        }

        QList<QVariant> data = QList<QVariant>();
        data.append(rawSearch);
        data.append(regexModifiersFromSearchOptions(searchOptions));
        data.append(forward);
        data.append(replacement);
				data.append(QString::number(static_cast<int>(searchMode)));
        editor->sendMessage("C_FUN_REPLACE", QVariant::fromValue(data));
    }
}

int frmSearchReplace::replaceAll(QString string, QString replacement, SearchHelpers::SearchMode searchMode, SearchHelpers::SearchOptions searchOptions) {
    QString rawSearch = SearchString::format(string, searchMode, searchOptions);
    if (searchMode == SearchHelpers::SearchMode::SpecialChars) {
            replacement = SearchString::unescape(replacement);
    }

    QList<QVariant> data = QList<QVariant>();
    data.append(rawSearch);
    data.append(regexModifiersFromSearchOptions(searchOptions));
    data.append(replacement);
		data.append(QString::number(static_cast<int>(searchMode)));
    QVariant count = currentEditor()->asyncSendMessageWithResult("C_FUN_REPLACE_ALL", QVariant::fromValue(data)).get();
    return count.toInt();
}

int frmSearchReplace::selectAll(QString string, SearchHelpers::SearchMode searchMode, SearchHelpers::SearchOptions searchOptions) {
    QString rawSearch = SearchString::format(string, searchMode, searchOptions);

    QList<QVariant> data = QList<QVariant>();
    data.append(rawSearch);
    data.append(regexModifiersFromSearchOptions(searchOptions));
    QVariant count = currentEditor()->asyncSendMessageWithResult("C_FUN_SEARCH_SELECT_ALL", QVariant::fromValue(data)).get();
    return count.toInt();
}

SearchHelpers::SearchMode frmSearchReplace::searchModeFromUI()
{
    if (ui->radSearchPlainText->isChecked())
        return SearchHelpers::SearchMode::PlainText;

    else if (ui->radSearchWithSpecialChars->isChecked())
        return SearchHelpers::SearchMode::SpecialChars;

    else if (ui->radSearchWithRegex->isChecked())
        return SearchHelpers::SearchMode::Regex;

    else
        return SearchHelpers::SearchMode::PlainText;
}

SearchHelpers::SearchOptions frmSearchReplace::searchOptionsFromUI()
{
    SearchHelpers::SearchOptions searchOptions;

    if (ui->chkMatchCase->isChecked())
        searchOptions.MatchCase = true;
    if (ui->chkMatchWholeWord->isChecked())
        searchOptions.MatchWholeWord = true;

    return searchOptions;
}

void frmSearchReplace::findFromUI(bool forward, bool searchFromStart)
{
    SearchHelpers::SearchOptions sOpts = searchOptionsFromUI();
    sOpts.SearchFromStart = searchFromStart;

    this->search(ui->cmbSearch->currentText(),
                 searchModeFromUI(),
                 forward,
                 sOpts);
}

void frmSearchReplace::replaceFromUI(bool forward, bool searchFromStart)
{
    SearchHelpers::SearchOptions sOpts = searchOptionsFromUI();
    sOpts.SearchFromStart = searchFromStart;

    this->replace(ui->cmbSearch->currentText(),
                  ui->cmbReplace->currentText(),
                  searchModeFromUI(),
                  forward,
                  sOpts);
}

void frmSearchReplace::on_btnFindNext_clicked()
{
    findFromUI(true);
    addToSearchHistory(ui->cmbSearch->currentText());
}

void frmSearchReplace::on_btnFindPrev_clicked()
{
    findFromUI(false);
    addToSearchHistory(ui->cmbSearch->currentText());
}

void frmSearchReplace::on_btnReplaceNext_clicked()
{
    replaceFromUI(true);
    addToSearchHistory(ui->cmbSearch->currentText());
    addToReplaceHistory(ui->cmbReplace->currentText());
}

void frmSearchReplace::on_btnReplacePrev_clicked()
{
    replaceFromUI(false);
    addToSearchHistory(ui->cmbSearch->currentText());
    addToReplaceHistory(ui->cmbReplace->currentText());
}

void frmSearchReplace::on_btnReplaceAll_clicked()
{
    int n = this->replaceAll(ui->cmbSearch->currentText(),
                             ui->cmbReplace->currentText(),
                             searchModeFromUI(),
                             searchOptionsFromUI());

    addToSearchHistory(ui->cmbSearch->currentText());
    addToReplaceHistory(ui->cmbReplace->currentText());

    QMessageBox::information(this, tr("Replace all"), tr("%1 occurrences have been replaced.").arg(n));
}

void frmSearchReplace::on_btnSelectAll_clicked()
{
    int count = this->selectAll(ui->cmbSearch->currentText(),
                                searchModeFromUI(),
                                searchOptionsFromUI());

    addToSearchHistory(ui->cmbSearch->currentText());

    if (count == 0) {
        QMessageBox::information(this, tr("Select all"), tr("No results found"));
    } else {
        // Focus on main window
        this->m_topEditorContainer->activateWindow();
    }
}

void frmSearchReplace::on_actionReplace_toggled(bool on)
{
    ui->btnReplaceAll->setVisible(on);
    ui->btnReplaceNext->setVisible(on);
    ui->btnReplacePrev->setVisible(on);
    ui->cmbReplace->setVisible(on);
    ui->lblReplace->setVisible(on);

    ui->cmbSearch->setFocus();

    manualSizeAdjust();
}

void frmSearchReplace::on_actionFind_toggled(bool /*on*/)
{
    ui->cmbSearch->setFocus();

    manualSizeAdjust();
}

void frmSearchReplace::manualSizeAdjust()
{
    int curX = geometry().x();
    int curY = geometry().y();

    QApplication::processEvents();
    QApplication::processEvents();
    setGeometry(curX, curY, width(), 0);

    setFixedSize(width(), height());
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

void frmSearchReplace::on_searchStringEdited(const QString &/*text*/)
{
    NqqSettings& s = NqqSettings::getInstance();

    if (s.Search.getSearchAsIType()) {
        if (ui->actionFind->isChecked()) {
            auto editor = currentEditor();

            QList<Editor::Selection> selections = editor->selections();
            if (selections.length() > 0) {
                editor->setCursorPosition(
                            std::min(selections[0].from, selections[0].to));
            }

            findFromUI(true);
        }
    }

    // Workaround. See comment in setSearchText().
    ui->cmbSearch->setAutoCompletion(true);
    ui->cmbSearch->completer()->setCaseSensitivity(Qt::CaseSensitive);
}

/**
 * @brief Helper function to modify a list of strings that serve as a history.
 *        The provided string will be prepended to the history, and the history
 *        will be given to the comboBox as the list of suggestions to display.
 *        This only serves as a helper to reduce code duplication.
 * @param history The current history list passed as a reference. This list will
 *        have the string from the second parameter prepended to it after the function
 *        finishes.
 * @param string The string to add to the history.
 * @param comboBox The QComboBox to receive the history as suggestions list.
 */
void addToHistory(QStringList& history, QString string, QComboBox *comboBox) {
    if (string.isEmpty())
        return;

    history.prepend(string);
    history.removeDuplicates();
    history = history.mid(0, 10);
    comboBox->clear();
    comboBox->addItems(history);
}

// Returns a QStringList of all items in the combo box.
static QStringList getComboBoxContents(const QComboBox* cb) {
    QStringList list;
    const int size = cb->count();
    for (int index = 0; index < size; index++) {
        list << cb->itemText(index);
    }
    return list;
}

void frmSearchReplace::addToSearchHistory(QString string)
{
    NqqSettings& s = NqqSettings::getInstance();

    auto history = s.Search.getSaveHistory() ?
                s.Search.getSearchHistory() :
                getComboBoxContents(ui->cmbSearch);

    addToHistory(history, string, ui->cmbSearch);

    if (s.Search.getSaveHistory()) {
        s.Search.setSearchHistory(history);
    }
}

void frmSearchReplace::addToReplaceHistory(QString string)
{
    NqqSettings& s = NqqSettings::getInstance();

    auto history = s.Search.getSaveHistory() ?
                s.Search.getReplaceHistory() :
                getComboBoxContents(ui->cmbReplace);

    addToHistory(history, string, ui->cmbReplace);

    if (s.Search.getSaveHistory()) {
        s.Search.setReplaceHistory(history);
    }
}
