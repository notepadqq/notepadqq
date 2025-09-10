---
title: all
---

# QtPromise::all

*Since: 0.5.0*

```
QtPromise::all(Sequence<QPromise<T>> promises) -> QPromise<QVector<T>>
QtPromise::all(Sequence<QPromise<void>> promises) -> QPromise<void>
```

Returns a `QPromise<QVector<T>>` (or `QPromise<void>`) that fulfills when **all** `promises` of (the same) type `T` have been fulfilled. The `output` value is a vector containing all the values of `promises`, in the same order, i.e., at the respective positions to the original sequence, regardless of completion order.

If any of the given `promises` fail, `output` immediately rejects with the error of the promise that rejected, whether or not the other promises are resolved.

`Sequence` is any STL compatible container (eg. `QVector`, `QList`, `std::vector`, etc.)

```cpp
QVector<QPromise<QByteArray> > promises{
    download(QUrl("http://a...")),
    download(QUrl("http://b...")),
    download(QUrl("http://c..."))
};

auto output = QtPromise::all(promises);

// output type: QPromise<QVector<QByteArray>>
output.then([](const QVector<QByteArray>& res) {
    // {...}
});
```
