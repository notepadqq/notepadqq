/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#include "../shared/utils.h"

#include <QtPromise>
#include <QtTest>

class tst_deprecations_qpromise_all : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void emptySequence();
    void emptySequence_void();
    void allPromisesSucceed();
    void allPromisesSucceed_void();
    void atLeastOnePromiseReject();
    void atLeastOnePromiseReject_void();
    void preserveOrder();
    void sequenceTypes();
    void sequenceTypes_void();
};

QTEST_MAIN(tst_deprecations_qpromise_all)
#include "tst_qpromise_all.moc"

namespace {

template<class Sequence>
struct SequenceTester
{
    Q_STATIC_ASSERT((std::is_same<typename Sequence::value_type, QtPromise::QPromise<int>>::value));

    static void exec()
    {
        Sequence promises{QtPromise::resolve(42), QtPromise::resolve(43), QtPromise::resolve(44)};

        promises.push_back(QtPromise::resolve(45));
        promises.insert(++promises.begin(), QtPromise::resolve(46));
        promises.pop_back();

        auto p = QtPromise::QPromise<int>::all(promises);

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QVector<int>>>::value));

        QCOMPARE(p.isPending(), true);
        QCOMPARE(waitForValue(p, QVector<int>{}), (QVector<int>{42, 46, 43, 44}));
    }
};

template<template<typename, typename...> class Sequence, typename... Args>
struct SequenceTester<Sequence<QtPromise::QPromise<void>, Args...>>
{
    static void exec()
    {
        Sequence<QtPromise::QPromise<void>, Args...> promises{QtPromise::resolve(),
                                                              QtPromise::resolve(),
                                                              QtPromise::resolve()};

        promises.push_back(QtPromise::resolve());
        promises.insert(++promises.begin(), QtPromise::resolve());
        promises.pop_back();

        auto p = QtPromise::QPromise<void>::all(promises);

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<void>>::value));

        QCOMPARE(p.isPending(), true);
        QCOMPARE(waitForValue(p, -1, 42), 42);
    }
};

} // anonymous namespace

void tst_deprecations_qpromise_all::emptySequence()
{
    auto p = QtPromise::QPromise<int>::all(QVector<QtPromise::QPromise<int>>{});

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QVector<int>>>::value));

    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(waitForValue(p, QVector<int>{}), QVector<int>{});
}

void tst_deprecations_qpromise_all::emptySequence_void()
{
    auto p = QtPromise::QPromise<void>::all(QVector<QtPromise::QPromise<void>>{});

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<void>>::value));

    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(waitForValue(p, -1, 42), 42);
}

void tst_deprecations_qpromise_all::allPromisesSucceed()
{
    auto p0 = QtPromise::resolve(42);
    auto p1 = QtPromise::resolve(44);
    auto p2 = QtPromise::QPromise<int>{[](const QtPromise::QPromiseResolve<int>& resolve) {
        QtPromisePrivate::qtpromise_defer([=]() {
            resolve(43);
        });
    }};

    auto p = QtPromise::QPromise<int>::all(QVector<QtPromise::QPromise<int>>{p0, p2, p1});

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QVector<int>>>::value));

    QCOMPARE(p0.isFulfilled(), true);
    QCOMPARE(p1.isFulfilled(), true);
    QCOMPARE(p2.isPending(), true);
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, QVector<int>{}), (QVector<int>{42, 43, 44}));
    QCOMPARE(p2.isFulfilled(), true);
}

void tst_deprecations_qpromise_all::allPromisesSucceed_void()
{
    auto p0 = QtPromise::resolve();
    auto p1 = QtPromise::resolve();
    auto p2 = QtPromise::QPromise<void>{[](const QtPromise::QPromiseResolve<void>& resolve) {
        QtPromisePrivate::qtpromise_defer([=]() {
            resolve();
        });
    }};

    auto p = QtPromise::QPromise<void>::all(QVector<QtPromise::QPromise<void>>{p0, p2, p1});

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<void>>::value));

    QCOMPARE(p0.isFulfilled(), true);
    QCOMPARE(p1.isFulfilled(), true);
    QCOMPARE(p2.isPending(), true);
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, -1, 42), 42);
    QCOMPARE(p2.isFulfilled(), true);
}

