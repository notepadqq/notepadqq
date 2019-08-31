---
title: QPromiseContextException
---

# QPromiseContextException

*Since: 0.5.0*

When a promise is created using [`QtPromise::connect()`](../helpers/connect.md), this exception is thrown when the `sender` object is destroyed, for example:

```cpp
auto promise = QtPromise::connect(sender, &Object::finished, &Object::error);

promise.fail([](const QPromiseContextException&) {
    // 'sender' has been destroyed.
})
```
