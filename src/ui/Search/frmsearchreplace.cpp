#include "include/Search/frmsearchreplace.h"
#include "include/Search/searchstring.h"
#include "include/iconprovider.h"
#include "include/nqqsettings.h"
#include "ui_frmsearchreplace.h"
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QThread>
#include <QCompleter>

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
    ui->cmbLookIn->addItems(s.Search.getFileHistory());
    ui->cmbLookIn->setCurrentText("");
    ui->cmbFilter->addItems(s.Search.getFilterHistory());
    ui->cmbFilter->setCurrentText("");

    connect(ui->cmbSearch->lineEdit(), &QLineEdit::textEdited, this, &frmSearchReplace::on_searchStringEdited);
    connect(ui->cmbSearch->lineEdit(), &QLineEdit::returnPressed, this, [=]() {
        if (ui->actionFind_in_files->isChecked()) {
            on_btnFindAll_clicked();
        } else {
            on_btnFindNext_clicked();
        }
    });
    connect(ui->cmbReplace->lineEdit(), &QLineEdit::returnPressed, this, &frmSearchReplace::on_btnFindNext_clicked);
    connect(ui->cmbLookIn->lineEdit(), &QLineEdit::returnPressed, this, &frmSearchReplace::on_btnFindAll_clicked);
    connect(ui->cmbFilter->lineEdit(), &QLineEdit::returnPressed, this, &frmSearchReplace::on_btnFindAll_clicked);

    ui->cmbFilter->lineEdit()->setPlaceholderText("*.ext1, *.ext2, ...");

    ui->actionFind->setIcon(IconProvider::fromTheme("edit-find"));
    ui->actionReplace->setIcon(IconProvider::fromTheme("edit-find-replace"));

    QActionGroup *tabGroup = new QActionGroup(this);
    tabGroup->addAction(ui->actionFind);
    tabGroup->addAction(ui->actionReplace);
    tabGroup->addAction(ui->actionFind_in_files);
    tabGroup->setExclusive(true);

    // Initialize all the tabs
    ui->actionFind->setChecked(true);
    ui->actionReplace->setChecked(true);
    ui->actionFind_in_files->setChecked(true);

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
}

void frmSearchReplace::setCurrentTab(Tabs tab)
{
    if (tab == TabSearch) {
        ui->actionFind->setChecked(true);
    } else if (tab == TabReplace) {
        ui->actionReplace->setChecked(true);
    } else if (tab == TabSearchInFiles) {
        ui->actionFind_in_files->setChecked(true);
    }
}

Editor *frmSearchReplace::currentEditor()
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
        QString rawSearch = SearchString::toRaw(string, searchMode, searchOptions);

        Editor *editor = currentEditor();

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
        QString rawSearch = SearchString::toRaw(string, searchMode, searchOptions);
        if (searchMode == SearchHelpers::SearchMode::SpecialChars) {
            replacement = SearchString::unescape(replacement);
        }

        Editor *editor = currentEditor();

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
    QString rawSearch = SearchString::toRaw(string, searchMode, searchOptions);
    if (searchMode == SearchHelpers::SearchMode::SpecialChars) {
            replacement = SearchString::unescape(replacement);
    }

    QList<QVariant> data = QList<QVariant>();
    data.append(rawSearch);
    data.append(regexModifiersFromSearchOptions(searchOptions));
    data.append(replacement);
		data.append(QString::number(static_cast<int>(searchMode)));
    QVariant count = currentEditor()->sendMessageWithResult("C_FUN_REPLACE_ALL", QVariant::fromValue(data));
    return count.toInt();
}

int frmSearchReplace::selectAll(QString string, SearchHelpers::SearchMode searchMode, SearchHelpers::SearchOptions searchOptions) {
    QString rawSearch = SearchString::toRaw(string, searchMode, searchOptions);

    QList<QVariant> data = QList<QVariant>();
    data.append(rawSearch);
    data.append(regexModifiersFromSearchOptions(searchOptions));
    QVariant count = currentEditor()->sendMessageWithResult("C_FUN_SEARCH_SELECT_ALL", QVariant::fromValue(data));
    return count.toInt();
}

void frmSearchReplace::sessionCleanup()
{
    if (m_session != nullptr) {
        m_session->threadSearch = nullptr;
        m_session->threadReplace = nullptr;
        if (m_session->msgBox != nullptr) {
            m_session->msgBox->hide();
            m_session->msgBox->deleteLater();
            m_session->msgBox = nullptr;
        }
        m_session->deleteLater();
        m_session = nullptr;
    }
}

void frmSearchReplace::displayThreadErrorMessageBox(const QString &message, int &operation)
{
    operation = QMessageBox::warning(
                this,
                tr("Error"),
                message,
                QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore,
                QMessageBox::Retry);
}

