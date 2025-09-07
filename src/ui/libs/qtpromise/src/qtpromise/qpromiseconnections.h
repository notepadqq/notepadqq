#ifndef QTPROMISE_QPROMISECONNECTIONS_H
#define QTPROMISE_QPROMISECONNECTIONS_H

// Qt
#include <QSharedPointer>

namespace QtPromise {

class QPromiseConnections
{
public:
    QPromiseConnections() : m_d(new Data()) { }

    int count() const { return m_d->connections.count(); }

    void disconnect() const { m_d->disconnect(); }

    void operator<<(QMetaObject::Connection&& other) const
    {
        m_d->connections.append(std::move(other));
    }

private:
    struct Data
    {
        QVector<QMetaObject::Connection> connections;

        ~Data() {
            if (!connections.empty()) {
                qWarning("QPromiseConnections: destroyed with unhandled connections.");
                disconnect();
            }
        }

        void disconnect() {
            for (const auto& connection: connections) {
                QObject::disconnect(connection);
            }
            connections.clear();
        }
    };

    QSharedPointer<Data> m_d;
};

} // namespace QtPromise

#endif // QTPROMISE_QPROMISECONNECTIONS_H
