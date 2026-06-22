# Disable deprecation warnings since we are testing deprecated features.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-Wno-deprecated-declarations)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/wd4996)
endif()

qtpromise_add_tests(deprecation
    SOURCES
        tst_helpers_qpromise.cpp
        tst_helpers_qpromiseall.cpp
        tst_qpromise_all.cpp
)
