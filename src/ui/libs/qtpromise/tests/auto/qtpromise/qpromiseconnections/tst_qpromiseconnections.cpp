#include "../shared/object.h"
#include "../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromiseconnections : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void connections();
    void destruction();
    void senderDestroyed();

}; // class tst_qpromiseconnections

QTEST_MAIN(tst_qpromiseconnections)
#include "tst_qpromiseconnections.moc"

void tst_qpromiseconnections::connections()
{
    Object sender;

    QPromiseConnections connections;
    QCOMPARE(sender.hasConnections(), false);
    QCOMPARE(connections.count(), 0);

    connections << connect(&sender, &Object::noArgSignal, [=]() {});
    QCOMPARE(sender.hasConnections(), true);
    QCOMPARE(connections.count(), 1);

    connections << connect(&sender, &Object::twoArgsSignal, [=]() {});
    QCOMPARE(sender.hasConnections(), true);
    QCOMPARE(connections.count(), 2);

    connections.disconnect();
    QCOMPARE(sender.hasConnections(), false);
    QCOMPARE(connections.count(), 0);
}

void tst_qpromiseconnections::destruction()
{
    Object sender;

    {
        QPromiseConnections connections;
        QCOMPARE(sender.hasConnections(), false);
        QCOMPARE(connections.count(), 0);

        connections << connect(&sender, &Object::noArgSignal, [=]() {});
        QCOMPARE(sender.hasConnections(), true);
        QCOMPARE(connections.count(), 1);
    }

    QCOMPARE(sender.hasConnections(), false);
}

void tst_qpromiseconnections::senderDestroyed()
{
    QPromiseConnections connections;
    QCOMPARE(connections.count(), 0);

    {
        Object sender;
        QCOMPARE(sender.hasConnections(), false);

        connections << connect(&sender, &Object::noArgSignal, [=]() {});
        QCOMPARE(sender.hasConnections(), true);
        QCOMPARE(connections.count(), 1);
    }

    // should not throw
    connections.disconnect();
    QCOMPARE(connections.count(), 0);
}