void frmSearchReplace::handleSearchResult(const FileSearchResult::SearchResult &result)
{
    sessionCleanup();
    emit fileSearchResultFinished(result);
}

void frmSearchReplace::handleReplaceResult(int replaceCount, int fileCount, bool stopped)
{
    sessionCleanup();
    QApplication::processEvents();
    if (!stopped) {
        QMessageBox::information(this,
                                 tr("Replace in files"),
                                 tr("%1 occurrences replaced in %2 files.").arg(replaceCount).arg(fileCount));
    } else {
        QMessageBox::information(this,
                                 tr("Replace in files"),
                                 tr("%1 occurrences replaced in %2 files, but the replacement has been canceled before it could finish.").arg(replaceCount).arg(fileCount));
    }
}

void frmSearchReplace::handleError(const QString &e)
{
    if (m_session->msgBox != nullptr) {
        m_session->msgBox->setTitle(tr("Error"));
        m_session->msgBox->setText(e);
    }
}

void frmSearchReplace::handleProgress(const QString &file, bool replace)
{
    if (m_session->msgBox != nullptr) {
        if (replace) {
            m_session->msgBox->setText(tr("Replacing in ").append("%1").arg(file));
        } else {
            m_session->msgBox->setText(tr("Searching in ").append("%1").arg(file));
        }
    }
}

void frmSearchReplace::handleReplaceInFiles(const FileSearchResult::SearchResult &result)
{
    QString replacement = ui->cmbReplace->currentText();
    m_session->msgBox->hide();
    m_session->msgBox->deleteLater();
    m_session->msgBox = new dlgSearching(this);
    m_session->msgBox->setTitle(tr("Replacing..."));
    m_session->threadReplace = new ReplaceInFilesWorker(this, result, replacement);

    connect(m_session->threadReplace, SIGNAL(finished()), m_session->threadReplace, SLOT(deleteLater()));
    connect(m_session->threadReplace, &ReplaceInFilesWorker::error, this, &frmSearchReplace::handleError);
    connect(m_session->threadReplace, &ReplaceInFilesWorker::errorReadingFile, this, &frmSearchReplace::displayThreadErrorMessageBox, Qt::BlockingQueuedConnection);
    connect(m_session->threadReplace, &ReplaceInFilesWorker::progress, this, &frmSearchReplace::handleProgress);
    connect(m_session->threadReplace, &ReplaceInFilesWorker::resultReady, this, &frmSearchReplace::handleReplaceResult);
    connect(this, &frmSearchReplace::stopReplaceInFiles, m_session->threadReplace, &ReplaceInFilesWorker::stop, Qt::DirectConnection);
    m_session->threadReplace->start();
    if (m_session->msgBox->exec()) {
        emit stopReplaceInFiles();
    }
    sessionCleanup();
}

bool frmSearchReplace::confirmReplaceInFiles(const QString &path, const QStringList &filters)
{
    return (QMessageBox::warning(this,
                        tr("Replace in files"),
                        tr("Are you sure you want to replace all occurrences in %1 for file types %2?")
                           .arg(path)
                           .arg(filters.isEmpty() ? "*" : filters.join(", ")),
                        QMessageBox::Ok | QMessageBox::Cancel,
                        QMessageBox::Cancel)
                  == QMessageBox::Ok);
}

