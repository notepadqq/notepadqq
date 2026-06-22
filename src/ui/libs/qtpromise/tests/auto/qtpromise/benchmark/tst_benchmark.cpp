/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#include "../shared/data.h"

#include <QtPromise>
#include <QtTest>

#ifdef Q_CC_MSVC
// MSVC calls the copy constructor on std::current_exception AND std::rethrow_exception
// https://stackoverflow.com/a/31820854
#    define EXCEPT_CALL_COPY_CTOR 1
#else
#    define EXCEPT_CALL_COPY_CTOR 0
#endif

class tst_benchmark : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void valueResolve();
    void valueReject();
    void valueThen();
    void valueFinally();
    void valueTap();
    void valueDelayed();
    void errorReject();
    void errorThen();

}; // class tst_benchmark

QTEST_MAIN(tst_benchmark)
#include "tst_benchmark.moc"

void tst_benchmark::valueResolve()
{
    { // should move the value when resolved by rvalue
        Data::logs().reset();
        QtPromise::QPromise<Data>{[&](const QtPromise::QPromiseResolve<Data>& resolve) {
            resolve(Data{42});
        }}.wait();

        QCOMPARE(Data::logs().ctor, 1);
        QCOMPARE(Data::logs().copy, 0);
        QCOMPARE(Data::logs().move, 1); // move value to the promise data
        QCOMPARE(Data::logs().refs, 0);
    }
    { // should create one copy of the value when resolved by lvalue
        Data::logs().reset();
        QtPromise::QPromise<Data>{[&](const QtPromise::QPromiseResolve<Data>& resolve) {
            Data value{42};
            resolve(value);
        }}.wait();

        QCOMPARE(Data::logs().ctor, 1);
        QCOMPARE(Data::logs().copy, 1); // copy value to the promise data
        QCOMPARE(Data::logs().move, 0);
        QCOMPARE(Data::logs().refs, 0);
    }
}

void tst_benchmark::valueReject()
{
    { // should not create any data if rejected
        Data::logs().reset();
        QtPromise::QPromise<Data>{[&](const QtPromise::QPromiseResolve<Data>&,
                                      const QtPromise::QPromiseReject<Data>& reject) {
            reject(QString{"foo"});
        }}.wait();

        QCOMPARE(Data::logs().ctor, 0);
        QCOMPARE(Data::logs().copy, 0);
        QCOMPARE(Data::logs().move, 0);
        QCOMPARE(Data::logs().refs, 0);
    }
}

void tst_benchmark::valueThen()
{
    { // should not copy value on continuation if fulfilled
        int value = -1;
        Data::logs().reset();
        QtPromise::QPromise<Data>::resolve(Data{42})
            .then([&](const Data& res) {
                value = res.value();
            })
            .wait();

        QCOMPARE(Data::logs().ctor, 1);
        QCOMPARE(Data::logs().copy, 0);
        QCOMPARE(Data::logs().move, 1); // move value to the promise data
        QCOMPARE(Data::logs().refs, 0);
        QCOMPARE(value, 42);
    }
    { // should not create value on continuation if rejected
        int value = -1;
        QString error;
        Data::logs().reset();
        QtPromise::QPromise<Data>::reject(QString{"foo"})
            .then(
                [&](const Data& res) {
                    value = res.value();
                },
                [&](const QString& err) {
                    error = err;
                })
            .wait();

        QCOMPARE(Data::logs().ctor, 0);
        QCOMPARE(Data::logs().copy, 0);
        QCOMPARE(Data::logs().move, 0);
        QCOMPARE(Data::logs().refs, 0);
        QCOMPARE(error, QString{"foo"});
        QCOMPARE(value, -1);
    }
    { // should move the returned value when fulfilled
        int value = -1;
        Data::logs().reset();
        QtPromise::QPromise<int>::resolve(42)
            .then([&](int res) {
                return Data{res + 2};
            })
            .then([&](const Data& res) {
                value = res.value();
            })
            .wait();

        QCOMPARE(Data::logs().ctor, 1);
        QCOMPARE(Data::logs().copy, 0);
        QCOMPARE(Data::logs().move, 1); // move values to the next promise data
        QCOMPARE(Data::logs().refs, 0);
        QCOMPARE(value, 44);
    }
    { // should not create any data if handler throws
        Data::logs().reset();
        QtPromise::QPromise<int>::resolve(42)
            .then([&](int res) {
                throw QString{"foo"};
                return Data{res + 2};
            })
            .wait();

        QCOMPARE(Data::logs().ctor, 0);
        QCOMPARE(Data::logs().copy, 0);
        QCOMPARE(Data::logs().move, 0);
        QCOMPARE(Data::logs().refs, 0);
    }
}

void tst_benchmark::valueDelayed()
{
    { // should not copy the value on continuation if fulfilled
        int value = -1;
        Data::logs().reset();
        QtPromise::QPromise<int>::resolve(42)
            .then([&](int res) {
                return QtPromise::QPromise<Data>::resolve(Data{res + 1});
            })
            .then([&](const Data& res) {
                value = res.value();
            })
            .wait();

        QCOMPARE(Data::logs().ctor, 1);
        QCOMPARE(Data::logs().copy, 0);
        QCOMPARE(Data::logs().move, 1); // move value to the input promise data
        QCOMPARE(Data::logs().refs, 0);
        QCOMPARE(value, 43);
    }
    { // should not create value on continuation if rejected
        Data::logs().reset();
        QtPromise::QPromise<int>::resolve(42)
            .then([&]() {
                return QtPromise::QPromise<Data>::reject(QString{"foo"});
            })
            .wait();

        QCOMPARE(Data::logs().ctor, 0);
        QCOMPARE(Data::logs().copy, 0);
        QCOMPARE(Data::logs().move, 0);
        QCOMPARE(Data::logs().refs, 0);
    }
}

