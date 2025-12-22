# Thread-Safety

QPromise is thread-safe and can be copied and accessed across different threads. QPromise relies on [explicitly data sharing](https://doc.qt.io/qt-5/qexplicitlyshareddatapointer.html#details) and thus `auto p2 = p1` represents the same promise: when `p1` resolves, handlers registered on `p1` and `p2` are called, the fulfilled value being shared between both instances.

::: warning IMPORTANT
While it's safe to access the resolved value from different threads using [`then`](qpromise/then.md), QPromise provides no guarantee about the object being pointed to. Thread-safety and reentrancy rules for that object still apply.
:::
