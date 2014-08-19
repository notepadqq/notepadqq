#-------------------------------------------------
#
# Project created by QtCreator 2014-08-13T23:31:36
#
#-------------------------------------------------

QT       += core gui widgets webkitwidgets

TARGET = notepadqq
TEMPLATE = app

RCC_DIR = ../../out/build_data
UI_DIR = ../../out/build_data
MOC_DIR = ../../out/build_data
OBJECTS_DIR = ../../out/build_data

isEmpty(DESTDIR) {
    CONFIG(debug, debug|release) {
        DESTDIR = ../../out/debug
    }
    CONFIG(release, debug|release) {
        DESTDIR = ../../out/release
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
editortarget.target = editor
editortarget.commands = (cd \"$$PWD\" && \
                         $${QMAKE_MKDIR} \"$$DESTDIR/editor\" && \ # ensure /out/editor exists
                         $${DEL_DIR} \"$$DESTDIR/editor\" && \ # delete /out/editor
                         $${QMAKE_MKDIR} \"$$DESTDIR/editor\" && \ # create a new empty /out/editor
                         $${QMAKE_COPY_DIR} \"../editor\"/* \"$$DESTDIR/editor/\") # TODO remove unnecessary files
QMAKE_EXTRA_TARGETS += editortarget
PRE_TARGETDEPS += editor


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
