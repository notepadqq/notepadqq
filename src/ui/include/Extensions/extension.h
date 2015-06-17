#ifndef EXTENSION_H
#define EXTENSION_H

#include <QObject>
#include <QProcess>

namespace Extensions {

    class Extension : public QObject
    {
        Q_OBJECT
    public:
        explicit Extension(QString path, QString serverSocketPath);
        ~Extension();

        QString id() const;
        QString name() const;
        static QJsonObject getManifest(const QString &extensionPath);
    signals:

    public slots:

    private slots:
        void on_processError(QProcess::ProcessError error);
    private:
        QString m_extensionId;
        QString m_name;
        QString m_runtime;
        void failedToLoadExtension(QString path, QString reason);
        QProcess *process = nullptr;
    };

}

#endif // EXTENSION_H
