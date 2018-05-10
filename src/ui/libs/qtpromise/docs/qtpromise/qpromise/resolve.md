## `[static] QPromise<T>::resolve`

```
[static] QPromise<T>::resolve(T value) -> QPromise<T>
```

Creates a `QPromise<T>` that is fulfilled with the given `value` of type `T`:

```cpp
QPromise<int> compute(const QString& type)
{
    if (type == "magic") {
        return QPromise<int>::resolve(42);
    }

    return QPromise<int>([](const QPromiseResolve<int>& resolve) {
        // {...}
    });
}
```

See also: [`qPromise`](../helpers/qpromise.md)
