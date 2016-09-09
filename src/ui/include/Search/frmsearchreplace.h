#ifndef FRMSEARCHREPLACE_H
#define FRMSEARCHREPLACE_H

#include "include/topeditorcontainer.h"
#include "include/Search/filesearchresult.h"
#include "include/Search/searchinfilesworker.h"
#include "include/Search/replaceinfilesworker.h"
#include "include/Search/searchhelpers.h"
#include "include/Search/dlgsearching.h"
#include <QDialog>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QComboBox>

namespace Ui {
class frmSearchReplace;
}

class frmSearchReplace : public QMainWindow
{
    Q_OBJECT

public:
    enum Tabs { TabSearch, TabReplace, TabSearchInFiles };
    explicit frmSearchReplace(TopEditorContainer *topEditorContainer, QWidget *parent = 0);
    ~frmSearchReplace();

   /**
    * @brief Sets the currently displayed tab.
    * @param `defaultTab`: The tab to change our display to.
    */
    void show(Tabs defaultTab);
   /**
    * @brief Sets the search input text to value specified in `string`.
    * @param `string`: Value to change the search input text to.
    */
    void setSearchText(QString string);

   /**
    * @brief Runs a "find next" or "find prev", taking the options from the UI.
    * @param `forward`: Direction in which to iterate.
    */
    void findFromUI(bool forward, bool searchFromStart = false);
   /**
    * @brief Runs a "replace next" or "replace prev", taking the options from the UI.
    * @param `forward`: Direction in which to iterate.
    */
    void replaceFromUI(bool forward, bool searchFromStart = false);
protected:
    void keyPressEvent(QKeyEvent *evt);

signals:
    void fileSearchResultFinished(FileSearchResult::SearchResult result);
    void stopSearchInFiles();
    void stopReplaceInFiles();

public slots:
   /**
    * @brief Handle file error request from thread.
    * @param `message`:   The message received from the working thread.
    * @param `operation`: The referenced value from the thread awaiting reply.
    */
    void displayThreadErrorMessageBox(const QString &message, int &operation);
   /**
    * @brief Display results from ReplaceInFilesWorker thread.
    * @param `replaceCount`:  Number of occurrences replaced.
    * @param `fileCount`:     Number of files changed.   
    * @param `stopped`:       Bool value which determines how we display results.
    */
    void handleReplaceResult(int replaceCount, int fileCount, bool stopped);
   /**
    * @brief Display results from SearchInFilesWorker thread.
    * @param `result`: FileSearchResult::SearchResult struct to generate the display from.
    */
    void handleSearchResult(const FileSearchResult::SearchResult &result);
   /**
    * @brief Handle general error message from thread.
    * @param `e`: The error message received.
    */
    void handleError(const QString &e);
   /**
    * @brief Handle progress report from thread.
    * @param `file`: Current file being worked on.
    * @param `replace`: Bool value which determines how we display progress.
    */
    void handleProgress(const QString &file, bool replace = false);
   /**
    * @brief Starts ReplaceInFilesWorker thread if we started the search in replaceMode.
    * @param `result`: FileSearchResult::SearchResult struct to work on.
    */
    void handleReplaceInFiles(const FileSearchResult::SearchResult &result);

private slots:
    void on_btnFindNext_clicked();
    void on_btnFindPrev_clicked();
    void on_btnReplaceNext_clicked();
    void on_btnReplacePrev_clicked();
    void on_btnReplaceAll_clicked();
    void on_btnSelectAll_clicked();
    void on_actionFind_toggled(bool on);
    void on_actionReplace_toggled(bool on);
    void on_actionFind_in_files_toggled(bool on);
    void on_chkShowAdvanced_toggled(bool checked);
    void on_radSearchWithRegex_toggled(bool checked);
    void on_radSearchPlainText_toggled(bool checked);
    void on_radSearchWithSpecialChars_toggled(bool checked);
    void on_searchStringEdited(const QString &text);
    void on_btnFindAll_clicked();
    void on_btnLookInBrowse_clicked();
    void on_btnReplaceAllInFiles_clicked();

private:
    Ui::frmSearchReplace*  ui;
    TopEditorContainer*    m_topEditorContainer;
    QString                m_lastSearch;

    class SearchInFilesSession : public QObject {
    public:
        SearchInFilesSession(QObject *parent) : QObject(parent) { }
        SearchInFilesWorker*   threadSearch = nullptr;
        ReplaceInFilesWorker*  threadReplace = nullptr;
        dlgSearching*          msgBox = nullptr;
    };

