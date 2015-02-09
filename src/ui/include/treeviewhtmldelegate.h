#ifndef TREEVIEWHTMLDELEGATE_H
#define TREEVIEWHTMLDELEGATE_H
#include <QSize>
#include <QPainter>
#include <QStyleOptionViewItemV4>
#include <QStyledItemDelegate>

class TreeViewHTMLDelegate : public QStyledItemDelegate
{
public:
    TreeViewHTMLDelegate(QObject *parent = 0);
    ~TreeViewHTMLDelegate();
protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // TREEVIEWHTMLDELEGATE_H
