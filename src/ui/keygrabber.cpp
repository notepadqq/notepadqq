#include "include/keygrabber.h"
#include <QHeaderView>
#include <QPainter>

KeyGrabber::KeyGrabber(QWidget* parent) : QTableWidget(parent)
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
    setHorizontalHeaderItem(0, new QTableWidgetItem("Action"));
    setHorizontalHeaderItem(1, new QTableWidgetItem("Keyboard Shortcut"));
    connect(this, &QTableWidget::itemChanged, this, &KeyGrabber::itemChanged);
}

void KeyGrabber::itemChanged(QTableWidgetItem*)
{
    // Don't do anything if it isn't active.
    if (!currentItem()) {
        return;
    }

    m_conflicts.clear();
    checkConflicts();
}

void KeyGrabber::checkConflicts()
{
    int rows = rowCount();
    QStringList dupes;
    for (int i = 0; i < rows; i++) {
        dupes.append(item(i, 1)->text());
    }

    for (int i = 0; i < dupes.size(); i++) {
        if (dupes.at(i).isEmpty()) {
            continue;
        }

        QString search = dupes.at(i);
        int x = dupes.indexOf(search, i+1);
        while (x != -1) {
            m_conflicts.insert(i);
            m_conflicts.insert(x);
            x = dupes.indexOf(search, x+1);
        }
    }
}

void KeyGrabber::paintEvent(QPaintEvent* event)
{
    QTableWidget::paintEvent(event);
    QPainter p(viewport());
    p.setPen(QColor(255, 200, 200, 255));

    QSetIterator<int> it(m_conflicts);
    while (it.hasNext()) {
        int current = it.next();
        QRect rect = visualItemRect(item(current, 1));
        p.drawRect(rect);
        p.fillRect(rect, QBrush(QColor(255, 100, 100, 64)));
        viewport()->update();
    }
}

void KeyGrabber::keyPressEvent(QKeyEvent* event)
{
    QString grab;
    QString key = QKeySequence(event->key()).toString();
    int modifiers = event->modifiers();

    // Keys to ignore (e.g. navigation keys with no modifiers)
    if (modifiers == 0) {
        switch(event->key()) {
            case Qt::Key_Up:
            case Qt::Key_Down:
                QTableWidget::keyPressEvent(event);
                return;
        }
    }

    if (modifiers & Qt::ControlModifier) grab.append("Ctrl+");
    if (modifiers & Qt::AltModifier) grab.append("Alt+");
    if (modifiers & Qt::MetaModifier) grab.append("Meta+");
    if (modifiers & Qt::ShiftModifier) grab.append("Shift+");
    
    switch(event->key()) {
        case Qt::Key_Alt:
        case Qt::Key_Control:
        case Qt::Key_Meta: 
        case Qt::Key_Shift:
            return;

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
        case Qt::Key_F13:
        case Qt::Key_F14:
        case Qt::Key_F15:
        case Qt::Key_F16:
        case Qt::Key_F17:
        case Qt::Key_F18:
        case Qt::Key_F19:
        case Qt::Key_F20:
        case Qt::Key_F21:
        case Qt::Key_F22:
        case Qt::Key_F23:
        case Qt::Key_F24:
        case Qt::Key_F25:
        case Qt::Key_F26:
        case Qt::Key_F27:
        case Qt::Key_F28:
        case Qt::Key_F29:
        case Qt::Key_F30:
        case Qt::Key_F31:
        case Qt::Key_F32:
        case Qt::Key_F33:
        case Qt::Key_F34:
        case Qt::Key_F35:
            grab.append(key);
            break;

        case Qt::Key_Backspace:
            if (modifiers) {
                grab.append(key);
            } else {
                item(currentRow(), 1)->setText("");
                return;
            }
            break;

        default:
            if (modifiers) {
                grab.append(key);
            }
            break;
    }

    item(currentRow(), 1)->setText(grab);
}
