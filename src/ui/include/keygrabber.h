#ifndef __KEYGRABBER_H__
#define __KEYGRABBER_H__
#include <QTableWidget>
#include <QKeyEvent>
class keyGrabber : public QTableWidget {
Q_OBJECT
public:
    keyGrabber(QWidget* parent = 0);
    void checkConflicts();

protected:
    void keyPressEvent(QKeyEvent* key);
    void paintEvent(QPaintEvent* event);

public slots:
    void itemChanged(QTableWidgetItem* item);

private:
    QSet<int> m_conflicts;
};

#endif
