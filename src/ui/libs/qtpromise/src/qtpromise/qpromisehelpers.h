#ifndef QTPROMISE_QPROMISEHELPERS_H
#define QTPROMISE_QPROMISEHELPERS_H

// QtPromise
#include "qpromise_p.h"

namespace QtPromise {

template <typename T>
static inline typename QtPromisePrivate::PromiseDeduce<T>::Type qPromise(T&& value)
{
    using namespace QtPromisePrivate;
    using Promise = typename PromiseDeduce<T>::Type;
    return Promise([&](
        const QPromiseResolve<typename Promise::Type>& resolve,
        const QPromiseReject<typename Promise::Type>& reject) {
        PromiseFulfill<T>::call(std::forward<T>(value), resolve, reject);
    });
}

static inline QPromise<void> qPromise()
{
    return QPromise<void>([](
        const QPromiseResolve<void>& resolve) {
        resolve();
    });
}

template <typename T, template <typename, typename...> class Sequence = QVector, typename ...Args>
static inline QPromise<QVector<T> > qPromiseAll(const Sequence<QPromise<T>, Args...>& promises)
{
    return QPromise<T>::all(promises);
}

template <template <typename, typename...> class Sequence = QVector, typename ...Args>
static inline QPromise<void> qPromiseAll(const Sequence<QPromise<void>, Args...>& promises)
{
    return QPromise<void>::all(promises);
}

} // namespace QtPromise

#endif // QTPROMISE_QPROMISEHELPERS_H
