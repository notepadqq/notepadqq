#ifndef FRMABOUT_H
#define FRMABOUT_H

#include <QDialog>

namespace Ui {
class frmAbout;
}

class frmAbout : public QDialog
{
    Q_OBJECT

public:
    explicit frmAbout(QWidget *parent = nullptr);
    ~frmAbout();

private slots:
    void on_lblContributors_linkActivated(const QString &link);

    void on_pushButton_clicked();

    void on_btnLicense_clicked();

private:
    Ui::frmAbout *ui;
};

#endif // FRMABOUT_H
