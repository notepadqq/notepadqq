find_package(Qt${QT_VERSION_MAJOR} REQUIRED
    COMPONENTS
        Concurrent
        Test
)

include(QtPromiseAddTest)

add_subdirectory(auto)
