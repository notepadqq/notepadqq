#ifndef FRMENCODINGCHOOSER_H
#define FRMENCODINGCHOOSER_H

#include <QDialog>

namespace Ui {
class frmEncodingChooser;
}

class frmEncodingChooser : public QDialog
{
    Q_OBJECT

public:
    explicit frmEncodingChooser(QWidget *parent = nullptr);
    ~frmEncodingChooser();

    QTextCodec *selectedCodec() const;
    void setEncoding(QTextCodec *codec);
    void setInfoText(const QString &text);

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::frmEncodingChooser *ui;
};

#endif // FRMENCODINGCHOOSER_H
