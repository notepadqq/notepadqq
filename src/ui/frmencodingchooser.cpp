#include "include/frmencodingchooser.h"
#include "ui_frmencodingchooser.h"
#include "include/notepadqq.h"
#include <QTextCodec>

frmEncodingChooser::frmEncodingChooser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmEncodingChooser)
{
    ui->setupUi(this);

    layout()->setSizeConstraint(QLayout::SetFixedSize);

    for (int mib : QTextCodec::availableMibs()) {
        QTextCodec *codec = QTextCodec::codecForMib(mib);
        ui->cmbEncodings->addItem(QString::fromUtf8(codec->name()), codec->mibEnum());
    }

    ui->cmbEncodings->model()->sort(0);
}

void frmEncodingChooser::setEncoding(QTextCodec *codec)
{
    int mib = codec->mibEnum();
    for (int i = 0; i < ui->cmbEncodings->count(); i++) {
        if (mib == ui->cmbEncodings->itemData(i)) {
            ui->cmbEncodings->setCurrentIndex(i);
            break;
        }
    }
}

void frmEncodingChooser::setInfoText(const QString &text)
{
    ui->label->setText(text);
}

QTextCodec *frmEncodingChooser::selectedCodec() const
{
    if (ui->cmbEncodings->currentIndex() >= 0) {
        return QTextCodec::codecForMib(ui->cmbEncodings->currentData().toInt());
    } else {
        return QTextCodec::codecForMib(MIB_UTF_8);
    }
}

frmEncodingChooser::~frmEncodingChooser()
{
    delete ui;
}

void frmEncodingChooser::on_buttonBox_accepted()
{
    accept();
}

void frmEncodingChooser::on_buttonBox_rejected()
{
    reject();
}
