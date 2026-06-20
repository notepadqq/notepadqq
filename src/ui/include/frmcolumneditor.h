#ifndef FRMCOLUMNEDITOR_H
#define FRMCOLUMNEDITOR_H

#include "include/EditorNS/editor.h"

#include <QDialog>

namespace Ui {
class frmColumnEditor;
}

class frmColumnEditor : public QDialog
{
    Q_OBJECT

public:
    explicit frmColumnEditor(QWidget *parent = nullptr);
    ~frmColumnEditor();

    QString insTxt() const;

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::frmColumnEditor *ui;
};

#endif // FRMCOLUMNEDITOR_H