void frmSearchReplace::searchReplaceInFiles(const QString &string, const QString &path, const QStringList &filters, const SearchHelpers::SearchMode &searchMode, const SearchHelpers::SearchOptions &searchOptions, bool replaceMode)
{
    if (m_session == nullptr) {
        m_session = new SearchInFilesSession(this);
    }
    if (!string.isEmpty()) {
        if (replaceMode && !confirmReplaceInFiles(path, filters)) {
            sessionCleanup();
            return;
        }
        m_session->threadSearch = new SearchInFilesWorker(this, string, path, filters, searchMode, searchOptions);
        m_session->msgBox = new dlgSearching(this);
        m_session->msgBox->setTitle(tr("Searching..."));
        m_session->msgBox->setWindowTitle(m_session->msgBox->title());

        connect(m_session->threadSearch, SIGNAL(finished()), m_session->threadSearch, SLOT(deleteLater()));
        connect(m_session->threadSearch, &SearchInFilesWorker::error, this, &frmSearchReplace::handleError);
        connect(m_session->threadSearch, &SearchInFilesWorker::errorReadingFile, this, &frmSearchReplace::displayThreadErrorMessageBox, Qt::BlockingQueuedConnection);
        connect(m_session->threadSearch, &SearchInFilesWorker::progress, this, &frmSearchReplace::handleProgress); 
        connect(this, &frmSearchReplace::stopSearchInFiles, m_session->threadSearch, &SearchInFilesWorker::stop, Qt::DirectConnection);
        //Send results to a different location in the event of replaceMode.
        if (replaceMode) {
            connect(m_session->threadSearch, &SearchInFilesWorker::resultReady, this, &frmSearchReplace::handleReplaceInFiles);
        } else {
            connect(m_session->threadSearch, &SearchInFilesWorker::resultReady, this, &frmSearchReplace::handleSearchResult);
        }
        m_session->threadSearch->start();

        int cancel = m_session->msgBox->exec();
        if (cancel) {
            emit stopSearchInFiles();
        }
        if (!replaceMode || cancel) {
            sessionCleanup();
        }
    }
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
    if (ui->chkIncludeSubdirs->isChecked())
        searchOptions.IncludeSubDirs = true;

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

void frmSearchReplace::on_actionFind_in_files_toggled(bool on)
{
    ui->lblReplace->setVisible(on);
    ui->cmbReplace->setVisible(on);
    ui->lblLookIn->setVisible(on);
    ui->cmbLookIn->setVisible(on);
    ui->lblFilter->setVisible(on);
    ui->cmbFilter->setVisible(on);
    ui->btnLookInBrowse->setVisible(on);
    ui->btnFindAll->setVisible(on);
    ui->lblSpacer1->setVisible(on);
    ui->btnReplaceAllInFiles->setVisible(on);
    ui->chkIncludeSubdirs->setVisible(on);
    ui->btnFindNext->setVisible(!on);
    ui->btnFindPrev->setVisible(!on);
    ui->btnSelectAll->setVisible(!on);

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
            Editor *editor = currentEditor();

            QList<Editor::Selection> selections = editor->selections();
            if (selections.length() > 0) {
                editor->setCursorPosition(
                            std::min(selections[0].from, selections[0].to));
            }

            findFromUI(true);
        }
    }
}

void frmSearchReplace::on_btnFindAll_clicked()
{
    searchReplaceInFiles(ui->cmbSearch->currentText(),
                  ui->cmbLookIn->currentText(),
                  fileFiltersFromUI(),
                  searchModeFromUI(),
                  searchOptionsFromUI());

    addToSearchHistory(ui->cmbSearch->currentText());
    addToFileHistory(ui->cmbLookIn->currentText());
    addToFilterHistory(ui->cmbFilter->currentText());
}

void frmSearchReplace::on_btnLookInBrowse_clicked()
{
    QString defaultDir = ui->cmbLookIn->currentText();
    if (defaultDir.isEmpty()) {
        QFileInfo file(currentEditor()->fileName().toLocalFile());
        defaultDir = file.absolutePath();
    }

    QString dir = QFileDialog::getExistingDirectory(this, tr("Look in"),
                                                     defaultDir,
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) {
        ui->cmbLookIn->setCurrentText(dir);
    }
}

void frmSearchReplace::on_btnReplaceAllInFiles_clicked()
{
    searchReplaceInFiles(ui->cmbSearch->currentText(),
                   ui->cmbLookIn->currentText(),
                   fileFiltersFromUI(),
                   searchModeFromUI(),
                   searchOptionsFromUI(),
                   true);

    addToSearchHistory(ui->cmbSearch->currentText());
    addToReplaceHistory(ui->cmbReplace->currentText());
    addToFileHistory(ui->cmbLookIn->currentText());
    addToFilterHistory(ui->cmbFilter->currentText());
}

QStringList frmSearchReplace::fileFiltersFromUI()
{
    QStringList filters = ui->cmbFilter->currentText().split(",", QString::SkipEmptyParts);
    for (int i = 0; i < filters.count(); i++) {
        filters[i] = filters[i].trimmed();
    }
    return filters;
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

void frmSearchReplace::addToSearchHistory(QString string)
{
    NqqSettings& s = NqqSettings::getInstance();

    auto history = s.Search.getSearchHistory();
    addToHistory(history, string, ui->cmbSearch);
    s.Search.setSearchHistory(history);
}

void frmSearchReplace::addToReplaceHistory(QString string)
{
    NqqSettings& s = NqqSettings::getInstance();

    auto history = s.Search.getReplaceHistory();
    addToHistory(history, string, ui->cmbReplace);
    s.Search.setReplaceHistory(history);
}

void frmSearchReplace::addToFileHistory(QString string)
{
    NqqSettings& s = NqqSettings::getInstance();

    auto history = s.Search.getFileHistory();
    addToHistory(history, string, ui->cmbLookIn);
    s.Search.setFileHistory(history);
}

void frmSearchReplace::addToFilterHistory(QString string)
{
    NqqSettings& s = NqqSettings::getInstance();

    auto history = s.Search.getFilterHistory();
    addToHistory(history, string, ui->cmbFilter);
    s.Search.setFilterHistory(history);
}
