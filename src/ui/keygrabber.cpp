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
        case Qt::Key_F1: grab.append("F1");break;
        case Qt::Key_F2: grab.append("F2");break;
        case Qt::Key_F3: grab.append("F3");break;
        case Qt::Key_F4: grab.append("F4");break;
        case Qt::Key_F5: grab.append("F5");break;
        case Qt::Key_F6: grab.append("F6");break;
        case Qt::Key_F7: grab.append("F7");break;
        case Qt::Key_F8: grab.append("F8");break;
        case Qt::Key_F9: grab.append("F9");break;
        case Qt::Key_F10: grab.append("F10");break;
        case Qt::Key_F11: grab.append("F11");break;
        case Qt::Key_F12: grab.append("F12");break;
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
