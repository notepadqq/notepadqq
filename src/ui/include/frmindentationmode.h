#ifndef FRMINDENTATIONMODE_H
#define FRMINDENTATIONMODE_H

#include <QDialog>
#include "include/EditorNS/editor.h"

namespace Ui {
class frmIndentationMode;
}

class frmIndentationMode : public QDialog
{
    Q_OBJECT

public:
    explicit frmIndentationMode(QWidget *parent = 0);
    ~frmIndentationMode();

    void populateWidgets(EditorNS::Editor::IndentationMode indentationMode);
    EditorNS::Editor::IndentationMode indentationMode();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::frmIndentationMode *ui;
};

#endif // FRMINDENTATIONMODE_H
