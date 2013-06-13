#ifndef FRMSRCHREPLACE_H
#define FRMSRCHREPLACE_H

#include <QDialog>
#include "searchengine.h"
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
    bool newsearch;
    bool regexp;
    bool casesense;
    bool wholeword;
    bool wrap;
    bool forward;

    void updateParameters();
    searchengine* se();

    void showEvent(QShowEvent *e);
    void closeEvent(QCloseEvent *e);

private slots:
    void buttonCountInstances_clicked();
    void buttonFindNext_clicked();
    void buttonReplace_clicked();
    void buttonReplaceAll_clicked();
    void updateMode(int newIndex);
    void setNewSearch(bool isnew=true);

};

#endif // FRMSRCHREPLACE_H
