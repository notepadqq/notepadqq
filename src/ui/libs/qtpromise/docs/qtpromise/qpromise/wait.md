## `QPromise<T>::wait`

```
QPromise<T>::wait() -> QPromise<T>
```

This method holds the execution of the remaining code until the `input` promise is resolved (either fulfilled or rejected), **without** blocking the event loop of the current thread:

```cpp
int result = -1;

QPromise<int> input = qPromise(QtConcurrent::run([]() {
    return 42;
})).tap([&](int res) {
    result = res;
});

// input.isPending() is true && result is -1

input.wait();

// input.isPending() is false && result is 42
```
