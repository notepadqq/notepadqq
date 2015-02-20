#ifndef SEARCHRESULTSITEMDELEGATE_H
#define SEARCHRESULTSITEMDELEGATE_H
#include <QSize>
#include <QPainter>
#include <QStyleOptionViewItemV4>
#include <QStyledItemDelegate>

class SearchResultsItemDelegate : public QStyledItemDelegate
{
public:
    SearchResultsItemDelegate(QObject *parent = 0);
    ~SearchResultsItemDelegate();
protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // SEARCHRESULTSITEMDELEGATE_H
