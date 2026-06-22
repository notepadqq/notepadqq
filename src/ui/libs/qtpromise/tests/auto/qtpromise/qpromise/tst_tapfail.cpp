/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#include "../shared/utils.h"

#include <QtPromise>
#include <QtTest>

class tst_qpromise_tapfail : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void fulfilled();
    void fulfilled_void();
    void rejected();
    void rejected_void();
    void throws();
    void throws_void();
    void delayedResolved();
    void delayedRejected();
};

QTEST_MAIN(tst_qpromise_tapfail)
#include "tst_tapfail.moc"

void tst_qpromise_tapfail::fulfilled()
{
    int value = -1;
    auto p = QtPromise::QPromise<int>::resolve(42).tapFail([&]() {
        value = 43;
    });

    QCOMPARE(waitForValue(p, 42), 42);
    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(value, -1);
}

void tst_qpromise_tapfail::fulfilled_void()
{
    int value = -1;
    auto p = QtPromise::QPromise<void>::resolve().tapFail([&]() {
        value = 43;
    });

    QCOMPARE(waitForValue(p, -1, 42), 42);
    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(value, -1);
}

void tst_qpromise_tapfail::rejected()
{
    QStringList errors;

    auto p0 = QtPromise::QPromise<int>::reject(QString{"foo"}).tapFail([&](const QString& err) {
        errors << "1:" + err;
    });

    auto p1 = p0.fail([&](const QString& err) {
        errors << "2:" + err;
        return 43;
    });

    QCOMPARE(waitForError(p0, QString{}), QString{"foo"});
    QCOMPARE(waitForValue(p1, -1), 43);
    QCOMPARE(p0.isRejected(), true);
    QCOMPARE(p1.isFulfilled(), true);
    QCOMPARE(errors, (QStringList{"1:foo", "2:foo"}));
}

void tst_qpromise_tapfail::rejected_void()
{
    QStringList errors;

    auto p0 = QtPromise::QPromise<void>::reject(QString{"foo"}).tapFail([&](const QString& err) {
        errors << "1:" + err;
    });

    auto p1 = p0.fail([&](const QString& err) {
        errors << "2:" + err;
    });

    QCOMPARE(waitForError(p0, QString{}), QString{"foo"});
    QCOMPARE(waitForValue(p1, -1, 43), 43);
    QCOMPARE(p0.isRejected(), true);
    QCOMPARE(p1.isFulfilled(), true);
    QCOMPARE(errors, (QStringList{"1:foo", "2:foo"}));
}

void tst_qpromise_tapfail::throws()
{
    auto p = QtPromise::QPromise<int>::reject(QString{"foo"}).tapFail([&]() {
        throw QString{"bar"};
    });

    QCOMPARE(waitForError(p, QString{}), QString{"bar"});
    QCOMPARE(p.isRejected(), true);
}

void tst_qpromise_tapfail::throws_void()
{
    auto p = QtPromise::QPromise<void>::reject(QString{"foo"}).tapFail([&]() {
        throw QString{"bar"};
    });

    QCOMPARE(waitForError(p, QString{}), QString{"bar"});
    QCOMPARE(p.isRejected(), true);
}

void tst_qpromise_tapfail::delayedResolved()
{
    QVector<int> values;
    auto p = QtPromise::QPromise<int>::reject(QString{"foo"}).tapFail([&]() {
        QtPromise::QPromise<void> p{[&](const QtPromise::QPromiseResolve<void>& resolve) {
            QtPromisePrivate::qtpromise_defer([=, &values]() {
                values << 3;
                resolve(); // ignored!
            });
        }};

        values << 2;
        return p;
    });

    QCOMPARE(waitForError(p, QString{}), QString{"foo"});
    QCOMPARE(values, (QVector<int>{2, 3}));
}

void tst_qpromise_tapfail::delayedRejected()
{
    QVector<int> values;
    auto p = QtPromise::QPromise<int>::reject(QString{"foo"}).tapFail([&]() {
        QtPromise::QPromise<void> p{[&](const QtPromise::QPromiseResolve<void>&,
                                        const QtPromise::QPromiseReject<void>& reject) {
            QtPromisePrivate::qtpromise_defer([=, &values]() {
                values << 3;
                reject(QString{"bar"});
            });
        }};

        values << 2;
        return p;
    });

    QCOMPARE(waitForError(p, QString{}), QString{"bar"});
    QCOMPARE(values, (QVector<int>{2, 3}));
}
