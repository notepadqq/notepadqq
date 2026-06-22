# QPromiseConversionException

*Since: 0.7.0*

This exception is thrown whenever a promise result conversion fails, for example: 

```cpp
QPromise<QVariant> input = {...};
auto output = input.convert<int>()
    .fail([](const QPromiseconversionException& e) {
        // conversion may fail because input could not be converted to number
    });
```
