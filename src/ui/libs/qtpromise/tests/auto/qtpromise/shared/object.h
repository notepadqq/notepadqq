#ifndef QTPROMISE_TESTS_AUTO_SHARED_SENDER_H
#define QTPROMISE_TESTS_AUTO_SHARED_SENDER_H

// Qt
#include <QObject>

class Object : public QObject
{
    Q_OBJECT

public:
    bool hasConnections() const { return m_connections > 0; }

Q_SIGNALS:
    void noArgSignal();
    void oneArgSignal(const QString& v);
    void twoArgsSignal(int v1, const QString& v0);

protected:
    int m_connections = 0;
    void connectNotify(const QMetaMethod&) Q_DECL_OVERRIDE { ++m_connections; }
    void disconnectNotify(const QMetaMethod&) Q_DECL_OVERRIDE { --m_connections; }
};

#endif // ifndef QTPROMISE_TESTS_AUTO_SHARED_SENDER_H
