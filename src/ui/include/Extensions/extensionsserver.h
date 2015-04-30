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
        bool startServer(QString name);
        void broadcastMessage(const QJsonObject &message);
        QSharedPointer<RuntimeSupport> runtimeSupport();
        QString socketPath();

    signals:

    public slots:

    private slots:
        void on_newConnection();

    private:
        QSharedPointer<RuntimeSupport> m_extensionsRTS;
        QLocalServer *m_server;
        QString m_bufferedData = "";
        QList<QLocalSocket *> m_sockets;

        void on_clientMessage(QLocalSocket *socket);
        void on_socketDisconnected(QLocalSocket *socket);
        void sendMessage(QLocalSocket *socket, const QJsonObject &message);
    };

}

#endif // EXTENSIONSSERVER_H
