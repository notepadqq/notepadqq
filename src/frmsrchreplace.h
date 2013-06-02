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
    bool newsearch;
    Ui::frmsrchreplace *ui;
    Qt::CaseSensitivity matchCase();



private slots:
    void buttonCountInstances_clicked();
    void buttonFindNext_clicked();
    void updateMode(int newIndex);
    void searchChanged();

};

#endif // FRMSRCHREPLACE_H
