#-------------------------------------------------
#
# Project created by QtCreator 2014-08-13T23:31:36
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets webkitwidgets

TARGET = notepadqq
TEMPLATE = app

RCC_DIR = ../../build/build_data
UI_DIR = ../../build/build_data
MOC_DIR = ../../build/build_data
OBJECTS_DIR = ../../build/build_data

isEmpty(DESTDIR) {
    CONFIG(debug, debug|release) {
        DESTDIR = ../../build/debug
    }
    CONFIG(release, debug|release) {
        DESTDIR = ../../build/release
    }
}

win32 {
    LIBS += User32.lib
}

unix|win32: LIBS += -lmagic

SOURCES += main.cpp\
    mainwindow.cpp \
    topeditorcontainer.cpp \
    editortabwidget.cpp \
    editor.cpp \
    docengine.cpp \
    languages.cpp \
    frmabout.cpp

HEADERS  += include/mainwindow.h \
    include/topeditorcontainer.h \
    include/editortabwidget.h \
    include/editor.h \
    include/docengine.h \
    include/languages.h \
    include/frmabout.h \
    include/constants.h

FORMS    += mainwindow.ui \
    frmabout.ui

RESOURCES += \
    resources.qrc


### EXTRA TARGETS ###
# FIXME Run even if nothing changed in the c++ code
QMAKE_POST_LINK += (cd \"$$PWD\" && \
                    $${QMAKE_MKDIR} \"$$DESTDIR/editor\" && \
                    $${QMAKE_COPY_DIR} \"../editor\"/* \"$$DESTDIR/editor/\") # TODO remove unnecessary files

QMAKE_DISTCLEAN += -r "$$DESTDIR/editor"


### INSTALL ###
unix {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }

    #target.path = $$INSTALL_ROOT$$PREFIX/bin/
    #target.files += $$DESTDIR/$$TARGET
    #share.path = $$INSTALL_ROOT$$PREFIX/share
    #share.files += sys_files/*
    #editor.path = $$INSTALL_ROOT$$PREFIX/share/notepadqq/editor
    #editor.files += sys_files/*
    #data.path  = $$INSTALL_ROOT$$PREFIX/share/notepadqq
    #data.files = syntax/*.xml

    # MAKE INSTALL
    #INSTALLS += target \
    #    share \
    #    data
}
