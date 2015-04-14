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

    void ExtensionsServer::startServer(QString path)
    {
        QLocalServer::removeServer(path);

        m_server = new QLocalServer(this);
        if (!m_server->listen(path)) {

            /*QMessageBox::critical(this, tr("Extensions"),
                                  tr("Unable to communicate with the extensions: %1.")
                                  .arg(m_server->errorString()));*/
            // FIXME Display error
            return;
        }

        connect(m_server, &QLocalServer::newConnection, this, &ExtensionsServer::on_newConnection);
    }

    void ExtensionsServer::on_newConnection()
    {
        QLocalSocket *client = m_server->nextPendingConnection();
        if (client != nullptr) {
            connect(client, &QLocalSocket::readyRead, this, [=]() { on_clientMessage(client); });
            connect(client, &QLocalSocket::disconnected, client, &QLocalSocket::deleteLater);
        }
    }

    void ExtensionsServer::on_clientMessage(QLocalSocket *socket)
    {
        // FIXME Can reopen a stream multiple times on the same socket?
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
}
