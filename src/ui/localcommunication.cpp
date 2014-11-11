#include "include/localcommunication.h"

LocalCommunication::LocalCommunication()
{
}

bool LocalCommunication::sendRaw(QByteArray data, QLocalSocket *socket)
{
    if (data.length() > MAX_PKT_SIZE)
        return false;

    QByteArray header = QString::number(data.length())
            .rightJustified(numOfDigits(MAX_PKT_SIZE), '0').toUtf8();

    int written = 0;
    while (written < header.length()) {
        written += socket->write(header);
    }

    written = 0;
    while (written < data.length()) {
        written += socket->write(data);
    }
}

bool LocalCommunication::send(QString message, QLocalSocket *socket)
{
    return sendRaw(message.toUtf8(), socket);
}

QByteArray LocalCommunication::receiveRaw(QLocalSocket *socket)
{
    QByteArray header;
    int bytes = QString(numOfDigits(MAX_PKT_SIZE), '0').toUtf8().length();

    while (header.length() < bytes) {
        header.append(socket->read(bytes - header.length()));
        socket->waitForReadyRead(100);
    }

    QString sizeStr = QString::fromUtf8(header);
    bool ok;
    int size = sizeStr.toInt(&ok);

    if (ok)
    {
        QByteArray msgData;

        while (msgData.length() < size) {
            msgData.append(socket->read(size - msgData.length()));
        }

        return msgData;
    } else {
        return QByteArray();
    }
}

QString LocalCommunication::receive(QLocalSocket *socket)
{
    return QString::fromUtf8(receiveRaw(socket));
}

int LocalCommunication::numOfDigits(int n) {
    int digits = 0;
    if (n <= 0) {
        n = -n;
        ++digits;
    }
    while (n) {
        n /= 10;
        ++digits;
    }
    return digits;
}
