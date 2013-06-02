#ifndef FRMSRCHREPLACE_H
#define FRMSRCHREPLACE_H

#include <QDialog>

namespace Ui {
class frmsrchreplace;
}

class frmsrchreplace : public QDialog
{
    Q_OBJECT
    
public:
    explicit frmsrchreplace(QWidget *parent = 0);
    ~frmsrchreplace();
    
private:
    Ui::frmsrchreplace *ui;
};

#endif // FRMSRCHREPLACE_H
