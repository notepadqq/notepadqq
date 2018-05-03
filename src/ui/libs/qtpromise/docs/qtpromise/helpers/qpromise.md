## `qPromise`

```
qPromise(T value) -> QPromise<R>
```

Similar to the [`QPromise<T>::resolve`](../qpromise/resolve.md) static method, creates a promise resolved from a given `value` without the extra typing:

```cpp
auto promise = qPromise();                // QPromise<void>
auto promise = qPromise(42);              // QPromise<int>
auto promise = qPromise(QString("foo"));  // QPromise<QString>
```

This method also allows to convert `QFuture<T>` to `QPromise<T>` delayed until the `QFuture` is finished ([read more](../qtconcurrent.md#convert)).
