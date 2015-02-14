#ifndef FILESEARCHRESULTSWIDGET_H
#define FILESEARCHRESULTSWIDGET_H

#include <QTreeView>
#include <QStandardItemModel>
#include <QAction>
#include "include/Search/filesearchresult.h"
#include "include/Search/treeviewhtmldelegate.h"

class FileSearchResultsWidget : public QTreeView
{
    Q_OBJECT

public:
    FileSearchResultsWidget(QWidget *parent = 0);
    ~FileSearchResultsWidget();
    void addSearchResult(const FileSearchResult::SearchResult &result);

private slots:
    void on_doubleClicked(const QModelIndex &index);

private:
    static const int RESULT_TYPE_ROLE = Qt::UserRole + 1;
    static const int RESULT_DATA_ROLE = Qt::UserRole + 2;

    enum ResultType {
        ResultTypeFile,
        ResultTypeMatch
    };

    QStandardItemModel*   m_filesFindResultsModel;
    TreeViewHTMLDelegate* m_treeViewHTMLDelegate;
    QAction* m_actionClear;
    QString getFileResultFormattedLine(const FileSearchResult::Result &result) const;
    void setupActions();

signals:
    void resultFileClicked(const FileSearchResult::FileResult &file);
    void resultMatchClicked(const FileSearchResult::FileResult &file, const FileSearchResult::Result &match);

protected:
    void contextMenuEvent(QContextMenuEvent *e);
};

#endif // FILESEARCHRESULTSWIDGET_H
