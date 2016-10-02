#ifndef _RUNPREFERENCES_H_
#define _RUNPREFERENCES_H_

#include <QDialog>
#include <QTableWidget>

class RunPreferences : public QDialog
{
    Q_OBJECT

private:
    QTableWidget *m_commands;
public:
    RunPreferences(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~RunPreferences();
};

#endif