void tst_deprecations_qpromise_all::atLeastOnePromiseReject()
{
    auto p0 = QtPromise::resolve(42);
    auto p1 = QtPromise::resolve(44);
    auto p2 = QtPromise::QPromise<int>{
        [](const QtPromise::QPromiseResolve<int>&, const QtPromise::QPromiseReject<int>& reject) {
            QtPromisePrivate::qtpromise_defer([=]() {
                reject(QString{"foo"});
            });
        }};

    auto p = QtPromise::QPromise<int>::all(QVector<QtPromise::QPromise<int>>{p0, p2, p1});

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QVector<int>>>::value));

    QCOMPARE(p0.isFulfilled(), true);
    QCOMPARE(p1.isFulfilled(), true);
    QCOMPARE(p2.isPending(), true);
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForError(p, QString{}), QString{"foo"});
    QCOMPARE(p2.isRejected(), true);
}

void tst_deprecations_qpromise_all::atLeastOnePromiseReject_void()
{
    auto p0 = QtPromise::resolve();
    auto p1 = QtPromise::resolve();
    auto p2 = QtPromise::QPromise<void>{
        [](const QtPromise::QPromiseResolve<void>&, const QtPromise::QPromiseReject<void>& reject) {
            QtPromisePrivate::qtpromise_defer([=]() {
                reject(QString{"foo"});
            });
        }};

    auto p = QtPromise::QPromise<void>::all(QVector<QtPromise::QPromise<void>>{p0, p2, p1});

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<void>>::value));

    QCOMPARE(p0.isFulfilled(), true);
    QCOMPARE(p1.isFulfilled(), true);
    QCOMPARE(p2.isPending(), true);
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForError(p, QString{}), QString{"foo"});
    QCOMPARE(p2.isRejected(), true);
}

void tst_deprecations_qpromise_all::preserveOrder()
{
    auto p0 = QtPromise::resolve(42).delay(500);
    auto p1 = QtPromise::resolve(43).delay(100);
    auto p2 = QtPromise::resolve(44).delay(250);

    auto p = QtPromise::QPromise<int>::all(QVector<QtPromise::QPromise<int>>{p0, p1, p2});

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QVector<int>>>::value));

    QCOMPARE(p0.isPending(), true);
    QCOMPARE(p1.isPending(), true);
    QCOMPARE(p2.isPending(), true);
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, QVector<int>{}), (QVector<int>{42, 43, 44}));
    QCOMPARE(p0.isFulfilled(), true);
    QCOMPARE(p1.isFulfilled(), true);
    QCOMPARE(p2.isFulfilled(), true);
}

// QVector::push_back/append isn't supported since it requires a default
// constructor (see https://github.com/simonbrunel/qtpromise/issues/3)

void tst_deprecations_qpromise_all::sequenceTypes()
{
    SequenceTester<QList<QtPromise::QPromise<int>>>::exec();
    // SequenceTester<QVector<QtPromise::QPromise<int>>>::exec();
    SequenceTester<std::list<QtPromise::QPromise<int>>>::exec();
    SequenceTester<std::vector<QtPromise::QPromise<int>>>::exec();
}

void tst_deprecations_qpromise_all::sequenceTypes_void()
{
    SequenceTester<QList<QtPromise::QPromise<void>>>::exec();
    // SequenceTester<QVector<QtPromise::QPromise<void>>>::exec();
    SequenceTester<std::list<QtPromise::QPromise<void>>>::exec();
    SequenceTester<std::vector<QtPromise::QPromise<void>>>::exec();
}
