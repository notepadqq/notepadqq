#-------------------------------------------------
#
# Project created by QtCreator 2014-08-13T23:31:36
#
#-------------------------------------------------

QT       += core gui widgets webkitwidgets

CONFIG += c++11

TARGET = notepadqq
TEMPLATE = app

RCC_DIR = ../../out/build_data
UI_DIR = ../../out/build_data
MOC_DIR = ../../out/build_data
OBJECTS_DIR = ../../out/build_data

# If DEPLOY=true, clear "rpath" so that we can specify Qt lib path via LD_LIBRARY_PATH
equals(DEPLOY, true) {
    QMAKE_RPATH=
}

unix: CMD_FULLDELETE = rm -rf
win32: CMD_FULLDELETE = del /F /S /Q

isEmpty(DESTDIR) {
    CONFIG(debug, debug|release) {
        DESTDIR = ../../out/debug
    }
    CONFIG(release, debug|release) {
        DESTDIR = ../../out/release
    }
}

INSTALLFILESDIR = ../../support_files

win32 {
    LIBS += User32.lib
    #LIBS += -L"$$PWD/3rdparty/libs/magic/" -lmagic
}

unix {
    LIBS += -lmagic
}

SOURCES += main.cpp\
    mainwindow.cpp \
    topeditorcontainer.cpp \
    editortabwidget.cpp \
    editor.cpp \
    docengine.cpp \
    languages.cpp \
    frmabout.cpp \
    frmsearchreplace.cpp \
    frmsearchlanguage.cpp \
    notepadqq.cpp

HEADERS  += include/mainwindow.h \
    include/topeditorcontainer.h \
    include/editortabwidget.h \
    include/editor.h \
    include/docengine.h \
    include/languages.h \
    include/frmabout.h \
    include/frmsearchreplace.h \
    include/frmsearchlanguage.h \
    include/notepadqq.h

FORMS    += mainwindow.ui \
    frmabout.ui \
    frmsearchreplace.ui \
    frmsearchlanguage.ui

RESOURCES += \
    resources.qrc


### EXTRA TARGETS ###

# Copy the editor in the "shared" folder
editorTarget.target = editor
editorTarget.commands = (cd \"$$PWD\" && \
                         $${CMD_FULLDELETE} \"$$DESTDIR/editor\" && \
                         $${QMAKE_COPY_DIR} \"../editor\" \"$$DESTDIR/editor\") # TODO remove unnecessary files

launchTarget.target = launch
launchTarget.commands = (cd \"$$PWD\" && \
                         $${QMAKE_MKDIR} \"$$DESTDIR/bin/\" && \
                         $${QMAKE_COPY} \"$$INSTALLFILESDIR/launch/notepadqq\" \"$$DESTDIR/bin/\")

QMAKE_EXTRA_TARGETS += editorTarget launchTarget
PRE_TARGETDEPS += editor launch

unix {
    # Set launch script permissions execute permission
    permissionsTarget.target = permissions
    permissionsTarget.commands = (chmod +x \"$$INSTALLFILESDIR/launch/notepadqq\")
    QMAKE_EXTRA_TARGETS += permissionsTarget
    PRE_TARGETDEPS += permissions
}

### INSTALL ###
unix {
    isEmpty(PREFIX) {
        PREFIX = /opt/notepadqq
    }

    target.path = $$INSTALL_ROOT$$PREFIX/
    target.files += $$DESTDIR/$$TARGET

    icon_h16.path = $$INSTALL_ROOT$$PREFIX/share/icons/hicolor/16x16/apps/
    icon_h16.files += $$INSTALLFILESDIR/icons/hicolor/16x16/apps/notepadqq.png
    icon_h24.path = $$INSTALL_ROOT$$PREFIX/share/icons/hicolor/24x24/apps/
    icon_h24.files += $$INSTALLFILESDIR/icons/hicolor/24x24/apps/notepadqq.png
    icon_h32.path = $$INSTALL_ROOT$$PREFIX/share/icons/hicolor/32x32/apps/
    icon_h32.files += $$INSTALLFILESDIR/icons/hicolor/32x32/apps/notepadqq.png
    icon_h48.path = $$INSTALL_ROOT$$PREFIX/share/icons/hicolor/48x48/apps/
    icon_h48.files += $$INSTALLFILESDIR/icons/hicolor/48x48/apps/notepadqq.png
    icon_h64.path = $$INSTALL_ROOT$$PREFIX/share/icons/hicolor/64x64/apps/
    icon_h64.files += $$INSTALLFILESDIR/icons/hicolor/64x64/apps/notepadqq.png
    icon_h96.path = $$INSTALL_ROOT$$PREFIX/share/icons/hicolor/96x96/apps/
    icon_h96.files += $$INSTALLFILESDIR/icons/hicolor/96x96/apps/notepadqq.png
    icon_h128.path = $$INSTALL_ROOT$$PREFIX/share/icons/hicolor/128x128/apps/
    icon_h128.files += $$INSTALLFILESDIR/icons/hicolor/128x128/apps/notepadqq.png
    icon_h256.path = $$INSTALL_ROOT$$PREFIX/share/icons/hicolor/256x256/apps/
    icon_h256.files += $$INSTALLFILESDIR/icons/hicolor/256x256/apps/notepadqq.png
    icon_h512.path = $$INSTALL_ROOT$$PREFIX/share/icons/hicolor/512x512/apps/
    icon_h512.files += $$INSTALLFILESDIR/icons/hicolor/512x512/apps/notepadqq.png
    icon_hscalable.path = $$INSTALL_ROOT$$PREFIX/share/icons/hicolor/scalable/apps/
    icon_hscalable.files += $$INSTALLFILESDIR/icons/hicolor/scalable/apps/notepadqq.svg

    shortcuts.path = $$INSTALL_ROOT$$PREFIX/share/applications/
    shortcuts.files += $$INSTALLFILESDIR/shortcuts/notepadqq.desktop

    misc_data.path = $$INSTALL_ROOT$$PREFIX/
    misc_data.files += $$DESTDIR/editor

    # MAKE INSTALL
    INSTALLS += target \
         icon_h16 icon_h24 icon_h32 icon_h48 icon_h64 icon_h96 icon_h128 icon_h256 icon_h512 icon_hscalable \
         shortcuts misc_data \


}
