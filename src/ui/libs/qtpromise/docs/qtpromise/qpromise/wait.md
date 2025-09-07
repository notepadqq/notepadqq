---
title: .wait
---

# QPromise::wait

*Since: 0.1.0*

```cpp
QPromise<T>::wait() -> QPromise<T>
```

This method holds the execution of the remaining code until the `input` promise is resolved (either fulfilled or rejected), **without** blocking the event loop of the current thread:

```cpp
int result = -1;

QPromise<int> input = QtPromise::resolve(QtConcurrent::run([]() {
    return 42;
})).tap([&](int res) {
    result = res;
});

// input.isPending() is true && result is -1

input.wait();

// input.isPending() is false && result is 42
```
