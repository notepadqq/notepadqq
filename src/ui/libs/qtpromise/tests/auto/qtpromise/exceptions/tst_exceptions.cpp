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

class tst_exceptions : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void canceled();
    void context();
    void conversion();
    void timeout();
    void undefined();

}; // class tst_exceptions

QTEST_MAIN(tst_exceptions)
#include "tst_exceptions.moc"

namespace {

template<class E>
void verify()
{
    auto p = QtPromise::resolve(QtConcurrent::run([]() {
        throw E();
    }));
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForRejected<E>(p), true);
    QCOMPARE(p.isRejected(), true);
}

} // anonymous namespace

void tst_exceptions::canceled()
{
    verify<QtPromise::QPromiseCanceledException>();
}

void tst_exceptions::context()
{
    verify<QtPromise::QPromiseContextException>();
}

void tst_exceptions::conversion()
{
    verify<QtPromise::QPromiseConversionException>();
}

void tst_exceptions::timeout()
{
    verify<QtPromise::QPromiseTimeoutException>();
}

void tst_exceptions::undefined()
{
    verify<QtPromise::QPromiseUndefinedException>();
}
