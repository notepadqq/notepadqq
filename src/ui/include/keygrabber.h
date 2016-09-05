#ifndef __KEYGRABBER_H__
#define __KEYGRABBER_H__

#include <QTreeWidget>
#include <QKeyEvent>

class KeyGrabber : public QTreeWidget {
Q_OBJECT
public:
    KeyGrabber(QWidget* parent = 0);

    /**
     * @brief findConflicts Checks the entire tree for conflicting shortcuts.
     * @return true if conflicts were found, false otherwise.
     */
    bool findConflicts();

    /**
     * @brief addMenus Traverses the list of menus and adds the contained actions to
     *        the tree.
     * @param listOfMenus
     */
    void addMenus(const QList<const QMenu*>& listOfMenus);

    /**
     * @brief Convenience class that contains the tree item and corresponding
     *        action, as well as a few functions to modify it from outside.
     *        This way, users of KeyGrabber do not have to interact with the
     *        QTreeWidget objects themselves.
     */
    struct NodeItem {
    private:
        friend class KeyGrabber;
        QTreeWidgetItem* treeItem;
        QAction* action;

    public:
        QAction* getAction() { return action; }

        void setText(QString seq) { treeItem->setText(1,seq); }
        QString text() const { return treeItem->text(1); }

        NodeItem(QAction* a, QTreeWidgetItem* item) : treeItem(item), action(a) {}
    };

    /**
     * @brief getAllBindings Returns a list of NodeItems, one for each action/keysquence
     *        pair.
     */
    QList<NodeItem>& getAllBindings();

protected:
    void keyPressEvent(QKeyEvent* key);

public slots:
    void itemChanged(QTreeWidgetItem* item);

private:
    /**
     * @brief populateNode traverses the given QMenu recursively and adds its children to
     *        the rootItem as new nodes.
     */
    void populateNode(QTreeWidgetItem*& rootItem, const QMenu*);

    QList<NodeItem> m_allActions;
    bool m_testingForConflicts = false;
};

#endif
