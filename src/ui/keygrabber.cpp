#include "include/keygrabber.h"
#include <QHeaderView>
keyGrabber::keyGrabber(QWidget* parent) : QTableWidget(parent)
{
    horizontalHeader()->setProperty("stretchLastSection",true);
    horizontalHeader()->setProperty("defaultSectionSize",200);
    verticalHeader()->setProperty("visible",false);
    verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setDefaultSectionSize(20);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setAlternatingRowColors(true);
    setColumnCount(2);
    setHorizontalHeaderItem(0,new QTableWidgetItem("Action"));
    setHorizontalHeaderItem(1,new QTableWidgetItem("Keyboard Shortcut"));
    //Initialize all the cells
}

void keyGrabber::keyPressEvent(QKeyEvent* event)
{
    QString grab;
    int modifiers = event->modifiers();
    if(modifiers & Qt::ControlModifier) grab.append("Ctrl+");
    if(modifiers & Qt::ShiftModifier) grab.append("Shift+");
    if(modifiers & Qt::AltModifier) grab.append("Alt+");
    switch(event->key()) {
        case Qt::Key_Alt:
        case Qt::Key_Control:
        case Qt::Key_Meta: 
        case Qt::Key_Shift:
            break;
        case Qt::Key_Backspace:
            if(modifiers){
                grab.append(QKeySequence(event->key()).toString());
             }else {
                 grab.clear();
             }
             break;
        default:
            if(modifiers)grab.append(QKeySequence(event->key()).toString(QKeySequence::PortableText));
            break;
    }
    item(currentRow(),1)->setText(grab);
}
