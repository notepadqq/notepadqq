function(qtpromise_add_test NAME)
    cmake_parse_arguments(_ARG "" "" "SOURCES;LIBRARIES" ${ARGN})

    set(_TARGET qtpromise.tests.auto.${NAME})

    add_executable(${_TARGET} ${_ARG_SOURCES})

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_link_libraries(${_TARGET} gcov)
        target_compile_options(${_TARGET}
            PRIVATE
                -fprofile-arcs
                -ftest-coverage
                -O0
                -g
        )
    endif()

    target_link_libraries(${_TARGET}
        Qt${QT_VERSION_MAJOR}::Concurrent
        Qt${QT_VERSION_MAJOR}::Test
        qtpromise
        qtpromise.tests.utils
        ${_ARG_LIBRARIES}
    )

    add_test(NAME ${_TARGET}
        COMMAND ${_TARGET}
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endfunction()

function(qtpromise_add_tests GROUP)
    cmake_parse_arguments(_ARG "" "" "SOURCES" ${ARGN})

    foreach(_FILE ${_ARG_SOURCES})
        get_filename_component(_FILE_NAME ${_FILE} NAME)
        if (_FILE_NAME MATCHES "^tst_(.+)\.cpp$")
            string(REGEX REPLACE "^tst_(.+)\.cpp$" "\\1" _TEST_NAME ${_FILE_NAME})
            qtpromise_add_test(${GROUP}.${_TEST_NAME} SOURCES ${_FILE} ${_ARG_UNPARSED_ARGUMENTS})
        endif()
    endforeach()
endfunction()

