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
    void addSearchResult(const FileSearchResult::SearchResult &searchResult);

private slots:
    void on_doubleClicked(const QModelIndex &index);

private:
    QStandardItemModel*   m_filesFindResultsModel;
    SearchResultsItemDelegate* m_treeViewHTMLDelegate;
    QAction* m_actionClear;
    void setupActions();

signals:
    void resultMatchClicked(const FileSearchResult::FileResult &file, const FileSearchResult::Result &match);

protected:
    void contextMenuEvent(QContextMenuEvent *e);
};

#endif // FILESEARCHRESULTSWIDGET_H
