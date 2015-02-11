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

    connect(this, &FileSearchResultsWidget::doubleClicked,
            this, &FileSearchResultsWidget::on_doubleClicked);
}

FileSearchResultsWidget::~FileSearchResultsWidget()
{

}

void FileSearchResultsWidget::addSearchResult(const FileSearchResult::SearchResult &result)
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
            lineRow[0]->setData(ResultTypeMatch, RESULT_TYPE_ROLE);
            lineRow[0]->setData(result, RESULT_DATA_ROLE);
            fileRow[0]->appendRow(lineRow);

            curFileMatches++;
            totalFileMatches++;
        }

        fileRow[0]->setText(QString("%1 (%2 hits)")
                            .arg(fileResult.fileName.toHtmlEscaped())
                            .arg(curFileMatches));
        fileRow[0]->setData(ResultTypeFile, RESULT_TYPE_ROLE);
        fileRow[0]->setData(fileResult, RESULT_DATA_ROLE);
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

QString FileSearchResultsWidget::getFileResultFormattedLine(const FileSearchResult::Result &result) const
{
    QString richTextLine = result.previewLine.mid(0, result.lineMatchStart).toHtmlEscaped()
            + "<span style=\"background-color: #ffef0b\">"
            + result.previewLine.mid(result.lineMatchStart, result.lineMatchEnd - result.lineMatchStart).toHtmlEscaped()
            + "</span>" + result.previewLine.mid(result.lineMatchEnd).toHtmlEscaped();

    return QString("Line %1: %2").arg(result.lineNumber + 1).arg(richTextLine);
}

void FileSearchResultsWidget::on_doubleClicked(const QModelIndex &index)
{
    QVariant type_q = index.data(RESULT_TYPE_ROLE);

    if (!type_q.canConvert<int>()) return;
    int type = type_q.toInt();

    QVariant data = index.data(RESULT_DATA_ROLE);

    switch (type)
    {
    case ResultTypeFile:
        emit resultFileClicked(data.value<FileSearchResult::FileResult>());
        break;

    case ResultTypeMatch:
        emit resultMatchClicked(
                    index.parent().data(RESULT_DATA_ROLE).value<FileSearchResult::FileResult>(),
                    data.value<FileSearchResult::Result>());
        break;
    }
}
