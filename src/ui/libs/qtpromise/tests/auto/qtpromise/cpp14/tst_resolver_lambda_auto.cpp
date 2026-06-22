/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#include "../shared/utils.h"

#include <QtPromise>
#include <QtTest>

#include <memory>

class tst_cpp14_resolver_lambda_auto : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void resolverTwoAutoArgs();
    void resolverTwoAutoArgs_void();
};

QTEST_MAIN(tst_cpp14_resolver_lambda_auto)
#include "tst_resolver_lambda_auto.moc"

void tst_cpp14_resolver_lambda_auto::resolverTwoAutoArgs()
{
    QtPromise::QPromise<int> p0{[](auto resolve, auto reject) {
        Q_UNUSED(reject)
        resolve(42);
    }};
    QtPromise::QPromise<int> p1{[](auto resolve, const auto& reject) {
        Q_UNUSED(reject)
        resolve(42);
    }};
    QtPromise::QPromise<int> p2{[](const auto& resolve, auto reject) {
        Q_UNUSED(reject)
        resolve(42);
    }};
    QtPromise::QPromise<int> p3{[](const auto& resolve, const auto& reject) {
        Q_UNUSED(reject)
        resolve(42);
    }};

    for (const auto& p : {p0, p1, p2, p3}) {
        QCOMPARE(p.isFulfilled(), true);
        QCOMPARE(waitForError(p, -1), -1);
        QCOMPARE(waitForValue(p, -1), 42);
    }
}

void tst_cpp14_resolver_lambda_auto::resolverTwoAutoArgs_void()
{
    QtPromise::QPromise<void> p0{[](auto resolve, auto reject) {
        Q_UNUSED(reject)
        resolve();
    }};
    QtPromise::QPromise<void> p1{[](auto resolve, const auto& reject) {
        Q_UNUSED(reject)
        resolve();
    }};
    QtPromise::QPromise<void> p2{[](const auto& resolve, auto reject) {
        Q_UNUSED(reject)
        resolve();
    }};
    QtPromise::QPromise<void> p3{[](const auto& resolve, const auto& reject) {
        Q_UNUSED(reject)
        resolve();
    }};

    for (const auto& p : {p0, p1, p2, p3}) {
        QCOMPARE(p.isFulfilled(), true);
        QCOMPARE(waitForError(p, -1), -1);
        QCOMPARE(waitForValue(p, -1, 42), 42);
    }
}
