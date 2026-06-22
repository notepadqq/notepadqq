/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#include "../shared/utils.h"

#include <QtConcurrent>
#include <QtPromise>
#include <QtTest>

#include <memory>

class tst_helpers_attempt : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void voidResult();
    void typedResult();
    void futureResult();
    void promiseResult();
    void functorThrows();
    void callWithParams();
};

QTEST_MAIN(tst_helpers_attempt)
#include "tst_attempt.moc"

void tst_helpers_attempt::voidResult()
{
    auto p = QtPromise::attempt([]() {});

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<void>>::value));
    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(waitForValue(p, -1, 42), 42);
}

void tst_helpers_attempt::typedResult()
{
    auto p = QtPromise::attempt([]() {
        return QString{"foo"};
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QString>>::value));
    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(waitForValue(p, QString{}), QString{"foo"});
}

void tst_helpers_attempt::futureResult()
{
    auto p = QtPromise::attempt([]() {
        return QtConcurrent::run([]() {
            return QString{"foo"};
        });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QString>>::value));
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, QString{}), QString{"foo"});
}

void tst_helpers_attempt::promiseResult()
{
    auto p = QtPromise::attempt([]() {
        return QtPromise::resolve(42).delay(200);
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<int>>::value));
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, -1), 42);
}

void tst_helpers_attempt::functorThrows()
{
    auto p = QtPromise::attempt([]() {
        if (true) {
            throw QString{"bar"};
        }
        return 42;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<int>>::value));
    QCOMPARE(p.isRejected(), true);
    QCOMPARE(waitForError(p, QString{}), QString{"bar"});
}

void tst_helpers_attempt::callWithParams()
{
    auto p = QtPromise::attempt(
        [&](int i, const QString& s) {
            return QString{"%1:%2"}.arg(i).arg(s);
        },
        42,
        "foo");

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QString>>::value));
    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(waitForValue(p, QString{}), QString{"42:foo"});
}
