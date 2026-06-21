#ifndef FRMINDENTATIONMODE_H
#define FRMINDENTATIONMODE_H

#include "include/EditorNS/editor_properties.h"

#include <QDialog>

namespace Ui {
class frmIndentationMode;
}

class frmIndentationMode : public QDialog
{
    Q_OBJECT

public:
    explicit frmIndentationMode(QWidget *parent = nullptr);
    ~frmIndentationMode();

    void populateWidgets(EditorNS::IndentationMode indentationMode);
    EditorNS::IndentationMode indentationMode();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::frmIndentationMode *ui;
};

#endif // FRMINDENTATIONMODE_H
