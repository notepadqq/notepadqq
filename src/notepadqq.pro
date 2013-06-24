#-------------------------------------------------
#
# Project created by QtCreator 2010-11-26T16:20:15
#
#-------------------------------------------------

QT       += core gui \
            network \
            xml

TARGET = notepadqq
TEMPLATE = app

RCC_DIR = ../build/build_data
UI_DIR = ../build/build_data
MOC_DIR = ../build/build_data
OBJECTS_DIR = ../build/build_data

isEmpty(DESTDIR) {
    CONFIG(debug, debug|release) {
        DESTDIR = ../build/debug
    }
    CONFIG(release, debug|release) {
        DESTDIR = ../build/release
    }
}

win32 {
    DEFINES += QSCINTILLA_DLL
    LIBS += User32.lib
}


SOURCES += main.cpp\
        mainwindow.cpp \
    qsciscintillaqq.cpp \
    frmabout.cpp \
    userlexer.cpp \
    qtabwidgetqq.cpp \
    generalfunctions.cpp \
    qtabwidgetscontainer.cpp \
    frmsrchreplace.cpp \
    searchengine.cpp \
    docengine.cpp \
    appwidesettings.cpp \
    lexerfactory.cpp \
    frmpreferences.cpp

HEADERS  += mainwindow.h \
    qsciscintillaqq.h \
    frmabout.h \
    constants.h \
    userlexer.h \
    qtabwidgetqq.h \
    generalfunctions.h \
    qtabwidgetscontainer.h \
    frmsrchreplace.h \
    searchengine.h \
    docengine.h \
    appwidesettings.h \
    lexerfactory.h \
    frmpreferences.h

FORMS    += mainwindow.ui \
    frmabout.ui \
    frmsrchreplace.ui \
    frmpreferences.ui

LIBS += -lqscintilla2

OTHER_FILES += \
    qscintilla/include/Scintilla.iface \
    qscintilla/include/HFacer.py \
    qscintilla/include/Face.py \
    syntax/stylers.xml \
    syntax/langs.xml

RESOURCES += \
    resources.qrc

TRANSLATIONS += L10n/notepadqq_en.ts \
                L10n/notepadqq_it.ts \


unix {
    # MAKE INSTALL
    INSTALLS += target \
        vfiles \
        data

    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }

    target.path = $$INSTALL_ROOT$$PREFIX/bin/
    target.files += $$DESTDIR/$$TARGET
    vfiles.path = $$INSTALL_ROOT$$PREFIX/
    vfiles.files += sys_files/usr/*
    data.path  = $$INSTALL_ROOT$$PREFIX/share/notepadqq
    data.files = syntax/*.xml
}

unix|win32: LIBS += -lmagic

