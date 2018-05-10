TEMPLATE = app
CONFIG += testcase
QT += testlib
QT -= gui

DEFINES += QT_DEPRECATED_WARNINGS

coverage {
    gcc {
        message("Code coverage enabled (gcov)")
        QMAKE_CXXFLAGS += --coverage -O0 -g
        QMAKE_LFLAGS += --coverage -O0 -g
    } else {
        message("Code coverage only available when compiling with GCC")
    }
}

HEADERS += \
    $$PWD/shared/utils.h

include(../../../qtpromise.pri)
