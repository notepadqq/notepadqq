#ifndef FRMPREFERENCES_H
#define FRMPREFERENCES_H

#include <QDialog>

namespace Ui {
class frmpreferences;
}

class frmpreferences : public QDialog
{
    Q_OBJECT
    
public:
    explicit frmpreferences(QWidget *parent = 0);
    ~frmpreferences();
    
private:
    Ui::frmpreferences *ui;
};

#endif // FRMPREFERENCES_H
