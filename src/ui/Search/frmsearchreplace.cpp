#include "include/Search/frmsearchreplace.h"
#include "include/iconprovider.h"
#include "ui_frmsearchreplace.h"
#include <QLineEdit>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
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

    QSettings s;
    ui->cmbSearch->addItems(s.value("Search/searchHistory", QStringList()).toStringList());
    ui->cmbSearch->setCurrentText("");
    ui->cmbReplace->addItems(s.value("Search/replaceHistory", QStringList()).toStringList());
    ui->cmbReplace->setCurrentText("");
    ui->cmbLookIn->addItems(s.value("Search/fileHistory", QStringList()).toStringList());
    ui->cmbLookIn->setCurrentText("");
    ui->cmbFilter->addItems(s.value("Search/filterHistory", QStringList()).toStringList());
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

QString frmSearchReplace::plainTextToRegex(QString text, bool matchWholeWord)
{
    // Transform it into a regex, but make sure to escape special chars
    QString regex = QRegularExpression::escape(text);

    if (matchWholeWord)
        regex = "\\b" + regex + "\\b";

    return regex;
}

QString frmSearchReplace::rawSearchString(QString search, SearchHelpers::SearchMode searchMode, SearchHelpers::SearchOptions searchOptions)
{
    QString rawSearch;

    if (searchMode == SearchHelpers::SearchMode::Regex) {
        rawSearch = search;
    } else if (searchMode == SearchHelpers::SearchMode::SpecialChars) {
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

QString frmSearchReplace::regexModifiersFromSearchOptions(SearchHelpers::SearchOptions searchOptions)
{
    QString modifiers = "m";
    if (!searchOptions.MatchCase)
        modifiers.append("i");

    return modifiers;
}

void frmSearchReplace::search(QString string, SearchHelpers::SearchMode searchMode, bool forward, SearchHelpers::SearchOptions searchOptions) {
    if (!string.isEmpty()) {
        QString rawSearch = rawSearchString(string, searchMode, searchOptions);

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
        QString rawSearch = rawSearchString(string, searchMode, searchOptions);

        Editor *editor = currentEditor();

        if (searchOptions.SearchFromStart) {
            editor->setCursorPosition(0, 0);
        }

        QList<QVariant> data = QList<QVariant>();
        data.append(rawSearch);
        data.append(regexModifiersFromSearchOptions(searchOptions));
        data.append(forward);
        data.append(replacement);
        editor->sendMessage("C_FUN_REPLACE", QVariant::fromValue(data));
    }
}

int frmSearchReplace::replaceAll(QString string, QString replacement, SearchHelpers::SearchMode searchMode, SearchHelpers::SearchOptions searchOptions) {
    QString rawSearch = rawSearchString(string, searchMode, searchOptions);

    QList<QVariant> data = QList<QVariant>();
    data.append(rawSearch);
    data.append(regexModifiersFromSearchOptions(searchOptions));
    data.append(replacement);
    QVariant count = currentEditor()->sendMessageWithResult("C_FUN_REPLACE_ALL", QVariant::fromValue(data));
    return count.toInt();
}

int frmSearchReplace::selectAll(QString string, SearchHelpers::SearchMode searchMode, SearchHelpers::SearchOptions searchOptions) {
    QString rawSearch = rawSearchString(string, searchMode, searchOptions);

    QList<QVariant> data = QList<QVariant>();
    data.append(rawSearch);
    data.append(regexModifiersFromSearchOptions(searchOptions));
    QVariant count = currentEditor()->sendMessageWithResult("C_FUN_SEARCH_SELECT_ALL", QVariant::fromValue(data));
    return count.toInt();
}

void frmSearchReplace::searchInFiles(const QString &string, const QString &path, const QStringList &filters, const SearchHelpers::SearchMode &searchMode, const SearchHelpers::SearchOptions &searchOptions)
{
    cleanFindInFilesPtrs();

    if (!string.isEmpty()) {
        SearchInFilesSession *session = new SearchInFilesSession(this);
        m_findInFilesPtrs.append(session);

        session->msgBox = new dlgSearching(this);
        session->msgBox->setTitle(tr("Searching..."));
        session->msgBox->setWindowTitle(session->msgBox->title());

        session->threadSearch = new QThread();
        session->workerSearch = new SearchInFilesWorker(string, path, filters, searchMode, searchOptions);
        session->workerSearch->moveToThread(session->threadSearch);

        connect(session->threadSearch, &QThread::started, session->workerSearch, &SearchInFilesWorker::run);

        connect(session->workerSearch, &SearchInFilesWorker::error, this, [=](QString err){
            if (session->msgBox != nullptr) {
                session->msgBox->setTitle(tr("Error"));
                session->msgBox->setText(err);
            }
        });

        connect(session->workerSearch, &SearchInFilesWorker::errorReadingFile, this, &frmSearchReplace::displayThreadErrorMessageBox, Qt::BlockingQueuedConnection);

        connect(session->workerSearch, &SearchInFilesWorker::progress, this, [=](QString file){
            if (session->msgBox != nullptr)
                session->msgBox->setText(QString("%1").arg(file));
        });

        connect(session->threadSearch, &QThread::finished, this, [=]{
            session->threadSearch->deleteLater();
            session->threadSearch = nullptr;
        });

        connect(session->workerSearch, &SearchInFilesWorker::finished, this, [=](bool stopped){
            FileSearchResult::SearchResult result;

            if (session->threadSearch != nullptr)
                session->threadSearch->quit();

            session->msgBox->hide();
            session->msgBox->deleteLater();
            session->msgBox = nullptr;

            if (stopped) {
                session->workerSearch->deleteLater();
                session->workerSearch = nullptr;

                return;
            } else {
                result = session->workerSearch->getResult();

                session->workerSearch->deleteLater();
                session->workerSearch = nullptr;
            }

            emit fileSearchResultFinished(result);
        });

        session->threadSearch->start();
        session->msgBox->exec();

        // If we're here, the search finished or the user wants to cancel it.

        if (session->workerSearch != nullptr)
            session->workerSearch->stop();
    }
}

void frmSearchReplace::replaceInFiles(const QString &string, const QString &replacement, const QString &path, const QStringList &filters, const SearchHelpers::SearchMode &searchMode, const SearchHelpers::SearchOptions &searchOptions)
{
    cleanFindInFilesPtrs();

    if (QMessageBox::warning(
                this,
                tr("Replace in files"),
                tr("Are you sure you want to replace all occurrences in %1 for file types %2?")
                   .arg(path)
                   .arg(filters.isEmpty() ? "*" : filters.join(", ")),
                QMessageBox::Ok | QMessageBox::Cancel,
                QMessageBox::Cancel)
          != QMessageBox::Ok) {

        return;
    }

    if (!string.isEmpty()) {
        SearchInFilesSession *session = new SearchInFilesSession(this);
        m_findInFilesPtrs.append(session);

        session->msgBox = new dlgSearching(this);
        session->msgBox->setTitle(tr("Searching..."));
        session->msgBox->setWindowTitle(session->msgBox->title());

        session->threadSearch = new QThread();
        session->workerSearch = new SearchInFilesWorker(string, path, filters, searchMode, searchOptions);
        session->workerSearch->moveToThread(session->threadSearch);


        connect(session->threadSearch, &QThread::started, session->workerSearch, &SearchInFilesWorker::run);

        connect(session->workerSearch, &SearchInFilesWorker::error, this, [=](QString err){
            if (session->msgBox != nullptr) {
                session->msgBox->setTitle(tr("Error"));
                session->msgBox->setText(err);
            }
        });

        connect(session->workerSearch, &SearchInFilesWorker::errorReadingFile, this, &frmSearchReplace::displayThreadErrorMessageBox, Qt::BlockingQueuedConnection);

        connect(session->workerSearch, &SearchInFilesWorker::progress, this, [=](QString file){
            if (session->msgBox != nullptr)
                session->msgBox->setText(tr("Searching in %1").arg(file));
        });

        connect(session->threadSearch, &QThread::finished, this, [=]{
            session->threadSearch->deleteLater();
            session->threadSearch = nullptr;
        });

        connect(session->workerSearch, &SearchInFilesWorker::finished, this, [=](bool stopped){
            FileSearchResult::SearchResult result;

            if (session->threadSearch != nullptr)
                session->threadSearch->quit();

            if (stopped) {
                session->workerSearch->deleteLater();
                session->workerSearch = nullptr;
                session->msgBox->deleteLater();
                session->msgBox = nullptr;

                return;
            } else {
                result = session->workerSearch->getResult();

                session->workerSearch->deleteLater();
                session->workerSearch = nullptr;
            }

            // Start to replace
            session->msgBox->setTitle(tr("Replacing..."));

            session->threadReplace = new QThread();
            session->workerReplace = new ReplaceInFilesWorker(result, replacement);
            session->workerReplace->moveToThread(session->threadReplace);

            connect(session->threadReplace, &QThread::started, session->workerReplace, &ReplaceInFilesWorker::run);

            connect(session->workerReplace, &ReplaceInFilesWorker::error, this, [=](QString err){
                if (session->msgBox != nullptr) {
                    session->msgBox->setTitle(tr("Error"));
                    session->msgBox->setText(err);
                }
            });

            connect(session->workerReplace, &ReplaceInFilesWorker::errorReadingFile, this, &frmSearchReplace::displayThreadErrorMessageBox, Qt::BlockingQueuedConnection);

            connect(session->workerReplace, &ReplaceInFilesWorker::errorWritingFile, this, &frmSearchReplace::displayThreadErrorMessageBox, Qt::BlockingQueuedConnection);

            connect(session->workerReplace, &ReplaceInFilesWorker::progress, this, [=](QString file){
                if (session->msgBox != nullptr)
                    session->msgBox->setText(tr("Replacing in %1").arg(file));
            });

            connect(session->threadReplace, &QThread::finished, this, [=]{
                session->threadReplace->deleteLater();
                session->threadReplace = nullptr;
            });

            connect(session->workerReplace, &ReplaceInFilesWorker::finished, this, [=](bool stopped){
                session->msgBox->hide();

                if (session->threadReplace != nullptr)
                    session->threadReplace->quit();

                QPair<int, int> result = session->workerReplace->getResult();

                session->workerReplace->deleteLater();
                session->workerReplace = nullptr;
                session->msgBox->deleteLater();
                session->msgBox = nullptr;

                QApplication::processEvents();

                if (!stopped) {
                    QMessageBox::information(this,
                                             tr("Replace in files"),
                                             tr("%1 occurrences replaced in %2 files.").arg(result.first).arg(result.second));
                } else {
                    QMessageBox::information(this,
                                             tr("Replace in files"),
                                             tr("%1 occurrences replaced in %2 files, but the replacement has been canceled before it could finish.").arg(result.first).arg(result.second));
                }
            });

            session->threadReplace->start();
        });

        session->threadSearch->start();
        session->msgBox->exec();

        // If we're here, the search finished or the user wants to cancel it.

        // If the search wasn't finished yet, cancel it
        if (session->workerSearch != nullptr)
            session->workerSearch->stop();
        if (session->workerReplace != nullptr)
            session->workerReplace->stop();
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

void frmSearchReplace::cleanFindInFilesPtrs()
{
    for (int i = m_findInFilesPtrs.count() - 1; i >= 0; i--) {
        SearchInFilesSession *session = m_findInFilesPtrs[i];
        if (session->msgBox == nullptr
                && session->threadSearch == nullptr
                && session->workerSearch == nullptr
                && session->threadReplace == nullptr
                && session->workerReplace == nullptr) {

            m_findInFilesPtrs.removeAt(i);
            session->deleteLater();
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
    QSettings s;

    if (s.value("Search/SearchAsIType", true).toBool()) {
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
    searchInFiles(ui->cmbSearch->currentText(),
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
    replaceInFiles(ui->cmbSearch->currentText(),
                   ui->cmbReplace->currentText(),
                   ui->cmbLookIn->currentText(),
                   fileFiltersFromUI(),
                   searchModeFromUI(),
                   searchOptionsFromUI());

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

void frmSearchReplace::addToHistory(QString string, QString type, QComboBox *comboBox)
{
    if (string != "") {
        QSettings s;
        QStringList history = s.value("Search/" + type + "History", QStringList()).toStringList();
        history.prepend(string);
        history.removeDuplicates();
        history = history.mid(0, 10);
        s.setValue("Search/" + type + "History", history);
        comboBox->clear();
        comboBox->addItems(history);
    }
}

void frmSearchReplace::addToSearchHistory(QString string)
{
    addToHistory(string, "search", ui->cmbSearch);
}

void frmSearchReplace::addToReplaceHistory(QString string)
{
    addToHistory(string, "replace", ui->cmbReplace);
}

void frmSearchReplace::addToFileHistory(QString string)
{
    addToHistory(string, "file", ui->cmbLookIn);
}

void frmSearchReplace::addToFilterHistory(QString string)
{
    addToHistory(string, "filter", ui->cmbFilter);
}
