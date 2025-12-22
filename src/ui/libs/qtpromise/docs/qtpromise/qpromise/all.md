---
title: ::all [static]
---

# QPromise::all [static]

*Since: 0.1.0*

```
[static] QPromise<T>::all(Sequence<QPromise<T>> promises) -> QPromise<QVector<T>>
```

Returns a `QPromise<QVector<T>>` that fulfills when **all** `promises` of (the same) type `T` have been fulfilled. The `output` value is a vector containing all the values of `promises`, in the same order, i.e., at the respective positions to the original sequence, regardless of completion order.

If any of the given `promises` fail, `output` immediately rejects with the error of the promise that rejected, whether or not the other promises are resolved.

`Sequence` is any STL compatible container (eg. `QVector`, `QList`, `std::vector`, etc.)

```cpp
QVector<QPromise<QByteArray> > promises{
    download(QUrl("http://a...")),
    download(QUrl("http://b...")),
    download(QUrl("http://c..."))
};

auto output = QPromise<QByteArray>::all(promises);

// output type: QPromise<QVector<QByteArray>>
output.then([](const QVector<QByteArray>& res) {
    // {...}
});
```

See also: [`QtPromise::all`](../helpers/all.md)
