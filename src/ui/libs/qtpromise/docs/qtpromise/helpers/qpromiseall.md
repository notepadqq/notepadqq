## `qPromiseAll`

```
qPromiseAll(Sequence<QPromise<T>> promises) -> QPromise<QVector<T>>
qPromiseAll(Sequence<QPromise<void>> promises) -> QPromise<void>
```

This method simply calls the appropriated [`QPromise<T>::all`](../qpromise/all.md) static method based on the given `QVector` type. In some cases, this method is more convenient than the static one since it avoid some extra typing:

```cpp
QVector<QPromise<QByteArray> > promises{...}

auto output = qPromiseAll(promises);
// eq. QPromise<QByteArray>::all(promises)
```

