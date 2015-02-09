#include "include/filesearchresultswidget.h"

FileSearchResultsWidget::FileSearchResultsWidget(QWidget *parent) :
    QTreeView(parent),
    m_filesFindResultsModel(new QStandardItemModel(this)),
    m_treeViewHTMLDelegate(new TreeViewHTMLDelegate(this))
{
    setModel(m_filesFindResultsModel);
    setItemDelegate(m_treeViewHTMLDelegate);

    setEditTriggers(NoEditTriggers);
    setHeaderHidden(true);
}

FileSearchResultsWidget::~FileSearchResultsWidget()
{

}

void FileSearchResultsWidget::addSearchResult(FileSearchResult::SearchResult result)
{
    // Row, in the model, relative to this search
    QList<QStandardItem *> searchRow;
    searchRow << new QStandardItem();

    // Total number of matches in all the files
    int totalFileMatches = 0;
    // Number of files that contain matches
    int totalFiles = 0;

    for (FileSearchResult::FileResult fileResult : result.fileResults)
    {
        totalFiles++;

        // Row, in the model, relative to this file
        QList<QStandardItem *> fileRow;
        fileRow << new QStandardItem();

        int curFileMatches = 0;

        for (FileSearchResult::Result result : fileResult.results)
        {
            QList<QStandardItem *> lineRow;
            lineRow << new QStandardItem(getFileResultFormattedLine(result));
            fileRow[0]->appendRow(lineRow);

            curFileMatches++;
            totalFileMatches++;
        }

        fileRow[0]->setText(QString("%1 (%2 hits)")
                            .arg(fileResult.fileName.toHtmlEscaped())
                            .arg(curFileMatches));
        searchRow[0]->appendRow(fileRow);
    }

    searchRow[0]->setText(QString("Search \"%1\" (%2 hits in %3 files)")
                          .arg(result.search.toHtmlEscaped())
                          .arg(totalFileMatches).arg(totalFiles));
    QStandardItem *root = m_filesFindResultsModel->invisibleRootItem();
    root->insertRow(0, searchRow);



    // Force an update of the treeView, otherwise the new row isn't available (Qt bug?)
    setModel(NULL);
    setModel(m_filesFindResultsModel);

    // Expand the first row
    expand(m_filesFindResultsModel->item(0)->index());
}

QString FileSearchResultsWidget::getFileResultFormattedLine(const FileSearchResult::Result & result)
{
    QString richTextLine = result.line.mid(0, result.lineMatchStart).toHtmlEscaped()
            + "<span style=\"background-color: #ffef0b\">"
            + result.line.mid(result.lineMatchStart, result.lineMatchEnd - result.lineMatchStart).toHtmlEscaped()
            + "</span>" + result.line.mid(result.lineMatchEnd).toHtmlEscaped();

    return QString("Line %1: %2").arg(result.lineNumber + 1).arg(richTextLine);
}
