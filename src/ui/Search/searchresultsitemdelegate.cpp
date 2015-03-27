#include "include/Search/searchresultsitemdelegate.h"
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QBrush>

SearchResultsItemDelegate::SearchResultsItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

SearchResultsItemDelegate::~SearchResultsItemDelegate()
{

}

QString SearchResultsItemDelegate::getFileResultFormattedLine(const FileSearchResult::Result &result)
{
    QString richTextLine = result.previewBeforeMatch.toHtmlEscaped()
            + "<span style=\"background-color: #ffef0b; color: black;\">"
            + result.match.toHtmlEscaped()
            + "</span>" + result.previewAfterMatch.toHtmlEscaped();

    return QString("Line %1: %2").arg(result.matchStartLine + 1).arg(richTextLine);
}

void SearchResultsItemDelegate::fillResultRowItem(QStandardItem *item, const FileSearchResult::Result &result)
{
    item->setText(getFileResultFormattedLine(result));
    item->setData(SearchResultsItemDelegate::ResultTypeMatch, SearchResultsItemDelegate::RESULT_TYPE_ROLE);
    item->setData(result, SearchResultsItemDelegate::RESULT_DATA_ROLE);
}

void SearchResultsItemDelegate::fillFileResultRowItem(QStandardItem *item, const FileSearchResult::FileResult &fileResult, const int matches)
{
    item->setText(QString("%1 (%2 hits)")
                        .arg(fileResult.fileName.toHtmlEscaped())
                        .arg(matches));
    item->setData(SearchResultsItemDelegate::ResultTypeFile, SearchResultsItemDelegate::RESULT_TYPE_ROLE);
    item->setData(fileResult, SearchResultsItemDelegate::RESULT_DATA_ROLE);
}

void SearchResultsItemDelegate::fillSearchResultRowItem(QStandardItem *item, const FileSearchResult::SearchResult &searchResult, const int totalFileMatches, const int totalFiles)
{
    item->setText(QString("Search \"%1\" (%2 hits in %3 files)")
                      .arg(searchResult.search.toHtmlEscaped())
                      .arg(totalFileMatches).arg(totalFiles));
}

SearchResultsItemDelegate::ResultType SearchResultsItemDelegate::rowItemType(const QModelIndex &index)
{
    QVariant type_q = index.data(SearchResultsItemDelegate::RESULT_TYPE_ROLE);

    if (!type_q.canConvert<int>()) return SearchResultsItemDelegate::ResultTypeError;
    int type = type_q.toInt();

    return static_cast<ResultType>(type);
}

FileSearchResult::Result SearchResultsItemDelegate::resultRowData(const QModelIndex &index)
{
    QVariant data = index.data(SearchResultsItemDelegate::RESULT_DATA_ROLE);
    return data.value<FileSearchResult::Result>();
}

FileSearchResult::FileResult SearchResultsItemDelegate::fileResultRowData(const QModelIndex &index)
{
    QVariant data = index.data(SearchResultsItemDelegate::RESULT_DATA_ROLE);
    return data.value<FileSearchResult::FileResult>();
}

void SearchResultsItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    ResultType rType = rowItemType(index);

    if (rType == ResultTypeMatch)
    {
        QStyleOptionViewItemV4 options = option;
        initStyleOption(&options, index);

        painter->save();

        QTextDocument doc;
        doc.setHtml(options.text);

        options.text = "";
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);

        // shift text right to make icon visible
        QSize iconSize = options.icon.actualSize(options.rect.size());
        painter->translate(options.rect.left()+iconSize.width(), options.rect.top());
        QRect clip(0, 0, options.rect.width()+iconSize.width(), options.rect.height());

        //doc.drawContents(painter, clip);

        painter->setClipRect(clip);
        QAbstractTextDocumentLayout::PaintContext ctx;

        // set text color for selected item
        if (option.state & QStyle::State_Selected)
            ctx.palette.setColor(QPalette::Text, options.palette.highlightedText().color());

        ctx.clip = clip;
        doc.documentLayout()->draw(painter, ctx);

        painter->restore();

    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize SearchResultsItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);

    QTextDocument doc;
    doc.setHtml(options.text);
    doc.setTextWidth(options.rect.width());
    return QSize(doc.idealWidth(), doc.size().height());
}
