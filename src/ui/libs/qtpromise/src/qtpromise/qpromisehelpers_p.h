#ifndef QTPROMISE_QPROMISEHELPERS_P_H
#define QTPROMISE_QPROMISEHELPERS_P_H

#include "qpromiseconnections.h"
#include "qpromiseexceptions.h"

namespace QtPromisePrivate {

// TODO: Suppress QPrivateSignal trailing private signal args
// TODO: Support deducing tuple from args (might require MSVC2017)

template <typename Signal>
using PromiseFromSignal = typename QtPromise::QPromise<Unqualified<typename ArgsOf<Signal>::first>>;

// Connect signal() to QPromiseResolve
template <typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count == 0)>::type
connectSignalToResolver(
    const QtPromise::QPromiseConnections& connections,
    const QtPromise::QPromiseResolve<void>& resolve,
    const Sender* sender,
    Signal signal)
{
    connections << QObject::connect(sender, signal, [=]() {
        connections.disconnect();
        resolve();
    });
}

// Connect signal() to QPromiseReject
template <typename T, typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count == 0)>::type
connectSignalToResolver(
    const QtPromise::QPromiseConnections& connections,
    const QtPromise::QPromiseReject<T>& reject,
    const Sender* sender,
    Signal signal)
{
    connections << QObject::connect(sender, signal, [=]() {
        connections.disconnect();
        reject(QtPromise::QPromiseUndefinedException());
    });
}

// Connect signal(args...) to QPromiseResolve
template <typename T, typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count >= 1)>::type
connectSignalToResolver(
    const QtPromise::QPromiseConnections& connections,
    const QtPromise::QPromiseResolve<T>& resolve,
    const Sender* sender,
    Signal signal)
{
    connections << QObject::connect(sender, signal, [=](const T& value) {
        connections.disconnect();
        resolve(value);
    });
}

// Connect signal(args...) to QPromiseReject
template <typename T, typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count >= 1)>::type
connectSignalToResolver(
    const QtPromise::QPromiseConnections& connections,
    const QtPromise::QPromiseReject<T>& reject,
    const Sender* sender,
    Signal signal)
{
    using V = Unqualified<typename ArgsOf<Signal>::first>;
    connections << QObject::connect(sender, signal, [=](const V& value) {
        connections.disconnect();
        reject(value);
    });
}

// Connect QObject::destroyed signal to QPromiseReject
template <typename T, typename Sender>
void connectDestroyedToReject(
    const QtPromise::QPromiseConnections& connections,
    const QtPromise::QPromiseReject<T>& reject,
    const Sender* sender)
{
    connections << QObject::connect(sender, &QObject::destroyed, [=]() {
        connections.disconnect();
        reject(QtPromise::QPromiseContextException());
    });
}

} // namespace QtPromisePrivate

#endif // QTPROMISE_QPROMISEHELPERS_P_H
