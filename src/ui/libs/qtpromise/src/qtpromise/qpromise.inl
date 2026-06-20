#include "qpromise.h"
#include "qpromisehelpers.h"

// Qt
#include <QCoreApplication>
#include <QSharedPointer>
#include <QTimer>

namespace QtPromise {

template <typename T>
template <typename F, typename std::enable_if<QtPromisePrivate::ArgsOf<F>::count == 1, int>::type>
inline QPromiseBase<T>::QPromiseBase(F callback)
    : m_d(new QtPromisePrivate::PromiseData<T>())
{
    QtPromisePrivate::PromiseResolver<T> resolver(*this);

    try {
        callback(QPromiseResolve<T>(resolver));
    } catch (...) {
        resolver.reject(std::current_exception());
    }
}

template <typename T>
template <typename F, typename std::enable_if<QtPromisePrivate::ArgsOf<F>::count != 1, int>::type>
inline QPromiseBase<T>::QPromiseBase(F callback)
    : m_d(new QtPromisePrivate::PromiseData<T>())
{
    QtPromisePrivate::PromiseResolver<T> resolver(*this);

    try {
        callback(QPromiseResolve<T>(resolver), QPromiseReject<T>(resolver));
    } catch (...) {
        resolver.reject(std::current_exception());
    }
}

template <typename T>
template <typename TFulfilled, typename TRejected>
inline typename QtPromisePrivate::PromiseHandler<T, TFulfilled>::Promise
QPromiseBase<T>::then(const TFulfilled& fulfilled, const TRejected& rejected) const
{
    using namespace QtPromisePrivate;
    using PromiseType = typename PromiseHandler<T, TFulfilled>::Promise;

    PromiseType next([&](
        const QPromiseResolve<typename PromiseType::Type>& resolve,
        const QPromiseReject<typename PromiseType::Type>& reject) {
        m_d->addHandler(PromiseHandler<T, TFulfilled>::create(fulfilled, resolve, reject));
        m_d->addCatcher(PromiseCatcher<T, TRejected>::create(rejected, resolve, reject));
    });

    if (!m_d->isPending()) {
        m_d->dispatch();
    }

    return next;
}

template <typename T>
template <typename TFulfilled>
inline typename QtPromisePrivate::PromiseHandler<T, TFulfilled>::Promise
QPromiseBase<T>::then(TFulfilled&& fulfilled) const
{
    return then(std::forward<TFulfilled>(fulfilled), nullptr);
}

template <typename T>
template <typename TRejected>
inline typename QtPromisePrivate::PromiseHandler<T, std::nullptr_t>::Promise
QPromiseBase<T>::fail(TRejected&& rejected) const
{
    return then(nullptr, std::forward<TRejected>(rejected));
}

template <typename T>
template <typename THandler>
inline QPromise<T> QPromiseBase<T>::finally(THandler handler) const
{
    QPromise<T> p = *this;
    return p.then(handler, handler).then([=]() {
        return p;
    });
}

template <typename T>
template <typename THandler>
inline QPromise<T> QPromiseBase<T>::tap(THandler handler) const
{
    QPromise<T> p = *this;
    return p.then(handler).then([=]() {
        return p;
    });
}

template <typename T>
template <typename THandler>
inline QPromise<T> QPromiseBase<T>::tapFail(THandler handler) const
{
    QPromise<T> p = *this;
    return p.then([](){}, handler).then([=]() {
        return p;
    });
}

template <typename T>
template <typename E>
inline QPromise<T> QPromiseBase<T>::timeout(int msec, E&& error) const
{
    QPromise<T> p = *this;
    return QPromise<T>([&](
        const QPromiseResolve<T>& resolve,
        const QPromiseReject<T>& reject) {

        QTimer::singleShot(msec, [=]() {
            // we don't need to verify the current promise state, reject()
            // takes care of checking if the promise is already resolved,
            // and thus will ignore this rejection.
            reject(std::move(error));
        });

        QtPromisePrivate::PromiseFulfill<QPromise<T>>::call(p, resolve, reject);
    });
}

template <typename T>
inline QPromise<T> QPromiseBase<T>::delay(int msec) const
{
    return tap([=]() {
        return QPromise<void>([&](const QPromiseResolve<void>& resolve) {
            QTimer::singleShot(msec, resolve);
        });
    });
}

template <typename T>
inline QPromise<T> QPromiseBase<T>::wait() const
{
    // @TODO wait timeout + global timeout
    while (m_d->isPending()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }

    return *this;
}

template <typename T>
template <typename E>
inline QPromise<T> QPromiseBase<T>::reject(E&& error)
{
    return QPromise<T>([&](const QPromiseResolve<T>&, const QPromiseReject<T>& reject) {
        reject(std::forward<E>(error));
    });
}

template <typename T>
template <typename Functor>
inline QPromise<T> QPromise<T>::each(Functor fn)
{
    return this->tap([=](const T& values) {
        int i = 0;

        std::vector<QPromise<void>> promises;
        for (const auto& v : values) {
            promises.push_back(
                QtPromise::attempt(fn, v, i)
                    .then([]() {
                        // Cast to void in case fn returns a non promise value.
                        // TODO remove when implicit cast is implemented.
                    }));

            i++;
        }

        return QtPromise::all(promises);
    });
}

template <typename T>
template <typename Functor>
inline QPromise<T> QPromise<T>::filter(Functor fn)
{
    return this->then([=](const T& values) {
        return QtPromise::filter(values, fn);
    });
}

template <typename T>
template <typename Functor>
inline typename QtPromisePrivate::PromiseMapper<T, Functor>::PromiseType
QPromise<T>::map(Functor fn)
{
    return this->then([=](const T& values) {
        return QtPromise::map(values, fn);
    });
}

template <typename T>
template <typename Functor, typename Input>
inline typename QtPromisePrivate::PromiseDeduce<Input>::Type
QPromise<T>::reduce(Functor fn, Input initial)
{
    return this->then([=](const T& values) {
        return QtPromise::reduce(values, fn, initial);
    });
}

template <typename T>
template <typename Functor, typename U>
inline typename QtPromisePrivate::PromiseDeduce<typename U::value_type>::Type
QPromise<T>::reduce(Functor fn)
{
    return this->then([=](const T& values) {
        return QtPromise::reduce(values, fn);
    });
}

template <typename T>
template <template <typename, typename...> class Sequence, typename ...Args>
inline QPromise<QVector<T>> QPromise<T>::all(const Sequence<QPromise<T>, Args...>& promises)
{
    return QtPromise::all(promises);
}

template <typename T>
inline QPromise<T> QPromise<T>::resolve(const T& value)
{
    return QPromise<T>([&](const QPromiseResolve<T>& resolve) {
       resolve(value);
    });
}

template <typename T>
inline QPromise<T> QPromise<T>::resolve(T&& value)
{
    return QPromise<T>([&](const QPromiseResolve<T>& resolve) {
       resolve(std::forward<T>(value));
    });
}

template <template <typename, typename...> class Sequence, typename ...Args>
inline QPromise<void> QPromise<void>::all(const Sequence<QPromise<void>, Args...>& promises)
{
    return QtPromise::all(promises);
}

inline QPromise<void> QPromise<void>::resolve()
{
    return QtPromise::resolve();
}

} // namespace QtPromise
