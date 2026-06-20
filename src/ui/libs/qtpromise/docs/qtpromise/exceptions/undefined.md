---
title: QPromiseUndefinedException
---

# QPromiseUndefinedException

*Since: 0.5.0*

This exception is thrown when rejecting a promise with no explicit reason, for example:

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

promise.fail([](const QPromiseUndefinedException&) {
    // promise rejected without reason!
})
```
