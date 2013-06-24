#ifndef FRMPREFERENCES_H
#define FRMPREFERENCES_H

#include <QDialog>

namespace Ui {
class frmpreferences;
}

class frmpreferences : public QDialog
{
    Q_OBJECT

public:
    explicit frmpreferences(QWidget *parent = 0);
    ~frmpreferences();

private:
    Ui::frmpreferences *ui;

private slots:
    void _on_toggle_tabbar_hide(bool on);
    void _on_toggle_tabbar_vertical( bool on );
    void _on_toggle_tabbar_lock( bool on );
    void _on_toggle_tabbar_reduce( bool on );
    void _on_toggle_tabbar_highlight( bool on );
};

#endif // FRMPREFERENCES_H
