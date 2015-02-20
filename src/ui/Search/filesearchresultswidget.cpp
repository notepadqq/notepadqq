#include "include/Search/filesearchresultswidget.h"
#include <QMenu>
#include <QContextMenuEvent>

FileSearchResultsWidget::FileSearchResultsWidget(QWidget *parent) :
    QTreeView(parent),
    m_filesFindResultsModel(new QStandardItemModel(this)),
    m_treeViewHTMLDelegate(new SearchResultsItemDelegate(this))
{
    setModel(m_filesFindResultsModel);
    setItemDelegate(m_treeViewHTMLDelegate);

    setEditTriggers(NoEditTriggers);
    setHeaderHidden(true);

    setupActions();

    connect(this, &FileSearchResultsWidget::doubleClicked,
            this, &FileSearchResultsWidget::on_doubleClicked);
}

FileSearchResultsWidget::~FileSearchResultsWidget()
{

}

void FileSearchResultsWidget::setupActions()
{
    m_actionClear = new QAction(tr("Clear"), this);
    connect(m_actionClear, &QAction::triggered, this, [&]{
        m_filesFindResultsModel->clear();
    });
}

void FileSearchResultsWidget::addSearchResult(const FileSearchResult::SearchResult &searchResult)
{
    // Row, in the model, relative to this search
    QList<QStandardItem *> searchRow;
    searchRow << new QStandardItem();

    // Total number of matches in all the files
    int totalFileMatches = 0;
    // Number of files that contain matches
    int totalFiles = 0;

    for (FileSearchResult::FileResult fileResult : searchResult.fileResults)
    {
        totalFiles++;

        // Row, in the model, relative to this file
        QList<QStandardItem *> fileRow;
        fileRow << new QStandardItem();

        int curFileMatches = 0;

        for (FileSearchResult::Result result : fileResult.results)
        {
            QList<QStandardItem *> lineRow;
            lineRow << new QStandardItem();
            SearchResultsItemDelegate::fillResultRowItem(lineRow[0], result);
            fileRow[0]->appendRow(lineRow);

            curFileMatches++;
            totalFileMatches++;
        }

        SearchResultsItemDelegate::fillFileResultRowItem(fileRow[0], fileResult, curFileMatches);
        searchRow[0]->appendRow(fileRow);
    }


    SearchResultsItemDelegate::fillSearchResultRowItem(searchRow[0], searchResult, totalFileMatches, totalFiles);

    QStandardItem *root = m_filesFindResultsModel->invisibleRootItem();
    root->insertRow(0, searchRow);


    // Force an update of the treeView, otherwise the new row isn't available (Qt bug?)
    setModel(NULL);
    setModel(m_filesFindResultsModel);

    // Expand the first row
    expand(m_filesFindResultsModel->item(0)->index());
}

void FileSearchResultsWidget::on_doubleClicked(const QModelIndex &index)
{
    int type = SearchResultsItemDelegate::rowItemType(index);

    if (type == SearchResultsItemDelegate::ResultTypeMatch) {
        emit resultMatchClicked(
                    SearchResultsItemDelegate::fileResultRowData(index.parent()),
                    SearchResultsItemDelegate::resultRowData(index));
    }
}

void FileSearchResultsWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);
    menu.addAction(m_actionClear);
    menu.exec(e->globalPos());
}
