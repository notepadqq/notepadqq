#ifndef SEARCHRESULTSITEMDELEGATE_H
#define SEARCHRESULTSITEMDELEGATE_H
#include <QSize>
#include <QPainter>
#include <QStyleOptionViewItemV4>
#include <QStyledItemDelegate>
#include <QStandardItem>
#include "include/Search/filesearchresult.h"

class SearchResultsItemDelegate : public QStyledItemDelegate
{
public:
    SearchResultsItemDelegate(QObject *parent = 0);
    ~SearchResultsItemDelegate();

    static const int RESULT_TYPE_ROLE = Qt::UserRole + 1;
    static const int RESULT_DATA_ROLE = Qt::UserRole + 2;

    enum ResultType {
        ResultTypeError,
        ResultTypeFile,
        ResultTypeMatch
    };

    static SearchResultsItemDelegate::ResultType rowItemType(const QModelIndex &index);

    static void fillResultRowItem(QStandardItem *item, const FileSearchResult::Result &result);
    static void fillFileResultRowItem(QStandardItem *item, const FileSearchResult::FileResult &fileResult, const int matches);
    static void fillSearchResultRowItem(QStandardItem *item, const FileSearchResult::SearchResult &searchResult, const int totalFileMatches, const int totalFiles);

    static FileSearchResult::Result resultRowData(const QModelIndex &index);
    static FileSearchResult::FileResult fileResultRowData(const QModelIndex &index);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    static QString getFileResultFormattedLine(const FileSearchResult::Result &result);
};

#endif // SEARCHRESULTSITEMDELEGATE_H
