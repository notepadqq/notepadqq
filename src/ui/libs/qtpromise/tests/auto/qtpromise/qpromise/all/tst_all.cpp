// Tests
#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_all : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void qList();
    //void qVector();
    void stdList();
    void stdVector();
};

QTEST_MAIN(tst_qpromise_all)
#include "tst_all.moc"

namespace {

template <class Sequence>
struct SequenceTester
{
};

template <template <typename, typename...> class Sequence, typename ...Args>
struct SequenceTester<Sequence<QPromise<int>, Args...> >
{
    static void exec()
    {
        Sequence<QPromise<int>, Args...> promises{
            QPromise<int>::resolve(42),
            QPromise<int>::resolve(43),
            QPromise<int>::resolve(44)
        };

        promises.push_back(QPromise<int>::resolve(45));
        promises.insert(++promises.begin(), QPromise<int>::resolve(46));
        promises.pop_back();

        auto p = QPromise<int>::all(promises);

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int> > >::value));
        QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({42, 46, 43, 44}));
    }
};

template <template <typename, typename...> class Sequence, typename ...Args>
struct SequenceTester<Sequence<QPromise<void>, Args...> >
{
    static void exec()
    {
        Sequence<QPromise<void>, Args...> promises{
            QPromise<void>::resolve(),
            QPromise<void>::resolve(),
            QPromise<void>::resolve()
        };

        promises.push_back(QPromise<void>::resolve());
        promises.insert(++promises.begin(), QPromise<void>::resolve());
        promises.pop_back();

        auto p = QPromise<void>::all(promises);

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void> >::value));
        QCOMPARE(waitForValue(p, -1, 42), 42);
    }
};

} // anonymous namespace

void tst_qpromise_all::qList()
{
    SequenceTester<QList<QPromise<int> > >::exec();
    SequenceTester<QList<QPromise<void> > >::exec();
}

// QVector::push_back/append isn't supported since it requires a default
// constructor (see https://github.com/simonbrunel/qtpromise/issues/3)
//void tst_qpromise_all::qVector()
//{
//    SequenceTester<QVector<QPromise<int> > >::exec();
//    SequenceTester<QVector<QPromise<void> > >::exec();
//}

void tst_qpromise_all::stdList()
{
    SequenceTester<std::list<QPromise<int> > >::exec();
    SequenceTester<std::list<QPromise<void> > >::exec();
}

void tst_qpromise_all::stdVector()
{
    SequenceTester<std::vector<QPromise<int> > >::exec();
    SequenceTester<std::vector<QPromise<void> > >::exec();
}
