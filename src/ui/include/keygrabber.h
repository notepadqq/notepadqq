#ifndef __KEYGRABBER_H__
#define __KEYGRABBER_H__
#include <QTableWidget>
#include <QKeyEvent>
class keyGrabber : public QTableWidget {
Q_OBJECT
public:
    keyGrabber(QWidget* parent = 0);

protected:
    void keyPressEvent(QKeyEvent* key);

};

#endif
