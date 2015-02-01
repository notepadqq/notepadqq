#ifndef FRMSEARCHREPLACE_H
#define FRMSEARCHREPLACE_H

#include "topeditorcontainer.h"
#include <QDialog>
#include <QMainWindow>
#include <QStandardItemModel>

namespace Ui {
class frmSearchReplace;
}

class frmSearchReplace : public QMainWindow
{
    Q_OBJECT

public:
    enum Tabs { TabSearch, TabReplace, TabSearchInFiles };
    explicit frmSearchReplace(TopEditorContainer *topEditorContainer, QStandardItemModel *filesFindResultsModel, QWidget *parent = 0);
    ~frmSearchReplace();
    void show(Tabs defaultTab);

    /**
     * @brief Runs a "find next" or "find prev", taking the options from the UI
     * @param forward
     */
    void findFromUI(bool forward, bool searchFromStart = false);
    void replaceFromUI(bool forward, bool searchFromStart = false);

protected:
    void keyPressEvent(QKeyEvent *evt);

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

private:
    Ui::frmSearchReplace*  ui;
    TopEditorContainer*    m_topEditorContainer;
    QString                m_lastSearch;
    QStandardItemModel*    m_filesFindResultsModel;
    Editor*                currentEditor();

    enum class SearchMode {
        PlainText,
        SpecialChars,
        Regex
    };

    struct SearchOptions {
        unsigned MatchCase : 1;
        unsigned MatchWholeWord : 1;
        unsigned SearchFromStart : 1;

        SearchOptions() : MatchCase(0), MatchWholeWord(0),
        SearchFromStart(0) { }
    };

    void search(QString string, SearchMode searchMode, bool forward, SearchOptions searchOptions);
    QString plainTextToRegex(QString text, bool matchWholeWord);
    void replace(QString string, QString replacement, SearchMode searchMode, bool forward, SearchOptions searchOptions);
    int replaceAll(QString string, QString replacement, SearchMode searchMode, SearchOptions searchOptions);
    int selectAll(QString string, SearchMode searchMode, SearchOptions searchOptions);
    void searchInFiles(QString string, QString path, QStringList filter, SearchMode searchMode, SearchOptions searchOptions);
    void setCurrentTab(Tabs tab);
    void manualSizeAdjust();
    SearchOptions searchOptionsFromUI();
    SearchMode searchModeFromUI();
    QString rawSearchString(QString search, SearchMode searchMode, SearchOptions searchOptions);
    QString regexModifiersFromSearchOptions(SearchOptions searchOptions);
};

#endif // FRMSEARCHREPLACE_H
