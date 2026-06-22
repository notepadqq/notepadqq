/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#include "../shared/utils.h"

#include <QtPromise>
#include <QtTest>

#include <chrono>

class tst_qpromise_timeout : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void fulfilled();
    void rejected();
    void timeout();

    void fulfilledStdChrono();
    void rejectedStdChrono();
    void timeoutStdChrono();
};

QTEST_MAIN(tst_qpromise_timeout)
#include "tst_timeout.moc"

void tst_qpromise_timeout::fulfilled()
{
    QElapsedTimer timer;
    qint64 elapsed = -1;

    timer.start();

    auto p = QtPromise::QPromise<int>{[](const QtPromise::QPromiseResolve<int>& resolve) {
                 QTimer::singleShot(1000, [=]() {
                     resolve(42);
                 });
             }}.timeout(2000)
                 .finally([&]() {
                     elapsed = timer.elapsed();
                 });

    QCOMPARE(waitForValue(p, -1), 42);
    QCOMPARE(p.isFulfilled(), true);
    QVERIFY(elapsed < 2000);
}

void tst_qpromise_timeout::rejected()
{
    QElapsedTimer timer;
    qint64 elapsed = -1;

    timer.start();

    auto p = QtPromise::QPromise<int>{[](const QtPromise::QPromiseResolve<int>&,
                                         const QtPromise::QPromiseReject<int>& reject) {
                 QTimer::singleShot(1000, [=]() {
                     reject(QString{"foo"});
                 });
             }}.timeout(2000)
                 .finally([&]() {
                     elapsed = timer.elapsed();
                 });

    QCOMPARE(waitForError(p, QString{}), QString{"foo"});
    QCOMPARE(p.isRejected(), true);
    QVERIFY(elapsed < 2000);
}

void tst_qpromise_timeout::timeout()
{
    QElapsedTimer timer;
    qint64 elapsed = -1;
    bool failed = false;

    timer.start();

    auto p = QtPromise::QPromise<int>{[](const QtPromise::QPromiseResolve<int>& resolve) {
                 QTimer::singleShot(4000, [=]() {
                     resolve(42);
                 });
             }}.timeout(2000)
                 .finally([&]() {
                     elapsed = timer.elapsed();
                 });

    p.fail([&](const QtPromise::QPromiseTimeoutException&) {
         failed = true;
         return -1;
     }).wait();

    QCOMPARE(waitForValue(p, -1), -1);
    QCOMPARE(p.isRejected(), true);
    QCOMPARE(failed, true);

    // Qt::CoarseTimer (default) Coarse timers try to
    // keep accuracy within 5% of the desired interval.
    // Require accuracy within 6% for passing the test.
    QVERIFY(elapsed >= static_cast<qint64>(2000 * 0.94));
    QVERIFY(elapsed <= static_cast<qint64>(2000 * 1.06));
}

void tst_qpromise_timeout::fulfilledStdChrono()
{
    QElapsedTimer timer;
    qint64 elapsed = -1;

    timer.start();

    auto p = QtPromise::QPromise<int>{[](const QtPromise::QPromiseResolve<int>& resolve) {
                 QTimer::singleShot(1000, [=]() {
                     resolve(42);
                 });
             }}.timeout(std::chrono::seconds{2})
                 .finally([&]() {
                     elapsed = timer.elapsed();
                 });

    QCOMPARE(waitForValue(p, -1), 42);
    QCOMPARE(p.isFulfilled(), true);
    QVERIFY(elapsed < 2000);
}

void tst_qpromise_timeout::rejectedStdChrono()
{
    QElapsedTimer timer;
    qint64 elapsed = -1;

    timer.start();

    auto p = QtPromise::QPromise<int>{[](const QtPromise::QPromiseResolve<int>&,
                                         const QtPromise::QPromiseReject<int>& reject) {
                 QTimer::singleShot(1000, [=]() {
                     reject(QString{"foo"});
                 });
             }}.timeout(std::chrono::seconds{2})
                 .finally([&]() {
                     elapsed = timer.elapsed();
                 });

    QCOMPARE(waitForError(p, QString{}), QString{"foo"});
    QCOMPARE(p.isRejected(), true);
    QVERIFY(elapsed < 2000);
}

void tst_qpromise_timeout::timeoutStdChrono()
{
    QElapsedTimer timer;
    qint64 elapsed = -1;
    bool failed = false;

    timer.start();

    auto p = QtPromise::QPromise<int>{[](const QtPromise::QPromiseResolve<int>& resolve) {
                 QTimer::singleShot(4000, [=]() {
                     resolve(42);
                 });
             }}.timeout(std::chrono::seconds{2})
                 .finally([&]() {
                     elapsed = timer.elapsed();
                 });

    p.fail([&](const QtPromise::QPromiseTimeoutException&) {
         failed = true;
         return -1;
     }).wait();

    QCOMPARE(waitForValue(p, -1), -1);
    QCOMPARE(p.isRejected(), true);
    QCOMPARE(failed, true);

    // Qt::CoarseTimer (default) Coarse timers try to
    // keep accuracy within 5% of the desired interval.
    // Require accuracy within 6% for passing the test.
    QVERIFY(elapsed >= static_cast<qint64>(2000 * 0.94));
    QVERIFY(elapsed <= static_cast<qint64>(2000 * 1.06));
}
