#include "include/keygrabber.h"
#include <QHeaderView>
#include <QPainter>

keyGrabber::keyGrabber(QWidget* parent) : QTableWidget(parent)
{
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setDefaultSectionSize(200);
    verticalHeader()->setVisible(false);
    verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setDefaultSectionSize(20);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setAlternatingRowColors(true);
    setColumnCount(2);
    setHorizontalHeaderItem(0,new QTableWidgetItem("Action"));
    setHorizontalHeaderItem(1,new QTableWidgetItem("Keyboard Shortcut"));
    connect(this,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(itemChanged(QTableWidgetItem*)));
}

void keyGrabber::itemChanged(QTableWidgetItem* item)
{
    //Don't do anything if it isn't active.
    if(!currentItem())return;
    m_conflicts.clear();
    checkConflicts();
}

void keyGrabber::checkConflicts()
{
    int rows = rowCount();
    QStringList dupes;
    for(int i = 0;i < rows;i++) {
        dupes.append(item(i,1)->text());
    }

    for(int i = 0;i<dupes.size();i++) {
        if(dupes.at(i).isEmpty()) continue;
        QString search = dupes.at(i);
        int x = dupes.indexOf(search,i+1);
        while(x != -1) {
            m_conflicts.insert(i);
            m_conflicts.insert(x);
            x = dupes.indexOf(search,x+1);
        }
    }
 
}

void keyGrabber::paintEvent(QPaintEvent* event)
{
    QTableWidget::paintEvent(event);
    QPainter p(viewport());
    p.setPen(QColor(255,200,200,255));

    QSetIterator<int> it(m_conflicts);
    while(it.hasNext()) {
        int current = it.next();
        QRect rect = visualItemRect(item(current,1));
        p.drawRect(rect);
        p.fillRect(rect,QBrush(QColor(255,100,100,64)));
        viewport()->update();
    }
}

void keyGrabber::keyPressEvent(QKeyEvent* event)
{
    QString grab;
    QString key = QKeySequence(event->key()).toString();
    int modifiers = event->modifiers();

    if(modifiers & Qt::ControlModifier) grab.append("Ctrl+");
    if(modifiers & Qt::AltModifier) grab.append("Alt+");
    if(modifiers & Qt::MetaModifier) grab.append("Meta+");
    if(modifiers & Qt::ShiftModifier) grab.append("Shift+");
    
    switch(event->key()) {
        case Qt::Key_Alt:
        case Qt::Key_Control:
        case Qt::Key_Meta: 
        case Qt::Key_Shift:
            return;

        case Qt::Key_F1 ... Qt::Key_F35:
            grab.append(key);
            break;

        case Qt::Key_Backspace:
            if(modifiers) {
                grab.append(key);
            }else {
                item(currentRow(),1)->setText("");
                return;
            }
            break;

        default:
            if(modifiers) grab.append(key);
            break;
    }
    item(currentRow(),1)->setText(grab);
}
