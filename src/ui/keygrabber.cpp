#include "include/keygrabber.h"
#include <QHeaderView>
#include <QPainter>

#include <QDebug>
#include <QMenu>

KeyGrabber::KeyGrabber(QWidget* parent) : QTreeWidget(parent)
{
    setColumnCount(2);
    setColumnWidth(0, 400);
    setAlternatingRowColors(true);
    setHeaderLabels(QStringList() << tr("Action") << tr("Keyboard Shortcut"));

    connect(this, &QTreeWidget::itemChanged, this, &KeyGrabber::itemChanged);
}

bool KeyGrabber::hasConflicts() const
{
    return m_firstConflict != nullptr;
}

void KeyGrabber::scrollToConflict()
{
    if (!m_firstConflict) {
        return;
    }

    scrollTo(indexFromItem(m_firstConflict));
}

void KeyGrabber::itemChanged(QTreeWidgetItem*)
{
    // Don't do anything if it isn't active or we're in the middle of testing for conflicts
    // since item->setBackground() emits an itemChanged signal.
    if (!currentItem() || m_testingForConflicts) {
        return;
    }

    checkForConflicts();
}

void KeyGrabber::checkForConflicts()
{
    m_testingForConflicts = true;

    // Find conflicts among shortcuts. We take a list of all shortcuts, sort them, then
    // walk through them and compare them for equality.
    QList<NodeItem> allNodes = m_allActions;
    qSort(allNodes.begin(), allNodes.end(), [](const NodeItem& a, const NodeItem&b ) {
        return a.treeItem->text(1) < b.treeItem->text(1);
    });

    for (const auto& n : allNodes) {
        n.treeItem->setBackground(1, QBrush());
    }

    m_firstConflict = nullptr;

    for (int i = 0; i < allNodes.count() - 1; i++) {
        QTreeWidgetItem* current = allNodes[i].treeItem;
        QTreeWidgetItem* next = allNodes[i+1].treeItem;

        if (current->text(1).isEmpty()) {
            continue;
        }

        if (current->text(1) == next->text(1)){
            current->setBackground(1, QBrush(QColor(255, 100, 100, 64)));
            next->setBackground(1, QBrush(QColor(255, 100, 100, 64)));

            if (!m_firstConflict) {
                m_firstConflict = current;
            }
        }
    }

    m_testingForConflicts = false;
}

void KeyGrabber::addMenus(const QList<const QMenu*>& listOfMenus)
{
    for (const QMenu* menu : listOfMenus) {

        if (menu->objectName() == "menu_Language" || menu->objectName().isEmpty()) {
            continue;
        }

        auto item = new QTreeWidgetItem();
        item->setText(0, menu->menuAction()->iconText());

        populateNode(item, menu);

        addTopLevelItem(item);
    }
}

QList<KeyGrabber::NodeItem>& KeyGrabber::getAllBindings()
{
    return m_allActions;
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
                QTreeWidget::keyPressEvent(event);
                return;
        }
    }

    const QVariant& data = currentItem()->data(0, Qt::UserRole);
    if(!data.isValid()) {
        QTreeWidget::keyPressEvent(event);
        return;
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
                currentItem()->setText(1, "");
                return;
            }
            break;

        default:
            if (modifiers) {
                grab.append(key);
            }
            break;
    }

    currentItem()->setText(1, grab);
}

void KeyGrabber::populateNode(QTreeWidgetItem*& rootItem, const QMenu* menu) {
    for (QAction* action : menu->actions()) {

        if (action->isSeparator()) {
            continue;

        } else if (action->menu()) {
            // If the action is a sub-menu, we add a new tree node and populate it with
            // the children of this menu. Exceptions that should not be added can be
            // specified here.
            if (action->menu()->objectName() == "menuRecent_Files")
                continue;

            auto item = new QTreeWidgetItem();
            item->setText(0, action->iconText());
            populateNode(item, action->menu());
            rootItem->addChild(item);
        } else {
            // Any action that does not have an object name or label won't be added.
            // This way we exclude things such as the entries in the recent documents
            // list.
            if (action->objectName().isEmpty() || action->iconText().isEmpty()) {
                continue;
            }

            auto item = new QTreeWidgetItem();
            item->setText(0, action->iconText());
            item->setText(1, action->shortcut().toString());
            // Every action that can have a shortcut gets this user value set to 'true'.
            // This is used in keyPressEvent to figure out whether to read the key evts.
            item->setData(0, Qt::UserRole, true);
            rootItem->addChild(item);

            m_allActions.push_back( NodeItem {action, item} );
        }
    }
}
