---
title: each
---

# QtPromise::each

*Since: 0.4.0*

```cpp
QtPromise::each(Sequence<T> values, Functor functor) -> QPromise<Sequence<T>>

// With:
// - Sequence: STL compatible container (e.g. QVector, etc.)
// - Functor: Function(T value, int index) -> void | QPromise<void>
```

Calls the given `functor` on each element in `values` then resolves to the original sequence unmodified. If `functor` throws, `output` is rejected with the new exception.

If `functor` returns a promise (or `QFuture`), the `output` promise is delayed until all the promises are resolved. If any of the promises fail, `output` immediately rejects with the error of the promise that rejected, whether or not the other promises are resolved.

```cpp
auto output = QtPromise::each(QVector<QUrl>{
    QUrl("http://a..."),
    QUrl("http://b..."),
    QUrl("http://c...")
}, [](const QUrl& url, ...) {
    return QPromise<void>([&](auto resolve, auto reject) {
        // process url asynchronously ...
    })
});

// `output` resolves as soon as all promises returned by
// `functor` are fulfilled or at least one is rejected.

// output type: QPromise<QVector<QUrl>>
output.then([](const QVector<QUrl>& res) {
    // 'res' contains the original values
});
```

See also: [`QPromise<T>::each`](../qpromise/each.md)
