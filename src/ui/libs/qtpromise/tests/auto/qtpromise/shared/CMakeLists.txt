project(qtpromise.tests.utils)

add_library(${PROJECT_NAME} STATIC
    data.h
    object.h
    utils.h
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    qtpromise
)
