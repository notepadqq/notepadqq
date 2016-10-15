#ifndef FILESEARCHRESULTSWIDGET_H
#define FILESEARCHRESULTSWIDGET_H

#include <QTreeView>
#include <QStandardItemModel>
#include <QAction>
#include "include/Search/filesearchresult.h"
#include "include/Search/searchresultsitemdelegate.h"

class FileSearchResultsWidget : public QTreeView
{
    Q_OBJECT

public:
    FileSearchResultsWidget(QWidget *parent = 0);
    ~FileSearchResultsWidget();
   
   /**
    * @brief Add search results to the results widget.
    * @param `searchResult`:  The search results to be added.
    */
    void addSearchResult(const FileSearchResult::SearchResult &searchResult);

private slots:
    void on_doubleClicked(const QModelIndex &index);

private:
    QStandardItemModel*   m_filesFindResultsModel;
    SearchResultsItemDelegate* m_treeViewHTMLDelegate;
    QAction* m_actionClear;

   /**
    * @brief Set up UI actions.
    */
    void setupActions();

signals:
   /**
    * @brief Handle match result being clicked.
    * @param `file`:  The file result struct to use.
    * @param `match`: The match result struct to use.
    */
    void resultMatchClicked(const QString &fileName, int startLine, int startCol, int endLine, int endCol);

protected:
    void contextMenuEvent(QContextMenuEvent *e);
};

#endif // FILESEARCHRESULTSWIDGET_H
