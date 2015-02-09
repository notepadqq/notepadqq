#ifndef FILESEARCHRESULTSWIDGET_H
#define FILESEARCHRESULTSWIDGET_H

#include <QTreeView>
#include <QStandardItemModel>
#include "include/filesearchresult.h"
#include "include/treeviewhtmldelegate.h"

class FileSearchResultsWidget : public QTreeView
{
public:
    FileSearchResultsWidget();
    ~FileSearchResultsWidget();
    void addSearchResult(FileSearchResult::SearchResult result);

private slots:
    void on_filesFindResultsModelRowsInserted(const QModelIndex & parent, int first, int last);

private:
    QStandardItemModel*  m_filesFindResultsModel;
    TreeViewHTMLDelegate* m_treeViewHTMLDelegate;
    QString getFileResultFormattedLine(const FileSearchResult::Result &result);
};

#endif // FILESEARCHRESULTSWIDGET_H
