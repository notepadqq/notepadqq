#ifndef EXTENSIONSSERVER_H
#define EXTENSIONSSERVER_H

#include <QObject>
#include <QLocalServer>
#include "include/Extensions/runtimesupport.h"

namespace Extensions {

    class ExtensionsServer : public QObject
    {
        Q_OBJECT
    public:
        explicit ExtensionsServer(QSharedPointer<RuntimeSupport> extensionsRTS, QObject *parent = 0);
        ~ExtensionsServer();
        void startServer(QString path);
    signals:

    public slots:

    private slots:
        void on_newConnection();
        void on_clientMessage(QLocalSocket *socket);

    private:
        QSharedPointer<RuntimeSupport> m_extensionsRTS;
        QLocalServer *m_server;
        QString m_bufferedData = "";

    };

}

#endif // EXTENSIONSSERVER_H
