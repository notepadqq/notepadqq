# https://cmake.org/cmake/help/latest/prop_gbl/CMAKE_CXX_KNOWN_FEATURES.html
# https://gcc.gnu.org/projects/cxx-status.html#cxx14
if ("cxx_generic_lambdas" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    set(CMAKE_CXX_STANDARD 14)
    qtpromise_add_tests(cpp14
        SOURCES
            tst_argsof_lambda_auto.cpp
            tst_resolver_lambda_auto.cpp
    )
endif()
