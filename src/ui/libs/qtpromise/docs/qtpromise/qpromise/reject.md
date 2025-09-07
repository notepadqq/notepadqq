---
title: ::reject [static]
---

# QPromise::reject [static]

*Since: 0.1.0*

```cpp
[static] QPromise<T>::reject(any reason) -> QPromise<T>
```

Creates a `QPromise<T>` that is rejected with the given `reason` of *whatever type*:

```cpp
QPromise<int> compute(const QString& type)
{
    if (type == "foobar") {
        return QPromise<int>::reject(QString("Unknown type: %1").arg(type));
    }

    return QPromise<int>([](const QPromiseResolve<int>& resolve) {
        // {...}
    });
}
```
