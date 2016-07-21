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
        case Qt::Key_F1:
        case Qt::Key_F2:
        case Qt::Key_F3:
        case Qt::Key_F4:
        case Qt::Key_F5:
        case Qt::Key_F6:
        case Qt::Key_F7:
        case Qt::Key_F8:
        case Qt::Key_F9:
        case Qt::Key_F10:
        case Qt::Key_F11:
        case Qt::Key_F12:
            grab.append(QKeySequence(event->key()).toString());
            break;
        default:
            if(modifiers)grab.append(QKeySequence(event->key()).toString());
            break;
    }
    item(currentRow(),1)->setText(grab);
}
