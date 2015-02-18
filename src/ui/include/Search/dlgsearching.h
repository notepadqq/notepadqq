#ifndef DLGSEARCHING_H
#define DLGSEARCHING_H

#include <QDialog>

namespace Ui {
class dlgSearching;
}

class dlgSearching : public QDialog
{
    Q_OBJECT

public:
    explicit dlgSearching(QWidget *parent = 0);
    ~dlgSearching();

    void setTitle(const QString &title);
    QString title() const;
    void setText(const QString &text);
    QString text() const;

private slots:
    void on_btnCancel_clicked();

private:
    Ui::dlgSearching *ui;
};

#endif // DLGSEARCHING_H
