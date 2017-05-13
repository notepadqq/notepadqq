#ifndef ADVANCEDSEARCHDOCK_H
#define ADVANCEDSEARCHDOCK_H

#include <QObject>
#include <QDockWidget>
#include <QAbstractButton>
#include <QString>

#include <memory>
#include <vector>

#include "searchinstance.h"

class QWidget;
class QLayout;
class QVBoxLayout;
class QToolButton;
class QComboBox;
class QCheckBox;

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
    QSize minimumSizeHint() const override { return sizeHint(); }

    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
};

class MainWindow;

class AdvancedSearchDock : public QObject
{
    Q_OBJECT
public:
    AdvancedSearchDock(MainWindow* mainWindow);

    QDockWidget* getDockWidget() const;

    void runSearch(SearchConfig cfg);

    void selectPrevResult();
    void selectNextResult();

signals:
    void resultItemClicked(const DocResult& doc, const MatchResult& result);

private:
    MainWindow* m_mainWindow;

    // Functions used to construct parts of the dock's user interface. Called in the constructor.
    QLayout* buildLeftTitlebar();
    QLayout* buildUpperTitlebarLayout();
    QLayout* buildReplaceOptionsLayout();
    QWidget* buildTitlebarWidget();
    QWidget* buildSearchPanelWidget();

    void clearHistory();
    void selectSearchFromHistory(int index);
    void updateSearchInProgressUi();
    void startReplace();
    void showReplaceDialog(const SearchResult& filteredResults, const QString& replaceText) const;

    void onChangeSearchScope(int index);
    void onCurrentSearchInstanceCompleted();
    void onUserInput();
    /**
     * @brief onSearchHistorySizeChange Called when the search history changes; used to disable some UI elements
     *                                  when the history is empty.
     */
    void onSearchHistorySizeChange();

    /**
     * @brief Adds the given item to one of the history lists stored in NqqSettings, also updates the
     *        corresponding QComboBox with the new history list.
     */
    void updateSearchHistory(const QString& item);
    void updateReplaceHistory(const QString& item);
    void updateDirectoryhHistory(const QString& item);
    void updateFilterHistory(const QString& item);

    /**
     * @brief getConfigFromInputs Reads out the UI (checkboxes, etc) and creates a SearchConfig object based
     *                            on these settings.
     */
    SearchConfig getConfigFromInputs();

    /**
     * @brief setInputsFromConfig Sets the UI (checkboxes, etc) to the settings specified in the given SearchConfig.
     */
    void setInputsFromConfig(const SearchConfig& config);

    QScopedPointer<QDockWidget> m_dockWidget; // TODO: Use Qt's parent system

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
    QComboBox*   m_cmbSearchTerm;
    QComboBox*   m_cmbSearchPattern;
    QComboBox*   m_cmbSearchDirectory;
    QToolButton* m_btnSelectSearchDirectory;
    QToolButton* m_btnSearch;
    QCheckBox*   m_chkMatchCase;
    QCheckBox*   m_chkMatchWords;
    QCheckBox*   m_chkUseRegex;
    QCheckBox*   m_chkUseSpecialChars;
    QCheckBox*   m_chkIncludeSubdirs;

    // Replace panel items
    QComboBox*   m_cmbReplaceText;
    QToolButton* m_btnReplaceSelected;
    QCheckBox*   m_chkReplaceWithSpecialChars;

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
