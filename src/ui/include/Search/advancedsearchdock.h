#ifndef ADVANCEDSEARCHDOCK_H
#define ADVANCEDSEARCHDOCK_H

#include "searchinstance.h"

#include <QAbstractButton>
#include <QDockWidget>
#include <QObject>
#include <QString>

#include <memory>
#include <vector>

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
class QSearchDockTitleButton : public QAbstractButton
{
    Q_OBJECT

public:
    QSearchDockTitleButton(QDockWidget *dockWidget);

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

    /**
     * @brief startSearch Starts a search with the settings given in the SearchConfig.
     */
    void startSearch(SearchConfig cfg);

    void selectPrevResult();
    void selectNextResult();

    bool isVisible() const;
    void show(bool show, bool setFocus=true);

signals:
    /**
     * @brief itemInteracted Emitted when an item in the current tree widget is interacted with.
     * @param doc The selected DocResult
     * @param result The selected MatchResult. If this is nullptr then the user only selected a DocResult
     * @param type The kind of interaction requested by the user
     */
    void itemInteracted(const DocResult& doc, const MatchResult* result, SearchUserInteraction type);

private:
    MainWindow* m_mainWindow;

    // Functions used to construct parts of the dock's user interface. Called in the constructor.
    QLayout* buildLeftTitlebar();
    QLayout* buildUpperTitlebarLayout();
    QLayout* buildReplaceOptionsLayout();
    QWidget* buildTitlebarWidget();
    QWidget* buildSearchPanelWidget();

    /**
     * @brief clearHistory Removes all items from the search history and resets UI back to "New Search"
     */
    void clearHistory();

    /**
     * @brief selectSearchFromHistory Selects a given SearchInstance and displays it.
     * @param index If 0, the "New Search" panel will be shown, else the respective SearchInstance will be shown.
     */
    void selectSearchFromHistory(int index);
    void updateSearchInProgressUi();
    /**
     * @brief startReplace Takes the current SearchInstance and replaces all of its selected matches with the replacement
     *                     string found in m_cmbReplaceText
     */
    void startReplace();

    /**
     * @brief showReplaceDialog Shows a blocking dialog asking the user whether they want to continue with replacing
     * @param filteredResults Contains all ResultMatches that will be replaced
     * @param replaceText The text that is used for replacing
     */
    void showReplaceDialog(const SearchResult& filteredResults, const QString& replaceText) const;

    void onChangeSearchScope(int index);
    void onCurrentSearchInstanceCompleted();
    void onUserInput();
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
    QComboBox*   m_cmbSearchTerm;
    QComboBox*   m_cmbSearchPattern;
    QComboBox*   m_cmbSearchDirectory;
    QToolButton* m_btnSelectSearchDirectory;
    QToolButton* m_btnSelectCurrentDirectory;
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
