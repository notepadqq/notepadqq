/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#include "../shared/utils.h"

#include <QtPromise>
#include <QtTest>

class tst_qpromise_fail : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void sameType();
    void baseClass();
    void catchAll();
    // TODO: sync / async
    void functionPtrHandlers();
    void stdFunctionHandlers();
    void stdBindHandlers();
    void lambdaHandlers();
};

QTEST_MAIN(tst_qpromise_fail)
#include "tst_fail.moc"

namespace {

const QString kErr{"0.42"};
const float kRes = 0.42f;
const float kFail = -1.f;

float fnNoArg()
{
    return kErr.toFloat();
}
float fnArgByVal(QString e)
{
    return e.toFloat();
}
float fnArgByRef(const QString& e)
{
    return e.toFloat();
}

class Klass
{
public: // STATICS
    static float kFnNoArg() { return kErr.toFloat(); }
    static float kFnArgByVal(QString e) { return e.toFloat(); }
    static float kFnArgByRef(const QString& e) { return e.toFloat(); }

public:
    Klass(float v) : m_v{v} { }

    float fnNoArg() const { return m_v; }
    float fnArgByVal(QString v) const { return v.toFloat() + m_v; }
    float fnArgByRef(const QString& v) const { return v.toFloat() + m_v; }

private:
    const float m_v;
};

} // namespace

void tst_qpromise_fail::sameType()
{
    // http://en.cppreference.com/w/cpp/error/exception
    auto p = QtPromise::QPromise<int>::reject(std::out_of_range("foo"));

    QString error;
    p.fail([&](const std::domain_error& e) {
         error += QString{e.what()} + "0";
         return -1;
     })
        .fail([&](const std::out_of_range& e) {
            error += QString{e.what()} + "1";
            return -1;
        })
        .fail([&](const std::exception& e) {
            error += QString{e.what()} + "2";
            return -1;
        })
        .wait();

    QCOMPARE(error, QString{"foo1"});
}

void tst_qpromise_fail::baseClass()
{
    // http://en.cppreference.com/w/cpp/error/exception
    auto p = QtPromise::QPromise<int>::reject(std::out_of_range("foo"));

    QString error;
    p.fail([&](const std::runtime_error& e) {
         error += QString{e.what()} + "0";
         return -1;
     })
        .fail([&](const std::logic_error& e) {
            error += QString{e.what()} + "1";
            return -1;
        })
        .fail([&](const std::exception& e) {
            error += QString{e.what()} + "2";
            return -1;
        })
        .wait();

    QCOMPARE(error, QString{"foo1"});
}

void tst_qpromise_fail::catchAll()
{
    auto p = QtPromise::QPromise<int>::reject(std::out_of_range("foo"));

    QString error;
    p.fail([&](const std::runtime_error& e) {
         error += QString{e.what()} + "0";
         return -1;
     })
        .fail([&]() {
            error += "bar";
            return -1;
        })
        .fail([&](const std::exception& e) {
            error += QString{e.what()} + "2";
            return -1;
        })
        .wait();

    QCOMPARE(error, QString{"bar"});
}

void tst_qpromise_fail::functionPtrHandlers()
{
    { // Global functions.
        auto p0 = QtPromise::QPromise<float>::reject(kErr).fail(&fnNoArg);
        auto p1 = QtPromise::QPromise<float>::reject(kErr).fail(&fnArgByVal);
        auto p2 = QtPromise::QPromise<float>::reject(kErr).fail(&fnArgByRef);

        QCOMPARE(waitForValue(p0, kFail), kRes);
        QCOMPARE(waitForValue(p1, kFail), kRes);
        QCOMPARE(waitForValue(p2, kFail), kRes);
    }
    { // Static member functions.
        auto p0 = QtPromise::QPromise<float>::reject(kErr).fail(&Klass::kFnNoArg);
        auto p1 = QtPromise::QPromise<float>::reject(kErr).fail(&Klass::kFnArgByVal);
        auto p2 = QtPromise::QPromise<float>::reject(kErr).fail(&Klass::kFnArgByRef);

        QCOMPARE(waitForValue(p0, kFail), kRes);
        QCOMPARE(waitForValue(p1, kFail), kRes);
        QCOMPARE(waitForValue(p2, kFail), kRes);
    }
}

