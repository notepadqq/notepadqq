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

class tst_helpers_filter : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void emptySequence();
    void filterValues();
    void delayedFulfilled();
    void delayedRejected();
    void functorThrows();
    void functorArguments();
    void preserveOrder();
    void sequenceTypes();
};

QTEST_MAIN(tst_helpers_filter)
#include "tst_filter.moc"

namespace {

template<class Sequence>
struct SequenceTester
{
    static void exec()
    {
        auto inputs = Sequence{42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
        auto p = QtPromise::filter(inputs, [](int v, ...) {
            return v % 3 == 0;
        });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<Sequence>>::value));
        QCOMPARE(waitForValue(p, Sequence{}), (Sequence{42, 45, 48, 51}));
    }
};

} // anonymous namespace

void tst_helpers_filter::emptySequence()
{
    auto p = QtPromise::filter(QVector<int>{}, [](int v, ...) {
        return v % 2 == 0;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>{}), QVector<int>{});
}

void tst_helpers_filter::filterValues()
{
    auto p = QtPromise::filter(QVector<int>{42, 43, 44}, [](int v, ...) {
        return v % 2 == 0;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>{}), (QVector<int>{42, 44}));
}

void tst_helpers_filter::delayedFulfilled()
{
    auto p = QtPromise::filter(QVector<int>{42, 43, 44}, [](int v, ...) {
        return QtPromise::QPromise<bool>{[&](const QtPromise::QPromiseResolve<bool>& resolve) {
            QtPromisePrivate::qtpromise_defer([=]() {
                resolve(v % 2 == 0);
            });
        }};
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>{}), (QVector<int>{42, 44}));
}

void tst_helpers_filter::delayedRejected()
{
    auto p = QtPromise::filter(QVector<int>{42, 43, 44}, [](int v, ...) {
        return QtPromise::QPromise<bool>{[&](const QtPromise::QPromiseResolve<bool>& resolve,
                                             const QtPromise::QPromiseReject<bool>& reject) {
            QtPromisePrivate::qtpromise_defer([=]() {
                if (v == 44) {
                    reject(QString{"foo"});
                }
                resolve(true);
            });
        }};
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QVector<int>>>::value));
    QCOMPARE(waitForError(p, QString{}), QString{"foo"});
}

void tst_helpers_filter::functorThrows()
{
    auto p = QtPromise::filter(QVector<int>{42, 43, 44}, [](int v, ...) {
        if (v == 44) {
            throw QString{"foo"};
        }
        return true;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QVector<int>>>::value));
    QCOMPARE(waitForError(p, QString{}), QString{"foo"});
}

void tst_helpers_filter::functorArguments()
{
    QMap<int, int> args;
    auto p = QtPromise::filter(QVector<int>{42, 43, 44}, [&](int v, int i) {
        args[v] = i;
        return i % 2 == 0;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>{}), (QVector<int>{42, 44}));
    QMap<int, int> expected{{42, 0}, {43, 1}, {44, 2}};
    QCOMPARE(args, expected);
}

void tst_helpers_filter::preserveOrder()
{
    auto p = QtPromise::filter(QVector<int>{500, 100, 300, 250, 400}, [](int v, ...) {
        return QtPromise::QPromise<bool>::resolve(v > 200).delay(v);
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QtPromise::QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>{}), (QVector<int>{500, 300, 250, 400}));
}

void tst_helpers_filter::sequenceTypes()
{
    SequenceTester<QList<int>>::exec();
    SequenceTester<QVector<int>>::exec();
    SequenceTester<std::list<int>>::exec();
    SequenceTester<std::vector<int>>::exec();
}
