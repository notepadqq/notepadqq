#include "include/Extensions/extensionsserver.h"
#include <QMessageBox>
#include <QLocalSocket>
#include <QJsonDocument>

namespace Extensions {

    ExtensionsServer::ExtensionsServer(QSharedPointer<RuntimeSupport> extensionsRTS, QObject *parent) :
        QObject(parent),
        m_extensionsRTS(extensionsRTS)
    {

    }

    ExtensionsServer::~ExtensionsServer()
    {

    }

    bool ExtensionsServer::startServer(QString name)
    {
        QLocalServer::removeServer(name);

        m_server = new QLocalServer(this);
        if (!m_server->listen(name)) {

            qCritical() << QString("Unable to start extensions server %1. Extensions will not be loaded.")
                           .arg(name).toStdString().c_str();

            return false;
        }

        connect(m_server, &QLocalServer::newConnection, this, &ExtensionsServer::on_newConnection);
        return true;
    }

    QString ExtensionsServer::socketPath()
    {
        if (m_server->isListening()) {
            return m_server->fullServerName();
        } else {
            return QString();
        }
    }

    void ExtensionsServer::on_newConnection()
    {
        QLocalSocket *client = m_server->nextPendingConnection();
        if (client != nullptr) {
            m_sockets.append(client);

            connect(client, &QLocalSocket::readyRead, this, [=] { on_clientMessage(client); });
            connect(client, &QLocalSocket::disconnected, this, [=] { on_socketDisconnected(client); });

            sendMessage(client, m_extensionsRTS->getCurrentExtensionStartedEvent());
        }
    }

    void ExtensionsServer::on_clientMessage(QLocalSocket *socket)
    {
        QTextStream stream(socket);

        if (stream.atEnd())
            return;

        while (1) {
            QString jsonRequest = stream.readLine();
            if (jsonRequest.isNull())
                break;

            QJsonDocument request = QJsonDocument::fromJson(jsonRequest.toUtf8());

            QJsonObject response = m_extensionsRTS->handleRequest(request.object());
            QString jsonResponse = QString(
                        QJsonDocument(response).toJson(QJsonDocument::Compact))
                    .trimmed();

            stream << jsonResponse << "\n";
        }
    }

    void ExtensionsServer::on_socketDisconnected(QLocalSocket *socket)
    {
        m_sockets.removeAll(socket);
        socket->deleteLater();
        Q_ASSERT(m_sockets.contains(socket) == false);
    }

    void ExtensionsServer::broadcastMessage(const QJsonObject &message)
    {
        QString jsonMessage = QString(
                    QJsonDocument(message).toJson(QJsonDocument::Compact))
                .trimmed();

        for (QLocalSocket *socket : m_sockets) {
            if (socket->isOpen()) {
                QTextStream stream(socket);
                stream << jsonMessage << "\n";
            }
        }
    }

    void ExtensionsServer::sendMessage(QLocalSocket *socket, const QJsonObject &message)
    {
        if (socket->isOpen()) {
            QString jsonMessage = QString(
                        QJsonDocument(message).toJson(QJsonDocument::Compact))
                    .trimmed();

            QTextStream stream(socket);
            stream << jsonMessage << "\n";
        }
    }

    QSharedPointer<RuntimeSupport> ExtensionsServer::runtimeSupport()
    {
        return m_extensionsRTS;
    }
}
