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
    explicit frmSearchReplace(TopEditorContainer *topEditorContainer, QWidget *parent = 0);
    ~frmSearchReplace();

private slots:
    void on_btnFindNext_clicked();

    void on_btnFindPrev_clicked();

private:
    Ui::frmSearchReplace*  ui;
    TopEditorContainer*    m_topEditorContainer;
    QString                m_lastSearch;
    Editor*                currentEditor();
    void search(QString string, bool isRegex, bool forward);
};

#endif // FRMSEARCHREPLACE_H
