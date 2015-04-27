#ifndef INSTALLEXTENSION_H
#define INSTALLEXTENSION_H

#include <QDialog>

namespace Ui {
class InstallExtension;
}

namespace Extensions {

    class InstallExtension : public QDialog
    {
        Q_OBJECT

    public:
        explicit InstallExtension(QWidget *parent = 0);
        ~InstallExtension();

    private:
        Ui::InstallExtension *ui;
    };

}
#endif // INSTALLEXTENSION_H
