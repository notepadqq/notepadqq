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
    QStandardItem *searchRow = new QStandardItem();

    // Total number of matches in all the files
    int totalFileMatches = 0;
    // Number of files that contain matches
    int totalFiles = 0;

    for (FileSearchResult::FileResult fileResult : searchResult.fileResults)
    {
        totalFiles++;

        // Row, in the model, relative to this file
        QStandardItem *fileRow = new QStandardItem();

        int curFileMatches = 0;

        for (FileSearchResult::Result result : fileResult.results)
        {
            QStandardItem *lineRow = new QStandardItem();
            SearchResultsItemDelegate::fillResultRowItem(lineRow, result);
            fileRow->appendRow(lineRow);

            curFileMatches++;
            totalFileMatches++;
        }

        SearchResultsItemDelegate::fillFileResultRowItem(fileRow, fileResult, curFileMatches);
        searchRow->appendRow(fileRow);
    }


    SearchResultsItemDelegate::fillSearchResultRowItem(searchRow, searchResult, totalFileMatches, totalFiles);

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
    QPoint start = SearchResultsItemDelegate::resultRowStartData(index);
    QPoint end   = SearchResultsItemDelegate::resultRowEndData(index);
    if (type == SearchResultsItemDelegate::ResultTypeMatch) {
        emit resultMatchClicked(
                    SearchResultsItemDelegate::fileResultRowData(index.parent()),
                    start.x(),
                    start.y(),
                    end.x(),
                    end.y());
    }
}

void FileSearchResultsWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);
    menu.addAction(m_actionClear);
    menu.exec(e->globalPos());
}
