#include "include/localsockethelpers.h"

#include <QLocalSocket>
#include <QPointer>

#include <memory>

QLocalSocket* LocalSocketHelpers::probe(
    QObject* owner, const QString& serverName, const std::function<bool(QLocalSocket*)>& handshake)
{
    auto socket = std::make_unique<QLocalSocket>(owner);
    socket->connectToServer(serverName);
    if (!socket->waitForConnected(2000) || !handshake(socket.get())) {
        return nullptr;
    }
    return socket.release();
}

void LocalSocketHelpers::deleteOnDisconnect(QLocalSocket* socket)
{
    if (socket == nullptr) {
        return;
    }
    QObject::connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
}
