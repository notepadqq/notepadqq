#ifndef SEARCHINSTANCE_H
#define SEARCHINSTANCE_H

#include <QObject>
#include <QString>
#include <QTreeWidget>
#include <QScopedPointer>

#include <memory>
#include <map>

#include "searchobjects.h"
#include "filesearcher.h"

/**
 * @brief The SearchInstance class contains all the data that represents a search, including the
 *        tree widget for displaying. It's used in conjunction with AdvancedSearchDock to display
 *        its search results. On construction, the SearchInstance object will also initiate the
 *        search.
 */
class SearchInstance : public QObject {
    Q_OBJECT

public:
    /**
     * @brief SearchInstance Constructs SearchInstance object and starts a search.
     * @param config If config.searchScope is ScopeFileSystem, a non-blocking file search will be started.
     *               If it's ScopeCurrentDocument or ScopeAllDocuments, a blocking document search will
     *               be started, but searching documents is fast enough not to visibly block the UI.
     */
    SearchInstance(const SearchConfig& config);
    ~SearchInstance();

    /**
     * @brief getShowFullLines Returns true if the user has checked the "Show Full Lines" option
     */
    bool getShowFullLines() const { return m_showFullLines; }

    /**
     * @brief areResultsExpanded Returns true if the user has checked the "Expand All" option
     */
    bool areResultsExpanded() const { return m_resultsAreExpanded; }

    /**
     * @brief isSearchInProgress Returns true if a file search is currently in progress.
     */
    bool isSearchInProgress() const { return m_isSearchInProgress; }

    QTreeWidget*        getResultTreeWidget() const { return m_treeWidget.data(); }
    const SearchConfig& getSearchConfig() const { return m_searchConfig; }
    const SearchResult& getSearchResult() const { return m_searchResult; }

    /**
     * @brief getFilteredSearchResult Returns a SearchResult object with only those MatchResults whose
     *                                respective item in the TreeWidget is checked.
     */
    SearchResult getFilteredSearchResult() const;

    // Actions
    void expandAllResults();
    void collapseAllResults();

    void selectNextResult();
    void selectPreviousResult();

    void showFullLines(bool showFullLines);
    void copySelectedLinesToClipboard() const;

signals:
    void searchCompleted();

    /**
     * @brief resultItemClicked Emitted when an item in the tree widget is double-clicked
     */
    void resultItemClicked(const DocResult& doc, const MatchResult& result);

private:
    void onSearchProgress(int processed, int total);
    void onSearchCompleted();

    bool m_isSearchInProgress = true; // Search is started in the constructor so it can default to true
    bool m_resultsAreExpanded = false;
    bool m_showFullLines = false;

    SearchConfig                m_searchConfig;
    QScopedPointer<QTreeWidget> m_treeWidget;
    SearchResult                m_searchResult;
    FileSearcher*               m_fileSearcher = nullptr;

    // These map each QTreeWidget item to their respective MatchResult or DocResult
    std::map<QTreeWidgetItem*, const MatchResult*>  m_resultMap;
    std::map<QTreeWidgetItem*, const DocResult*>    m_docMap;
};


#endif // SEARCHINSTANCE_H
