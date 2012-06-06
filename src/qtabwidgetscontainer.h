#ifndef QTABWIDGETSCONTAINER_H
#define QTABWIDGETSCONTAINER_H

#include <QSplitter>
#include "qtabwidgetqq.h"

class QTabWidgetsContainer : public QSplitter
{
    Q_OBJECT
public:
    explicit QTabWidgetsContainer(QWidget *parent = 0);
    
    QTabWidgetqq *focusQTabWidgetqq();
    QTabWidgetqq *QTabWidgetqqAt(int index);
signals:
    
public slots:
    
};

#endif // QTABWIDGETSCONTAINER_H
