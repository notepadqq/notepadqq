#include "include/singleapplication.h"
#include <QLocalSocket>
#include <QDir>
#include <QRegularExpression>
#include <QDataStream>
#include "include/localcommunication.h"

#if defined(Q_OS_WIN)
#include <QLibrary>
#include <qt_windows.h>
typedef BOOL(WINAPI*PProcessIdToSessionId)(DWORD,DWORD*);
static PProcessIdToSessionId pProcessIdToSessionId = 0;
#endif
#if defined(Q_OS_UNIX)
#include <unistd.h>
#endif

/* ========= PROTOCOLS ===========
 *
 * |Ping:
 *
 *   Client             Server
 *      -- "NEW_CLIENT" -->
 *      <---- "HELLO" -----
 * _______________________________
 *
 * |Passing arguments:
 *
 *   Client             Server
 *      ----- "ARGS" ----->
 *      <----- "OK" -------
 *      ---- ..data.. ---->   where "data" is a list like
 *                            [working_dir, arg0, ..., argn]
 */

SingleApplication::SingleApplication(int &argc, char **argv) :
    QApplication(argc, argv)
{
}

QLocalSocket * SingleApplication::alreadyRunningInstance()
{
    QLocalSocket *socket = new QLocalSocket(this);
    socket->connectToServer(socketNameForUser());
    if (socket->waitForConnected(2000))
    {
        LocalCommunication::send("NEW_CLIENT", socket);
        QString reply = LocalCommunication::receive(socket);

        if (reply == "HELLO") {
            return socket;
        }
    }

    return nullptr;
}

void SingleApplication::startServer()
{
    if (m_localServer == nullptr) {
        m_localServer = new QLocalServer(this);
        connect(m_localServer, &QLocalServer::newConnection, this, &SingleApplication::newConnection);
        QString socketName = socketNameForUser();
        m_localServer->removeServer(socketName);
        m_localServer->listen(socketName);
    }
}

void SingleApplication::newConnection()
{
    while (m_localServer->hasPendingConnections()) {
        QLocalSocket *conn = m_localServer->nextPendingConnection();
        connect(conn, &QLocalSocket::readyRead, this, [=]() {
            QString message = LocalCommunication::receive(conn);

            if (message == "NEW_CLIENT")
            {
                LocalCommunication::send("HELLO", conn);
            }
            else if (message == "ARGS")
            {
                LocalCommunication::send("OK", conn);
                QByteArray ar = LocalCommunication::receiveRaw(conn);

                QDataStream args(&ar, QIODevice::ReadOnly);
                QStringList argList;
                args >> argList;

                if (!argList.isEmpty()) {
                    QString workingDir = argList.takeFirst();
                    emit receivedArguments(workingDir, argList);
                }
            }
        });
    }
}

QString SingleApplication::socketNameForUser()
{
    QString id = "notepadqq";
    QString prefix = id;
    if (id.isEmpty()) {
        id = QCoreApplication::applicationFilePath();
#if defined(Q_OS_WIN)
        id = id.toLower();
#endif
        prefix = id.section(QLatin1Char('/'), -1);
    }
    prefix.remove(QRegularExpression("[^a-zA-Z]"));
    prefix.truncate(6);

    QByteArray idc = id.toUtf8();
    quint16 idNum = qChecksum(idc.constData(), idc.size());
    QString socketName = QLatin1String("qtsingleapp-") + prefix
            + QLatin1Char('-') + QString::number(idNum, 16);

#if defined(Q_OS_WIN)
    if (!pProcessIdToSessionId) {
        QLibrary lib("kernel32");
        pProcessIdToSessionId = (PProcessIdToSessionId)lib.resolve("ProcessIdToSessionId");
    }
    if (pProcessIdToSessionId) {
        DWORD sessionId = 0;
        pProcessIdToSessionId(GetCurrentProcessId(), &sessionId);
        socketName += QLatin1Char('-') + QString::number(sessionId, 16);
    }
#else
    socketName += QLatin1Char('-') + QString::number(::getuid(), 16);
#endif

    return socketName;
}

bool SingleApplication::sendCommandLineArguments(QLocalSocket *socket)
{
    LocalCommunication::send("ARGS", socket);
    if (LocalCommunication::receive(socket) == "OK") {
        QStringList data = QApplication::arguments();
        data.prepend(QDir::currentPath());

        QByteArray ar;
        QDataStream args(&ar, QIODevice::WriteOnly);
        args << data;

        LocalCommunication::sendRaw(ar, socket);

        // Ensure that all the bytes get written:
        // after sendCommandLineArguments() returns, the
        // application will probably exit.
        return socket->waitForBytesWritten(5000);
    }
    return false;
}

bool SingleApplication::attachToOtherInstance()
{
    // See if there are other instances open, and send them the arguments.
    QLocalSocket *sck = alreadyRunningInstance();
    if (sck != nullptr) {
        bool ret = sendCommandLineArguments(sck);
        sck->disconnectFromServer();
        return ret;
    }

    return false;
}
