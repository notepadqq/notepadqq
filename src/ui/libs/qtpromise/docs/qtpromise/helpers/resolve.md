---
title: resolve
---

# QtPromise::resolve

*Since: 0.5.0*

```
QtPromise::resolve(T value) -> QPromise<R>
```

Similar to the [`QPromise<T>::resolve`](../qpromise/resolve.md) static method, creates a promise resolved from a given `value` but without the extra typing:

```cpp
auto promise = QtPromise::resolve();                // QPromise<void>
auto promise = QtPromise::resolve(42);              // QPromise<int>
auto promise = QtPromise::resolve(QString("foo"));  // QPromise<QString>
```

This method also allows to convert `QFuture<T>` to `QPromise<T>`, delayed until the `QFuture` is finished ([read more](../qtconcurrent.md#convert)).
