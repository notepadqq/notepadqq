# Qt Signals

QtPromise supports creating promises that are resolved or rejected by regular [Qt signals](https://doc.qt.io/qt-5/signalsandslots.html).

::: warning IMPORTANT
A promise connected to a signal will be resolved (fulfilled or rejected) **only one time**, no matter if the signals are emitted multiple times. Internally, the promise is disconnected from all signals as soon as one signal is emitted.
:::

## Resolve Signal

The [`QtPromise::connect()`](helpers/connect.md) helper allows to create a promise resolved from a single signal:

```cpp
// [signal] Object::finished(const QByteArray&)
auto output = QtPromise::connect(obj, &Object::finished);

// output type: QPromise<QByteArray>
output.then([](const QByteArray& data) {
    // {...}
});
```

If the signal doesn't provide any argument, a `QPromise<void>` is returned:

```cpp
// [signal] Object::done()
auto output = QtPromise::connect(obj, &Object::done);

// output type: QPromise<void>
output.then([]() {
    // {...}
});
```

::: tip NOTE
QtPromise currently only supports single argument signals, which means that only the first argument is used to fulfill or reject the connected promise, other arguments being ignored.
:::

## Reject Signal

The [`QtPromise::connect()`](helpers/connect.md) helper also allows to reject the promise from another signal:

```cpp
// [signal] Object::finished(const QByteArray& data)
// [signal] Object::error(ObjectError error)
auto output = QtPromise::connect(obj, &Object::finished, &Object::error);

// output type: QPromise<QByteArray>
output.then([](const QByteArray& data) {
    // {...}
}).fail(const ObjectError& error) {
    // {...}
});
```

If the rejection signal doesn't provide any argument, the promise will be rejected
with [`QPromiseUndefinedException`](../exceptions/undefined), for example:

```cpp
// [signal] Object::finished()
// [signal] Object::error()
auto output = QtPromise::connect(obj, &Object::finished, &Object::error);

// output type: QPromise<QByteArray>
output.then([]() {
    // {...}
}).fail(const QPromiseUndefinedException& error) {
    // {...}
});
```

A third variant allows to connect the resolve and reject signals from different objects:

```cpp
// [signal] ObjectA::finished(const QByteArray& data)
// [signal] ObjectB::error(ObjectBError error)
auto output = QtPromise::connect(objA, &ObjectA::finished, objB, &ObjectB::error);

// output type: QPromise<QByteArray>
output.then([](const QByteArray& data) {
    // {...}
}).fail(const ObjectBError& error) {
    // {...}
});
```

Additionally to the rejection signal, promises created using [`QtPromise::connect()`](helpers/connect.md) are automatically rejected with [`QPromiseContextException`](exceptions/context.md) if the sender is destroyed before fulfilling the promise.

See [`QtPromise::connect()`](helpers/connect.md) for more details.
