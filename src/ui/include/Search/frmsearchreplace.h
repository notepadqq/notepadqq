#ifndef FRMSEARCHREPLACE_H
#define FRMSEARCHREPLACE_H

#include "include/Search/searchhelpers.h"
#include "include/topeditorcontainer.h"

#include <QComboBox>
#include <QDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QStandardItemModel>

namespace Ui {
class frmSearchReplace;
}

class frmSearchReplace : public QMainWindow
{
    Q_OBJECT

public:
    enum Tabs { TabSearch, TabReplace };
    explicit frmSearchReplace(TopEditorContainer *topEditorContainer, QWidget *parent = nullptr);
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
    void toggleAdvancedSearch();

private slots:
    void on_btnFindNext_clicked();
    void on_btnFindPrev_clicked();
    void on_btnReplaceNext_clicked();
    void on_btnReplacePrev_clicked();
    void on_btnReplaceAll_clicked();
    void on_btnSelectAll_clicked();
    void on_actionFind_toggled(bool on);
    void on_actionReplace_toggled(bool on);
    void on_chkShowAdvanced_toggled(bool checked);
    void on_radSearchWithRegex_toggled(bool checked);
    void on_radSearchPlainText_toggled(bool checked);
    void on_radSearchWithSpecialChars_toggled(bool checked);
    void on_searchStringEdited(const QString &text);

private:
    Ui::frmSearchReplace*  ui;
    TopEditorContainer*    m_topEditorContainer;
    QString                m_lastSearch;

   /**
    * @brief Get the current editor.
    */
    QSharedPointer<Editor> currentEditor();

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
     * @brief Displays the abort/retry/ignore message box for read and write errors
     *        in SearchInFilesWorker and ReplaceInFilesWorker.
     *        Assigns the return value to "operation".
     * @param message
     * @param operation
     */
    void addToSearchHistory(QString string);
    void addToReplaceHistory(QString string);
};

#endif // FRMSEARCHREPLACE_H