    SearchInFilesSession* m_session = nullptr;
    QList<SearchInFilesSession*> m_findInFilesPtrs;

   /**
    * @brief Get the current editor.
    */
    Editor*                currentEditor();
   /**
    * @brief Clean up Search in file sessions.
    */
    void sessionCleanup();
   /**
    * @brief Ask for confirmation for replacing in files.
    * @param `path`:    The directory path being worked on.
    * @param `filters`: The filters that will be applied.
    * @return `bool`:   The result of the request.
    */
    bool confirmReplaceInFiles(const QString &path, const QStringList &filters);
   /**
    * @brief Perform a search within the current document.
    * @param `string`:        The string to search for.
    * @param `searchMode`:    Search mode to use.
    * @param `forward`:       Direction in which to search.
    * @param `searchOptions`: Search options to use.
    */
    void search(QString string, SearchHelpers::SearchMode searchMode, bool forward, SearchHelpers::SearchOptions searchOptions);
   /**
    * @brief Perform a replace within the current document.
    * @param `string`:        The string to search for.
    * @param `replacement`:   The string which will replace `string`.
    * @param `searchMode`:    Search mode to use.
    * @param `forward`:       Direction in which to search.
    * @param `searchOptions`: Search options to use.
    */
    void replace(QString string, QString replacement, SearchHelpers::SearchMode searchMode, bool forward, SearchHelpers::SearchOptions searchOptions);
   /**
    * @brief Perform a full document replace within the current document.
    * @param `string`:        The string to search for.
    * @param `replacement`:   The string which will replace `string`.
    * @param `searchMode`:    Search mode to use.
    * @param `searchOptions`: Search options to use.
    */
    int replaceAll(QString string, QString replacement, SearchHelpers::SearchMode searchMode, SearchHelpers::SearchOptions searchOptions);
   /**
    * @brief Select all instances of `string` within the current document.
    * @param `string`:        The string to search for.
    * @param `searchMode`:    Search mode to use.
    * @param `forward`:       Direction in which to search.
    * @param `searchOptions`: Search options to use.
    */
    int selectAll(QString string, SearchHelpers::SearchMode searchMode, SearchHelpers::SearchOptions searchOptions);
   /**
    * @brief Perform a search or replacement of `string` within the selected `path`.
    * @param `string`:        The string to be searched for.
    * @param `path`:          The directory path to work in.
    * @param `filters`:       File filters to limit the scope of the search/replacement.
    * @param `searchMode`:    Search mode to use.
    * @param `searchOptions`: Search options to use.
    * @param `replaceMode`:   Replace found occurrences if true.
    */
    void searchReplaceInFiles(const QString &string, const QString &path, const QStringList &filters, const SearchHelpers::SearchMode &searchMode, const SearchHelpers::SearchOptions &searchOptions, bool replaceMode = false);
   /**
    * @brief Sets the current tab.
    * @param `tab`: The tab to be set to.
    */
    void setCurrentTab(Tabs tab);
   /**
    * @brief Adjust the size of the frmSearchReplace window.
    */
    void manualSizeAdjust();
   /**
    * @brief Retrieve search options from UI.
    * @return `SearchHelpers::SearchOptions`: The UI search options.
    */
    SearchHelpers::SearchOptions searchOptionsFromUI();
   /**
    * @brief Retrieve search mode from UI.
    * @return `SearchHelpers::SearchMode`: The UI search mode.
    */
    SearchHelpers::SearchMode searchModeFromUI();
   /**
    * @brief Apply regex modifiers based on UI options.
    * @param `searchOptions`: The search options to use.
    * @return `QString`: Modified string based on `searchOptions`.
    */
    QString regexModifiersFromSearchOptions(SearchHelpers::SearchOptions searchOptions);
   /**
    * @brief Retrieve file filters from UI.
    * @return `QStringList`: List of current UI file filters.
    */
    QStringList fileFiltersFromUI();

    /**
     * @brief Displays the abort/retry/ignore message box for read and write errors
     *        in SearchInFilesWorker and ReplaceInFilesWorker.
     *        Assigns the return value to "operation".
     * @param message
     * @param operation
     */
    void addToSearchHistory(QString string);
    void addToReplaceHistory(QString string);
    void addToFileHistory(QString string);
    void addToFilterHistory(QString string);
};

#endif // FRMSEARCHREPLACE_H
