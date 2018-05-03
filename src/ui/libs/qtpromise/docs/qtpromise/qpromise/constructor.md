## `QPromise<T>::QPromise`

```
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

> **Note:** `QPromise<void>` is specialized to not contain any value, meaning that the `resolve` callback takes no argument.

**C++14**

```cpp
QPromise<int> promise([](const auto& resolve, const auto& reject) {
    // {...}
});
```