void tst_benchmark::valueFinally()
{
    { // should not copy the value on continuation if fulfilled
        int value = -1;
        Data::logs().reset();
        QtPromise::QPromise<Data>::resolve(Data{42})
            .finally([&]() {
                value = 42;
            })
            .wait();

        QCOMPARE(Data::logs().ctor, 1);
        QCOMPARE(Data::logs().copy, 0);
        QCOMPARE(Data::logs().move, 1); // move value to the input and output promise data
        QCOMPARE(Data::logs().refs, 0);
        QCOMPARE(value, 42);
    }
    { // should not create value on continuation if rejected
        int value = -1;
        Data::logs().reset();
        QtPromise::QPromise<Data>::reject(QString{"foo"})
            .finally([&]() {
                value = 42;
            })
            .wait();

        QCOMPARE(Data::logs().ctor, 0);
        QCOMPARE(Data::logs().copy, 0);
        QCOMPARE(Data::logs().move, 0);
        QCOMPARE(Data::logs().refs, 0);
        QCOMPARE(value, 42);
    }
}

void tst_benchmark::valueTap()
{
    { // should not copy the value on continuation if fulfilled
        int value = -1;
        Data::logs().reset();
        QtPromise::QPromise<Data>::resolve(Data{42})
            .tap([&](const Data& res) {
                value = res.value();
            })
            .wait();

        QCOMPARE(Data::logs().ctor, 1);
        QCOMPARE(Data::logs().copy, 0);
        QCOMPARE(Data::logs().move, 1); // move value to the input and output promise data
        QCOMPARE(Data::logs().refs, 0);
        QCOMPARE(value, 42);
    }
    { // should not create value on continuation if rejected
        int value = -1;
        Data::logs().reset();
        QtPromise::QPromise<Data>::reject(QString{"foo"})
            .tap([&](const Data& res) {
                value = res.value();
            })
            .wait();

        QCOMPARE(Data::logs().ctor, 0);
        QCOMPARE(Data::logs().copy, 0);
        QCOMPARE(Data::logs().move, 0);
        QCOMPARE(Data::logs().refs, 0);
        QCOMPARE(value, -1);
    }
}

void tst_benchmark::errorReject()
{
    { // should create one copy of the error when rejected by rvalue
        Data::logs().reset();
        QtPromise::QPromise<int>{[&](const QtPromise::QPromiseResolve<int>&,
                                     const QtPromise::QPromiseReject<int>& reject) {
            reject(Data{42});
        }}.wait();

        QCOMPARE(Data::logs().ctor, 1);
        QCOMPARE(Data::logs().copy, 1 + EXCEPT_CALL_COPY_CTOR); // copy value in std::exception_ptr
        QCOMPARE(Data::logs().move, 0);
        QCOMPARE(Data::logs().refs, 0);
    }
    { // should create one copy of the error when rejected by lvalue (no extra copy)
        Data::logs().reset();
        QtPromise::QPromise<int>{[&](const QtPromise::QPromiseResolve<int>&,
                                     const QtPromise::QPromiseReject<int>& reject) {
            Data error{42};
            reject(error);
        }}.wait();

        QCOMPARE(Data::logs().ctor, 1);
        QCOMPARE(Data::logs().copy, 1 + EXCEPT_CALL_COPY_CTOR); // copy value to the promise data
        QCOMPARE(Data::logs().move, 0);
        QCOMPARE(Data::logs().refs, 0);
    }
}

void tst_benchmark::errorThen()
{
    { // should not copy error on continuation if rejected
        int value = -1;
        Data::logs().reset();
        QtPromise::QPromise<void>::reject(Data{42})
            .fail([&](const Data& res) {
                value = res.value();
            })
            .wait();

        QCOMPARE(Data::logs().ctor, 1);
        QCOMPARE(Data::logs().copy,
                 1 + 2 * EXCEPT_CALL_COPY_CTOR); // (initial) copy value in std::exception_ptr
        QCOMPARE(Data::logs().move, 0);
        QCOMPARE(Data::logs().refs, 0);
        QCOMPARE(value, 42);
    }
    { // should not copy error on continuation if rethrown
        int value = -1;
        Data::logs().reset();
        QtPromise::QPromise<void>::reject(Data{42})
            .fail([](const Data&) {
                throw;
            })
            .fail([&](const Data& res) {
                value = res.value();
            })
            .wait();

        QCOMPARE(Data::logs().ctor, 1);
        QCOMPARE(Data::logs().copy,
                 1 + 4 * EXCEPT_CALL_COPY_CTOR); // (initial) copy value in std::exception_ptr
        QCOMPARE(Data::logs().move, 0);
        QCOMPARE(Data::logs().refs, 0);
        QCOMPARE(value, 42);
    }
}
