#include "../../shared/object.h"
#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_helpers_connect : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // connect(QObject* sender, Signal resolver)
    void resolveOneSenderNoArg();
    void resolveOneSenderOneArg();
    void resolveOneSenderManyArgs();

    // connect(QObject* sender, Signal resolver, Signal rejecter)
    void rejectOneSenderNoArg();
    void rejectOneSenderOneArg();
    void rejectOneSenderManyArgs();
    void rejectOneSenderDestroyed();

    // connect(QObject* s0, Signal resolver, QObject* s1, Signal rejecter)
    void rejectTwoSendersNoArg();
    void rejectTwoSendersOneArg();
    void rejectTwoSendersManyArgs();
    void rejectTwoSendersDestroyed();
};

QTEST_MAIN(tst_helpers_connect)
#include "tst_connect.moc"

void tst_helpers_connect::resolveOneSenderNoArg()
{
    Object sender;
    QtPromisePrivate::qtpromise_defer([&]() {
        Q_EMIT sender.noArgSignal();
    });

    auto p = QtPromise::connect(&sender, &Object::noArgSignal);
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QCOMPARE(sender.hasConnections(), true);
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, -1, 42), 42);
    QCOMPARE(sender.hasConnections(), false);
}

void tst_helpers_connect::resolveOneSenderOneArg()
{
    Object sender;
    QtPromisePrivate::qtpromise_defer([&]() {
        Q_EMIT sender.oneArgSignal("foo");
    });

    auto p = QtPromise::connect(&sender, &Object::oneArgSignal);
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));
    QCOMPARE(sender.hasConnections(), true);
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, QString()), QString("foo"));
    QCOMPARE(sender.hasConnections(), false);
}

void tst_helpers_connect::resolveOneSenderManyArgs()
{
    Object sender;
    QtPromisePrivate::qtpromise_defer([&]() {
        Q_EMIT sender.twoArgsSignal(42, "foo");
    });

    auto p = QtPromise::connect(&sender, &Object::twoArgsSignal);
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));
    QCOMPARE(sender.hasConnections(), true);
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, -1), 42);
    QCOMPARE(sender.hasConnections(), false);
}

void tst_helpers_connect::rejectOneSenderNoArg()
{
    Object sender;
    QtPromisePrivate::qtpromise_defer([&]() {
        Q_EMIT sender.noArgSignal();
    });

    auto p = QtPromise::connect(&sender, &Object::oneArgSignal, &Object::noArgSignal);
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));
    QCOMPARE(sender.hasConnections(), true);
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForRejected<QPromiseUndefinedException>(p), true);
    QCOMPARE(sender.hasConnections(), false);
}

void tst_helpers_connect::rejectOneSenderOneArg()
{
    Object sender;
    QtPromisePrivate::qtpromise_defer([&]() {
        Q_EMIT sender.oneArgSignal("bar");
    });

    auto p = QtPromise::connect(&sender, &Object::noArgSignal, &Object::oneArgSignal);
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QCOMPARE(sender.hasConnections(), true);
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForError(p, QString()), QString("bar"));
    QCOMPARE(sender.hasConnections(), false);
}

void tst_helpers_connect::rejectOneSenderManyArgs()
{
    Object sender;
    QtPromisePrivate::qtpromise_defer([&]() {
        Q_EMIT sender.twoArgsSignal(42, "bar");
    });

    auto p = QtPromise::connect(&sender, &Object::noArgSignal, &Object::twoArgsSignal);
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QCOMPARE(sender.hasConnections(), true);
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForError(p, -1), 42);
    QCOMPARE(sender.hasConnections(), false);
}

void tst_helpers_connect::rejectOneSenderDestroyed()
{
    Object* sender = new Object();
    QtPromisePrivate::qtpromise_defer([&]() {
        sender->deleteLater();
    });

    auto p = QtPromise::connect(sender, &Object::twoArgsSignal);
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForRejected<QPromiseContextException>(p), true);
}

void tst_helpers_connect::rejectTwoSendersNoArg()
{
    Object s0, s1;
    QtPromisePrivate::qtpromise_defer([&]() {
        Q_EMIT s1.noArgSignal();
    });

    auto p = QtPromise::connect(&s0, &Object::noArgSignal, &s1, &Object::noArgSignal);
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QCOMPARE(s0.hasConnections(), true);
    QCOMPARE(s1.hasConnections(), true);
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForRejected<QPromiseUndefinedException>(p), true);
    QCOMPARE(s0.hasConnections(), false);
    QCOMPARE(s1.hasConnections(), false);
}

void tst_helpers_connect::rejectTwoSendersOneArg()
{
    Object s0, s1;
    QtPromisePrivate::qtpromise_defer([&]() {
        Q_EMIT s1.oneArgSignal("bar");
    });

    auto p = QtPromise::connect(&s0, &Object::noArgSignal, &s1, &Object::oneArgSignal);
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QCOMPARE(s0.hasConnections(), true);
    QCOMPARE(s1.hasConnections(), true);
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForError(p, QString()), QString("bar"));
    QCOMPARE(s0.hasConnections(), false);
    QCOMPARE(s1.hasConnections(), false);
}

void tst_helpers_connect::rejectTwoSendersManyArgs()
{
    Object s0, s1;
    QtPromisePrivate::qtpromise_defer([&]() {
        Q_EMIT s1.twoArgsSignal(42, "bar");
    });

    auto p = QtPromise::connect(&s0, &Object::noArgSignal, &s1, &Object::twoArgsSignal);
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QCOMPARE(s0.hasConnections(), true);
    QCOMPARE(s1.hasConnections(), true);
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForError(p, -1), 42);
    QCOMPARE(s0.hasConnections(), false);
    QCOMPARE(s1.hasConnections(), false);
}

void tst_helpers_connect::rejectTwoSendersDestroyed()
{
    Object* s0 = new Object();
    Object* s1 = new Object();

    QtPromisePrivate::qtpromise_defer([&]() {
        QObject::connect(s1, &QObject::destroyed, [&]() {
            // Let's first delete s1, then resolve s0 and make sure
            // we don't reject when the rejecter object is destroyed.
            Q_EMIT s0->noArgSignal();
        });

        s1->deleteLater();
    });

    auto p = QtPromise::connect(s0, &Object::noArgSignal, s1, &Object::twoArgsSignal);
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, -1, 42), 42);
}
