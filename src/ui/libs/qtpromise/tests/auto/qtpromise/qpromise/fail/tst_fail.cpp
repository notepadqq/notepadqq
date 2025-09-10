#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_fail : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void sameType();
    void baseClass();
    void catchAll();
    // TODO: sync / async
};

QTEST_MAIN(tst_qpromise_fail)
#include "tst_fail.moc"

void tst_qpromise_fail::sameType()
{
    // http://en.cppreference.com/w/cpp/error/exception
    auto p = QPromise<int>::reject(std::out_of_range("foo"));

    QString error;
    p.fail([&](const std::domain_error& e) {
        error += QString(e.what()) + "0";
        return -1;
    }).fail([&](const std::out_of_range& e) {
        error += QString(e.what()) + "1";
        return -1;
    }).fail([&](const std::exception& e) {
        error += QString(e.what()) + "2";
        return -1;
    }).wait();

    QCOMPARE(error, QString("foo1"));
}

void tst_qpromise_fail::baseClass()
{
    // http://en.cppreference.com/w/cpp/error/exception
    auto p = QPromise<int>::reject(std::out_of_range("foo"));

    QString error;
    p.fail([&](const std::runtime_error& e) {
        error += QString(e.what()) + "0";
        return -1;
    }).fail([&](const std::logic_error& e) {
        error += QString(e.what()) + "1";
        return -1;
    }).fail([&](const std::exception& e) {
        error += QString(e.what()) + "2";
        return -1;
    }).wait();

    QCOMPARE(error, QString("foo1"));
}

void tst_qpromise_fail::catchAll()
{
    auto p = QPromise<int>::reject(std::out_of_range("foo"));

    QString error;
    p.fail([&](const std::runtime_error& e) {
        error += QString(e.what()) + "0";
        return -1;
    }).fail([&]() {
        error += "bar";
        return -1;
    }).fail([&](const std::exception& e) {
        error += QString(e.what()) + "2";
        return -1;
    }).wait();

    QCOMPARE(error, QString("bar"));
}
