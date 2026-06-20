---
title: constructor
---

# QPromise::QPromise

*Since: 0.1.0*

```cpp
QPromise<T>::QPromise(Function resolver)
```

Creates a new promise that will be fulfilled or rejected by the given `resolver` lambda:

```cpp
QPromise<int> promise([](const QPromiseResolve<int>& resolve, const QPromiseReject<int>& reject) {
    async_method([=](bool success, int result) {
        if (success) {
            resolve(result);
        } else {
            reject(customException());
        }
    });
});
```

::: tip NOTE
`QPromise<void>` is specialized to not contain any value, meaning that the `resolve` callback takes no argument.
:::

**C++14**

```cpp
QPromise<int> promise([](const auto& resolve, const auto& reject) {
    // {...}
});
```

**Undefined rejection reason**

*Since: 0.5.0*

While not recommended because it makes tracking errors more difficult, it's also possible to reject a promise without explicit reason, in which case, a built-in [`QPromiseUndefinedException`](../exceptions/undefined.md) is thrown:

```cpp
QPromise<int> promise([](const QPromiseResolve<int>& resolve, const QPromiseReject<int>& reject) {
    async_method([=](bool success, int result) {
        if (success) {
            resolve(result);
        } else {
            reject();
        }
    });
});
```

```cpp
// The exception can be caught explicitly
promise.fail([](const QPromiseUndefinedException&) {
    // { ... }
})

// ... or implicitly (since undefined)
promise.fail([]() {
    // { ... }
})
```
