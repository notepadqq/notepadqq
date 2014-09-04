#ifndef FRMSEARCHREPLACE_H
#define FRMSEARCHREPLACE_H

#include "topeditorcontainer.h"
#include <QDialog>

namespace Ui {
class frmSearchReplace;
}

class frmSearchReplace : public QDialog
{
    Q_OBJECT

public:
    enum Tabs { TabSearch, TabReplace };
    explicit frmSearchReplace(TopEditorContainer *topEditorContainer, Tabs defaultTab = TabSearch, QWidget *parent = 0);
    ~frmSearchReplace();
private slots:
    void on_btnFindNext_clicked();

    void on_btnFindPrev_clicked();

    void on_btnFindNext_3_clicked();

    void on_btnFindPrev_3_clicked();

    void on_tabWidget_currentChanged(int index);

    void on_btnReplaceNext_clicked();

    void on_btnReplacePrev_clicked();

    void on_btnReplaceAll_clicked();

    void on_btnSelectAll_clicked();

private:
    Ui::frmSearchReplace*  ui;
    TopEditorContainer*    m_topEditorContainer;
    QString                m_lastSearch;
    Editor*                currentEditor();
    void search(QString string, bool isRegex, bool forward);
    QString plainTextToRegex(QString text);
    void replace(QString string, QString replacement, bool isRegex, bool forward);
    int replaceAll(QString string, QString replacement, bool isRegex);
    int selectAll(QString string, bool isRegex);
};

#endif // FRMSEARCHREPLACE_H
