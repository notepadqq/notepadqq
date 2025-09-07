include(../qtpromise.pri)

DEFINES -= QT_DEPRECATED_WARNINGS
gcc:QMAKE_CXXFLAGS += -Wno-deprecated-declarations
msvc:QMAKE_CXXFLAGS -= -wd4996
