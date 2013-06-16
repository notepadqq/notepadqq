#ifndef QTABWIDGETSCONTAINER_H
#define QTABWIDGETSCONTAINER_H

#include <QSplitter>
#include "qtabwidgetqq.h"

class QTabWidgetsContainer : public QSplitter
{
    Q_OBJECT
public:
    explicit QTabWidgetsContainer(QWidget *parent = 0);
    
    QTabWidgetqq *focusQTabWidgetqq(bool fallback = true);
    QTabWidgetqq *QTabWidgetqqAt(int index);
    QTabWidgetqq *addQTabWidgetqq();
signals:
    void newQsciScintillaqqChildCreated(QsciScintillaqq *sci);
    
public slots:
    void _on_newQsciScintillaqqWidget(QsciScintillaqq *sci); // Called when a child scintilla widget is inserted
    
};

#endif // QTABWIDGETSCONTAINER_H
