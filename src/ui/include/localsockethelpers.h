#ifndef LOCALSOCKETHELPERS_H
#define LOCALSOCKETHELPERS_H

#include <functional>

class QObject;
class QLocalSocket;
class QString;

namespace LocalSocketHelpers {
// Probes a server with scoped ownership and releases the socket only after a valid handshake.
QLocalSocket* probe(QObject* owner, const QString& serverName, const std::function<bool(QLocalSocket*)>& handshake);

// Makes a server-side socket self-cleaning when its peer disconnects.
void deleteOnDisconnect(QLocalSocket* socket);
} // namespace LocalSocketHelpers

#endif
