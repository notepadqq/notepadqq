#ifndef FRMLINENUMBERCHOOSER_H
#define FRMLINENUMBERCHOOSER_H

#include <QDialog>

namespace Ui {
class frmLineNumberChooser;
}

class frmLineNumberChooser : public QDialog
{
    Q_OBJECT

public:
    explicit frmLineNumberChooser(int min, int max, int defaultValue, QWidget *parent = nullptr);
    ~frmLineNumberChooser();

    int value();

private:
    Ui::frmLineNumberChooser *ui;
};

#endif // FRMLINENUMBERCHOOSER_H
