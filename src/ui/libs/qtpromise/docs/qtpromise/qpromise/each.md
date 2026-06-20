---
title: .each
---

# QPromise::each

*Since: 0.4.0*

```cpp
QPromise<Sequence<T>>::each(Functor functor) -> QPromise<Sequence<T>>

// With:
// - Sequence: STL compatible container
// - Functor: Function(T value, int index) -> any
```

::: warning IMPORTANT
This method only applies to promise with sequence value.
:::

Calls the given `functor` on each element in the promise value (i.e. `Sequence<T>`), then resolves to the original sequence unmodified. If `functor` throws, `output` is rejected with the new exception.

```cpp
QPromise<QList<QByteArray>> input = {...}

auto output = input.each([](const QByteArray& value, int index) {
    // process value ...
});

// output type: QPromise<QList<QByteArray>>
output.then([](const QList<QByteArray>& res) {
    // 'res' contains the original values
});
```

If `functor` returns a promise (or `QFuture`), the `output` promise is delayed until all the promises are resolved. If any of the promises fail, `output` immediately rejects with the error of the promise that rejected, whether or not the other promises are resolved.

```cpp
QPromise<QList<QUrl>> input = {...}

auto output = input.each([](const QUrl& url, ...) {
    return QPromise<void>([&](auto resolve, auto reject) {
        // process url asynchronously ...
    })
});

// `output` resolves as soon as all promises returned by
// `functor` are fulfilled or at least one is rejected.

// output type: QPromise<QList<QUrl>>
output.then([](const QList<QUrl>& res) {
    // 'res' contains the original values
});
```

See also: [`QtPromise::each`](../helpers/each.md)