// https://github.com/simonbrunel/qtpromise/issues/29
void tst_qpromise_fail::stdFunctionHandlers()
{
    { // lvalue.
        std::function<float()> stdFnNoArg = fnNoArg;
        std::function<float(QString)> stdFnArgByVal = fnArgByVal;
        std::function<float(const QString&)> stdFnArgByRef = fnArgByRef;

        auto p0 = QtPromise::QPromise<float>::reject(kErr).fail(stdFnNoArg);
        auto p1 = QtPromise::QPromise<float>::reject(kErr).fail(stdFnArgByVal);
        auto p2 = QtPromise::QPromise<float>::reject(kErr).fail(stdFnArgByRef);

        QCOMPARE(waitForValue(p0, kFail), kRes);
        QCOMPARE(waitForValue(p1, kFail), kRes);
        QCOMPARE(waitForValue(p2, kFail), kRes);
    }
    { // const lvalue.
        const std::function<float()> stdFnNoArg = fnNoArg;
        const std::function<float(QString)> stdFnArgByVal = fnArgByVal;
        const std::function<float(const QString&)> stdFnArgByRef = fnArgByRef;

        auto p0 = QtPromise::QPromise<float>::reject(kErr).fail(stdFnNoArg);
        auto p1 = QtPromise::QPromise<float>::reject(kErr).fail(stdFnArgByVal);
        auto p2 = QtPromise::QPromise<float>::reject(kErr).fail(stdFnArgByRef);

        QCOMPARE(waitForValue(p0, kFail), kRes);
        QCOMPARE(waitForValue(p1, kFail), kRes);
        QCOMPARE(waitForValue(p2, kFail), kRes);
    }
    { // rvalue.
        auto p0 = QtPromise::QPromise<float>::reject(kErr).fail(std::function<float()>{fnNoArg});
        auto p1 = QtPromise::QPromise<float>::reject(kErr).fail(
            std::function<float(QString)>{fnArgByVal});
        auto p2 = QtPromise::QPromise<float>::reject(kErr).fail(
            std::function<float(const QString&)>{fnArgByRef});

        QCOMPARE(waitForValue(p0, kFail), kRes);
        QCOMPARE(waitForValue(p1, kFail), kRes);
        QCOMPARE(waitForValue(p2, kFail), kRes);
    }
}

//// https://github.com/simonbrunel/qtpromise/issues/29
void tst_qpromise_fail::stdBindHandlers()
{
    using namespace std::placeholders;

    const float val = 42.f;
    const Klass obj{val};

    const std::function<float()> bindNoArg = std::bind(&Klass::fnNoArg, &obj);
    const std::function<float(QString)> bindArgByVal = std::bind(&Klass::fnArgByVal, &obj, _1);
    const std::function<float(const QString&)> bindArgByRef =
        std::bind(&Klass::fnArgByRef, &obj, _1);

    auto p0 = QtPromise::QPromise<float>::reject(kErr).fail(bindNoArg);
    auto p1 = QtPromise::QPromise<float>::reject(kErr).fail(bindArgByVal);
    auto p2 = QtPromise::QPromise<float>::reject(kErr).fail(bindArgByRef);

    QCOMPARE(waitForValue(p0, kFail), val);
    QCOMPARE(waitForValue(p1, kFail), val + kRes);
    QCOMPARE(waitForValue(p2, kFail), val + kRes);
}

void tst_qpromise_fail::lambdaHandlers()
{
    { // lvalue.
        auto lambdaNoArg = []() {
            return kRes;
        };
        auto lambdaArgByVal = [](QString v) {
            return v.toFloat();
        };
        auto lambdaArgByRef = [](const QString& v) {
            return v.toFloat();
        };

        auto p0 = QtPromise::QPromise<float>::reject(kErr).fail(lambdaNoArg);
        auto p1 = QtPromise::QPromise<float>::reject(kErr).fail(lambdaArgByVal);
        auto p2 = QtPromise::QPromise<float>::reject(kErr).fail(lambdaArgByRef);

        QCOMPARE(waitForValue(p0, kFail), kRes);
        QCOMPARE(waitForValue(p1, kFail), kRes);
        QCOMPARE(waitForValue(p2, kFail), kRes);
    }
    { // const lvalue.
        const auto lambdaNoArg = []() {
            return kRes;
        };
        const auto lambdaArgByVal = [](QString v) {
            return v.toFloat();
        };
        const auto lambdaArgByRef = [](const QString& v) {
            return v.toFloat();
        };

        auto p0 = QtPromise::QPromise<float>::reject(kErr).fail(lambdaNoArg);
        auto p1 = QtPromise::QPromise<float>::reject(kErr).fail(lambdaArgByVal);
        auto p2 = QtPromise::QPromise<float>::reject(kErr).fail(lambdaArgByRef);

        QCOMPARE(waitForValue(p0, kFail), kRes);
        QCOMPARE(waitForValue(p1, kFail), kRes);
        QCOMPARE(waitForValue(p2, kFail), kRes);
    }
    { // rvalue.
        auto p0 = QtPromise::QPromise<float>::reject(kErr).fail([]() {
            return kRes;
        });
        auto p1 = QtPromise::QPromise<float>::reject(kErr).fail([](QString v) {
            return v.toFloat();
        });
        auto p2 = QtPromise::QPromise<float>::reject(kErr).fail([](const QString& v) {
            return v.toFloat();
        });

        QCOMPARE(waitForValue(p0, kFail), kRes);
        QCOMPARE(waitForValue(p1, kFail), kRes);
        QCOMPARE(waitForValue(p2, kFail), kRes);
    }
}
