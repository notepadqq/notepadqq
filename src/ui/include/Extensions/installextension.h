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
        explicit InstallExtension(const QString &extensionFilename, QWidget *parent = 0);
        ~InstallExtension();

    private slots:
        void on_btnCancel_clicked();
        void on_btnInstall_clicked();

    private:
        Ui::InstallExtension *ui;
        QString m_extensionFilename;
        QString m_uniqueName;
        QString m_runtime;

        static QString readExtensionManifest(const QString &archivePath);
        void installNodejsExtension(const QString &packagePath);
        static QString getAbsoluteExtensionFolder(const QString &extensionsPath, const QString &extensionUniqueName);
        void setUIClean(bool success);
    };

}
#endif // INSTALLEXTENSION_H
