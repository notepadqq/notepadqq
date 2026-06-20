TEMPLATE = app
CONFIG += testcase warn_on
QT += testlib
QT -= gui

DEFINES += QT_DEPRECATED_WARNINGS

# Additional warnings and make all warnings into errors
# https://github.com/simonbrunel/qtpromise/issues/10
gcc:QMAKE_CXXFLAGS += -Werror -Wold-style-cast
msvc:QMAKE_CXXFLAGS += -WX

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
    $$PWD/shared/data.h \
    $$PWD/shared/object.h \
    $$PWD/shared/utils.h

include(../../../qtpromise.pri)
