#ifndef SEARCHRESULTSITEMDELEGATE_H
#define SEARCHRESULTSITEMDELEGATE_H

#include "include/Search/filesearchresult.h"
#include <QSize>
#include <QPainter>
#include <QStyleOptionViewItemV4>
#include <QStyledItemDelegate>
#include <QStandardItem>

class SearchResultsItemDelegate : public QStyledItemDelegate
{
public:
    SearchResultsItemDelegate(QObject *parent = 0);
    ~SearchResultsItemDelegate();

    static const int RESULT_TYPE_ROLE = Qt::UserRole + 1;
    static const int RESULT_DATA_ROLE = Qt::UserRole + 2;
    static const int RESULT_DATA_EX_ROLE = Qt::UserRole + 3;

    enum ResultType {
        ResultTypeError,
        ResultTypeFile,
        ResultTypeMatch
    };
   
   /**
    * @brief Get the current `index` row item type.
    * @return `SearchResultsItemDelegate::ResultType`: The result type of `index`.
    */
    static SearchResultsItemDelegate::ResultType rowItemType(const QModelIndex &index);

   /**
    * @brief Populate result items.
    * @param `item`:             The item to populate.
    * @param `result`:           The result to populate `item` with.
    * @param `matches`:          Total number of matches within a file.
    * @param `totalFileMatches`: Total number of matches from a search.
    * @param `totalFiles`:       Total number of files matched within.
    */
    static void fillResultRowItem(QStandardItem *item, const FileSearchResult::Result &result);
    static void fillFileResultRowItem(QStandardItem *item, const FileSearchResult::FileResult &fileResult, const int matches);
    static void fillSearchResultRowItem(QStandardItem *item, const FileSearchResult::SearchResult &searchResult, const int totalFileMatches, const int totalFiles);

   /**
    * @brief Retrieve result data.
    * @param `index`: The index to use for retrieving result data.
    * @return `FileSearchResult::Result/FileResult`: The data at `index`.
    */
    static QPoint resultRowStartData(const QModelIndex &index);
    static QPoint resultRowEndData(const QModelIndex &index);
    static QString fileResultRowData(const QModelIndex &index);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    static QString getFileResultFormattedLine(const FileSearchResult::Result &result);
};

#endif // SEARCHRESULTSITEMDELEGATE_H
