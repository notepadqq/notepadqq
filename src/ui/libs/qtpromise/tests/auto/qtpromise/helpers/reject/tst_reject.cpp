#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

// STL
#include <memory>

using namespace QtPromise;

class tst_helpers_reject : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void rejectWithValue();
    void rejectWithQSharedPtr();
    void rejectWithStdSharedPtr();
};

QTEST_MAIN(tst_helpers_reject)
#include "tst_reject.moc"

void tst_helpers_reject::rejectWithValue()
{
    auto p = QPromise<int>::reject(42);

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(waitForError(p, -1), 42);
}

// https://github.com/simonbrunel/qtpromise/issues/6
void tst_helpers_reject::rejectWithQSharedPtr()
{
    QWeakPointer<int> wptr;

    {
        QSharedPointer<int> sptr(new int(42));
        auto p = QPromise<int>::reject(sptr);

        QCOMPARE(waitForError(p, QSharedPointer<int>()), sptr);

        wptr = sptr;
        sptr.reset();

        QCOMPARE(wptr.isNull(), false); // "p" still holds a reference
    }

    QCOMPARE(wptr.isNull(), true);
}

// https://github.com/simonbrunel/qtpromise/issues/6
void tst_helpers_reject::rejectWithStdSharedPtr()
{
    std::weak_ptr<int> wptr;

    {
        std::shared_ptr<int> sptr(new int(42));
        auto p = QPromise<int>::reject(sptr);

        QCOMPARE(waitForError(p, std::shared_ptr<int>()), sptr);

        wptr = sptr;
        sptr.reset();

        QCOMPARE(wptr.use_count(), 1l); // "p" still holds a reference
    }

    QCOMPARE(wptr.use_count(), 0l);
}
