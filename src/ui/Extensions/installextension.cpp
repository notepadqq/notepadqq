#include "include/Extensions/installextension.h"
#include "ui_installextension.h"

namespace Extensions {

    InstallExtension::InstallExtension(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::InstallExtension)
    {
        ui->setupUi(this);

        QFont f = ui->lblName->font();
        f.setPointSizeF(f.pointSizeF() * 1.2);
        ui->lblName->setFont(f);

        //setFixedSize(this->width(), this->height());
        setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMinMaxButtonsHint);
    }

    InstallExtension::~InstallExtension()
    {
        delete ui;
    }

}
