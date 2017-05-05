#ifndef ADVANCEDSEARCHDOCK_H
#define ADVANCEDSEARCHDOCK_H

#include <QObject>
#include <QScopedPointer>
#include <QDockWidget>
#include <QAbstractButton>
#include <QPoint>
#include <QString>
#include <QTreeWidget>

#include <memory>
#include <vector>
#include <map>

#include "searchworker.h"

class QWidget;
class QLayout;
class QVBoxLayout;
class QToolButton;
class QComboBox;
class QLineEdit;
class QCheckBox;

class SearchInstance : public QObject {
    Q_OBJECT

public:
    SearchConfig m_searchConfig;
    std::unique_ptr<QTreeWidget> m_treeWidget;
    SearchResult m_searchResult;
    FileSearcher* m_fileSearcher;

    SearchInstance(const SearchConfig& config);
    ~SearchInstance();

    bool isMaximized = false;
    bool searchInProgress = true;

    bool getShowFullLines() const { return m_showFullLines; }
    void setShowFullLines(bool m_showFullLines);

    std::map<QTreeWidgetItem*, const MatchResult*> resultMap;
    std::map<QTreeWidgetItem*, const DocResult*> docMap;

signals:
    void searchCompleted();
    void resultItemClicked(const DocResult& doc, const MatchResult& result);

public slots:
    void onSearchProgress(int processed, int total);
    void onSearchCompleted();

private:
    bool m_showFullLines = false;
};


/**
 * @brief The QDockWidgetTitleButton class is used to display a normal
 *        close/minimize/maximize tool button. This is needed for the search
 *        dock because customizing a dock's titlebar will remove these buttons.
 *        This class is copied almost 1-to-1 from Qt's source code and will
 *        display a button exactly like the dock's default titlebar.
 */
class QDockWidgetTitleButton : public QAbstractButton
{
    Q_OBJECT

public:
    QDockWidgetTitleButton(QDockWidget *dockWidget);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override
    { return sizeHint(); }

    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
};


class AdvancedSearchDock : public QObject
{
    Q_OBJECT
public:
    AdvancedSearchDock();

    QDockWidget* getDockWidget() const;

    void runSearch(SearchConfig cfg);

    void selectPrevResult();
    void selectNextResult();

signals:
    void resultItemClicked(const DocResult& doc, const MatchResult& result);

private:
    QLayout* buildLeftTitlebar();
    QLayout* buildUpperTitlebarLayout();
    QLayout* buildReplaceOptionsLayout();
    QWidget* buildTitlebarWidget();
    QWidget* buildSearchPanelWidget();

    void clearHistory();
    void selectSearchFromHistory(int index);
    void onChangeSearchScope(int index);
    void updateSearchInProgressUi();

    void onCurrentSearchInstanceCompleted();
    void onUserInput();

    SearchConfig getConfigFromInputs();
    void setInputsFromConfig(const SearchConfig& config);

    void onSearchHistorySizeChange();

    QScopedPointer<QDockWidget> m_dockWidget;

    // Left-hand titlebar items
    QToolButton* m_btnClearHistory;
    QComboBox*   m_cmbSearchHistory;
    QToolButton* m_btnMoreOptions;
    QToolButton* m_btnPrevResult;
    QToolButton* m_btnNextResult;
    QToolButton* m_btnToggleReplaceOptions;

    // Right-hand titlebar items
    QAbstractButton* m_btnClose;
    QAbstractButton* m_btnDockUndock;

    // Search panel items
    QComboBox*   m_cmbSearchScope;
    QLineEdit*   m_edtSearchTerm; // TODO: Change to combo boxes and remember history
    QLineEdit*   m_edtSearchPattern;
    QLineEdit*   m_edtSearchDirectory;
    QToolButton* m_btnSelectSearchDirectory;
    QToolButton* m_btnSearch;
    QCheckBox*   m_chkMatchCase;
    QCheckBox*   m_chkMatchWords;
    QCheckBox*   m_chkUseRegex;
    QCheckBox*   m_chkIncludeSubdirs;

    // Replace panel items
    QLineEdit* m_edtReplaceText;
    QToolButton* m_btnReplaceOne;
    QToolButton* m_btnReplaceSelected;

    // "More Options" menu items
    QAction* m_actExpandAll;
    QAction* m_actRedoSearch;
    QAction* m_actCopyContents;
    QAction* m_actShowFullLines;
    QAction* m_actRemoveSearch;

    QVBoxLayout* m_titlebarLayout;
    QVBoxLayout* m_replaceOptionsLayout;

    QWidget* m_searchPanelWidget;

    // For handling multiple search instances
    std::vector<std::unique_ptr<SearchInstance>> m_searchInstances;
    SearchInstance* m_currentSearchInstance = nullptr;
};

#endif // ADVANCEDSEARCHDOCK_H
