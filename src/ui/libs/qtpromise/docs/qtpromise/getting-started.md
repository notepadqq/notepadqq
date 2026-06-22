# Getting Started

## Installation

### Using CMake

*Since: 0.6.0*

If your project uses [CMake](https://cmake.org/) as build system, QtPromise can be installed using
the `FetchContent` module. Please refer to the [CMake (FetchContent)](#cmake-fetchcontent) section
for details of using this method.

### Using qpm

If your project is configured to use [qpm](https://www.qpm.io/), QtPromise can be installed using
the following command:

```sh
 qpm install com.github.simonbrunel.qtpromise
```

See also: [com.github.simonbrunel.qtpromise](https://www.qpm.io/packages/com.github.simonbrunel.qtpromise/)

### Using Git

If your project uses [Git](https://git-scm.com/) as version control system, QtPromise can be
installed either as a [`subtree`](#subtree) or a [`submodule`](#submodule). Read more about these
commands in ["Git: submodules vs subtrees"](https://nering.dev/2016/git-submodules-vs-subtrees/)
which provides a good comparison between these two workflows.

The following examples install QtPromise version 0.7.0 under the `3rdparty/qtpromise` subdirectory.
Note that the install directory is arbitrary and can be any empty directory under your repository.
Once installed, refer to the [CMake](#cmake) or [qmake](#qmake) sections for details of integrating
QtPromise into your project.

#### subtree

```sh
cd <your/project/repository>
git remote add qtpromise https://github.com/simonbrunel/qtpromise.git
git subtree add -P 3rdparty/qtpromise qtpromise v0.7.0 --squash -m "Add QtPromise v0.7.0"
```

#### submodule

```sh
cd <your/project/repository>
git submodule add https://github.com/simonbrunel/qtpromise.git 3rdparty/qtpromise
cd 3rdparty/qtpromise
git checkout v0.7.0
cd ../..
git add 3rdparty/qtpromise
git commit -m "Add QtPromise v0.7.0"
```

### Download

QtPromise can be downloaded from the [GitHub release page](https://github.com/simonbrunel/qtpromise/releases)
as a `zip` or `tar.gz` archive. Under Linux, you can use the following commands:

```sh
cd <your/project/repository>
wget -q -O qtpromise.tar.gz https://github.com/simonbrunel/qtpromise/archive/v0.7.0.tar.gz
tar xzf qtpromise.tar.gz --strip 1 --one-top-level=3rdparty/qtpromise
rm qtpromise.tar.gz
```

## Integration

QtPromise is a [header-only](https://en.wikipedia.org/wiki/Header-only) library. Integrating it
within your project consists only in configuring the include path(s) to the library headers. To
simplify this step, [qmake](#qmake) and [CMake](#cmake) integrations are provided.

### CMake

*Since: 0.6.0*

After installing QtPromise using [Git](#using-git) or the [download](#download) method, you can use
the [`add_subdirectory`](https://cmake.org/cmake/help/latest/command/add_subdirectory.html) command
to make its targets available to your CMake project:

```cmake
add_subdirectory(<path/to/qtpromise>)

target_link_libraries(<target> qtpromise)
```

### CMake (FetchContent)

*Since: 0.6.0*

Alternatively, the [`FetchContent`](https://cmake.org/cmake/help/latest/module/FetchContent.html)
module (**CMake 3.11+**) allows to install QtPromise from your CMake project at configure time:

```cmake
include(FetchContent)

FetchContent_Declare(qtpromise
  GIT_REPOSITORY https://github.com/simonbrunel/qtpromise.git
  GIT_TAG v0.7.0
  GIT_SHALLOW true
)

# CMake v3.14+
FetchContent_MakeAvailable(qtpromise)

target_link_libraries(<target> qtpromise)
```

If your CMake version **is prior to v3.14**, you need to explicitly define the population steps:

```cmake
# CMake v3.11+ (alternative for FetchContent_MakeAvailable)
FetchContent_GetProperties(qtpromise)
if(NOT qtpromise_POPULATED)
  FetchContent_Populate(qtpromise)
  add_subdirectory(
    ${qtpromise_SOURCE_DIR}
    ${qtpromise_BINARY_DIR}
  )
endif()
```

### qmake

After installing QtPromise using [Git](#using-git) or the [download](#download) method, you can
[include](https://doc.qt.io/qt-5/qmake-test-function-reference.html#include-filename) `qtpromise.pri`
file from the install directory:

```cmake
include(<path/to/qtpromise>/qtpromise.pri)
```

## Example

To start using QtPromise in your code, you first need to include the single module header:

```cpp
#include <QtPromise>
```

The following `download` function creates a [promise from callbacks](qpromise/constructor.md) which
will be resolved when the network request is finished:

```cpp
QtPromise::QPromise<QByteArray> download(const QUrl& url)
{
    return QtPromise::QPromise<QByteArray>{[&](
        const QtPromise::QPromiseResolve<QByteArray>& resolve,
        const QtPromise::QPromiseReject<QByteArray>& reject) {

        QNetworkReply* reply = manager->get(QNetworkRequest{url});
        QObject::connect(reply, &QNetworkReply::finished, [=]() {
            if (reply->error() == QNetworkReply::NoError) {
                resolve(reply->readAll());
            } else {
                reject(reply->error());
            }

            reply->deleteLater();
        });
    }};
}
```

The following method `uncompress` data in a separate thread and returns a [promise from QFuture](qtconcurrent.md):

```cpp
QtPromise::QPromise<Entries> uncompress(const QByteArray& data)
{
    return QtPromise::resolve(QtConcurrent::run([](const QByteArray& data) {
        Entries entries;

        // {...} uncompress data and parse content.

        if (error) {
            throw MalformedException{};
        }

        return entries;
    }, data));
}
```

It's then easy to chain the whole asynchronous process using promises:

- initiate the promise chain by downloading a specific URL,
- [`then`](qpromise/then.md) *and only if download succeeded*, uncompress received data,
- [`then`](qpromise/then.md) validate and process the uncompressed entries,
- [`finally`](qpromise/finally.md) perform operations whatever the process succeeded or failed,
- and handle specific errors using [`fail`](qpromise/fail.md).

```cpp
download(url).then(&uncompress).then([](const Entries& entries) {
    if (entries.isEmpty()) {
        throw UpdateException{"No entries"};
    }
    // {...} process entries
}).finally([]() {
    // {...} cleanup
}).fail([](QNetworkReply::NetworkError error) {
    // {...} handle network error
}).fail([](const UpdateException& error) {
    // {...} handle update error
}).fail([]() {
    // {...} catch all
});
```

Note that `MalformedException` in the example above is thrown from a QtConcurrent thread and should
meet [specific conditions](qtconcurrent.md#error).
