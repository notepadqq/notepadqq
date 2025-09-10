---
title: connect
---

# QtPromise::connect

*Since: 0.5.0*

```cpp
(1) QtPromise::connect(QObject* sender, Signal(T) resolver) -> QPromise<T>
(2) QtPromise::connect(QObject* sender, Signal(T) resolver, Signal(R) rejecter) -> QPromise<T>
(3) QtPromise::connect(QObject* sender, Signal(T) resolver, QObject* sender2, Signal(R) rejecter) -> QPromise<T>
```

Creates a `QPromise<T>` that will be fulfilled with the `resolver` signal's first argument, or a `QPromise<void>` if `resolver` doesn't provide any argument.

The second `(2)` and third `(3)` variants of this method will reject the `output` promise when the `rejecter` signal is emitted. The rejection reason is the value of the `rejecter` signal's first argument or [`QPromiseUndefinedException`](../exceptions/undefined) if `rejected` doesn't provide any argument.

Additionally, the `output` promise will be automatically rejected with [`QPromiseContextException`](../exceptions/context.md) if `sender` is destroyed before the promise is resolved (that doesn't apply to `sender2`).

```cpp
class Sender : public QObject
{
Q_SIGNALS:
    void finished(const QByteArray&);
    void error(ErrorCode);
};

auto sender = new Sender();
auto output = QtPromise::connect(sender, &Sender::finished, &Sender::error);

// 'output' resolves as soon as one of the following events happens:
// - the 'sender' object is destroyed, the promise is rejected
// - the 'finished' signal is emitted, the promise is fulfilled
// - the 'error' signal is emitted, the promise is rejected

// 'output' type: QPromise<QByteArray>
output.then([](const QByteArray& res) {
    // 'res' is the first argument of the 'finished' signal.
}).fail([](ErrorCode err) {
    // 'err' is the first argument of the 'error' signal.
}).fail([](const QPromiseContextException& err) {
    // the 'sender' object has been destroyed before any of
    // the 'finished' or 'error' signals have been emitted.
});
```

See also the [`Qt Signals`](../qtsignals.md) section for more examples.
