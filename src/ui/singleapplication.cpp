#include "include/singleapplication.h"

#ifndef USE_DBUS
#include "include/localcommunication.h"

#include <QDataStream>
#include <QDir>
#include <QLocalSocket>
#include <QRegularExpression>
#endif

#if defined(Q_OS_WIN)
#include <QLibrary>
#include <qt_windows.h>
typedef BOOL(WINAPI*PProcessIdToSessionId)(DWORD,DWORD*);
static PProcessIdToSessionId pProcessIdToSessionId = 0;
#endif
#if defined(Q_OS_UNIX)
#include <unistd.h>
#endif

SingleApplication::SingleApplication(int &argc, char **argv) :
    QApplication(argc, argv)
{
}

#ifdef USE_DBUS

#include <QtCore/QCoreApplication>
#include <QtDBus/QtDBus>
#define SERVICE_NAME "com.notepadqq.Notepadqq"

void SingleApplication::startServer()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        qDebug() <<  "Cannot connect to the D-Bus session bus.\n"
                        "To start it, run:\n"
                        "\teval `dbus-launch --auto-syntax`\n";
        Q_ASSERT(true);
    }

    if (!QDBusConnection::sessionBus().registerService(SERVICE_NAME)) {
        qDebug() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        Q_ASSERT(true);
    }

    bool ok = QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportScriptableSlots);
    qDebug() << "Started and object registered: " << ok;
}

bool SingleApplication::attachToOtherInstance() {
    QDBusInterface iface(SERVICE_NAME, "/", "", QDBusConnection::sessionBus());
    if (!iface.isValid())
        return false;

    QDBusReply<void> reply = iface.call("receive", QDir::currentPath(), QApplication::arguments());
    if (reply.isValid())
        return true;

    qDebug() <<  "Call to SingleApplication::receive() failed: " << reply.error().message();
    return false;
}

void SingleApplication::receive(const QString& workingDirectory, const QStringList& arguments) {
    if (arguments.isEmpty()) {
        qWarning() << "Invalid DBus message with empty arguments parameter received.";
        return;
    }

    emit receivedArguments(workingDirectory, arguments);
}


#else //ifdef USE_DBUS

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

#endif //ifdef USE_DBUS
