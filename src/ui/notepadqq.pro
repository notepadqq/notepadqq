#-------------------------------------------------
#
# Project created by QtCreator 2014-08-13T23:31:36
#
#-------------------------------------------------

QT       += core gui widgets webkitwidgets

CONFIG += c++11

TARGET = notepadqq-bin
TEMPLATE = app

RCC_DIR = ../../out/build_data
UI_DIR = ../../out/build_data
MOC_DIR = ../../out/build_data
OBJECTS_DIR = ../../out/build_data

# clear "rpath" so that we can override Qt lib path via LD_LIBRARY_PATH
QMAKE_RPATH=

# Avoid automatic casts from QString to QUrl
DEFINES += QT_NO_URL_CAST_FROM_STRING

unix: CMD_FULLDELETE = rm -rf
win32: CMD_FULLDELETE = del /F /S /Q

isEmpty(DESTDIR) {
    CONFIG(debug, debug|release) {
        DESTDIR = ../../out/debug/bin
    }
    CONFIG(release, debug|release) {
        DESTDIR = ../../out/release/bin
    }
}

APPDATADIR = "$$DESTDIR/../appdata"

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
    docengine.cpp \
    frmabout.cpp \
    frmsearchreplace.cpp \
    notepadqq.cpp \
    frmpreferences.cpp \
    customqwebview.cpp \
    iconprovider.cpp \
    EditorNS/editor.cpp \
    EditorNS/bannerfilechanged.cpp \
    EditorNS/bannerbasicmessage.cpp \
    EditorNS/bannerfileremoved.cpp

HEADERS  += include/mainwindow.h \
    include/topeditorcontainer.h \
    include/editortabwidget.h \
    include/docengine.h \
    include/frmabout.h \
    include/frmsearchreplace.h \
    include/notepadqq.h \
    include/frmpreferences.h \
    include/customqwebview.h \
    include/iconprovider.h \
    include/EditorNS/editor.h \
    include/EditorNS/bannerfilechanged.h \
    include/EditorNS/bannerbasicmessage.h \
    include/EditorNS/bannerfileremoved.h

FORMS    += mainwindow.ui \
    frmabout.ui \
    frmsearchreplace.ui \
    frmpreferences.ui

RESOURCES += \
    resources.qrc


### EXTRA TARGETS ###

# Copy the editor in the "shared" folder
editorTarget.target = make_editor
editorTarget.commands = (cd \"$$PWD\" && \
                         $${CMD_FULLDELETE} \"$$APPDATADIR/editor\" && \
                         $${QMAKE_MKDIR} \"$$APPDATADIR/\" && \
                         $${QMAKE_COPY_DIR} \"../editor\" \"$$APPDATADIR/editor\") # TODO remove unnecessary files

launchTarget.target = make_launch
launchTarget.commands = (cd \"$$PWD\" && \
                         $${QMAKE_MKDIR} \"$$DESTDIR/\" && \
                         $${QMAKE_COPY} \"$$INSTALLFILESDIR/launch/notepadqq\" \"$$DESTDIR/\")

QMAKE_EXTRA_TARGETS += editorTarget launchTarget
PRE_TARGETDEPS += make_editor make_launch

unix {
    # Set launch script permissions
    permissionsTarget.target = permissions
    permissionsTarget.commands = (chmod 755 \"$$INSTALLFILESDIR/launch/notepadqq\")
    QMAKE_EXTRA_TARGETS += permissionsTarget
    PRE_TARGETDEPS += permissions
}

### INSTALL ###
unix {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }

    target.path = "$$INSTALL_ROOT$$PREFIX/bin/"
    target.files += "$$DESTDIR/$$TARGET"

    icon_h16.path = "$$INSTALL_ROOT$$PREFIX/share/icons/hicolor/16x16/apps/"
    icon_h16.files += "$$INSTALLFILESDIR/icons/hicolor/16x16/apps/notepadqq.png"
    icon_h24.path = "$$INSTALL_ROOT$$PREFIX/share/icons/hicolor/24x24/apps/"
    icon_h24.files += "$$INSTALLFILESDIR/icons/hicolor/24x24/apps/notepadqq.png"
    icon_h32.path = "$$INSTALL_ROOT$$PREFIX/share/icons/hicolor/32x32/apps/"
    icon_h32.files += "$$INSTALLFILESDIR/icons/hicolor/32x32/apps/notepadqq.png"
    icon_h48.path = "$$INSTALL_ROOT$$PREFIX/share/icons/hicolor/48x48/apps/"
    icon_h48.files += "$$INSTALLFILESDIR/icons/hicolor/48x48/apps/notepadqq.png"
    icon_h64.path = "$$INSTALL_ROOT$$PREFIX/share/icons/hicolor/64x64/apps/"
    icon_h64.files += "$$INSTALLFILESDIR/icons/hicolor/64x64/apps/notepadqq.png"
    icon_h96.path = "$$INSTALL_ROOT$$PREFIX/share/icons/hicolor/96x96/apps/"
    icon_h96.files += "$$INSTALLFILESDIR/icons/hicolor/96x96/apps/notepadqq.png"
    icon_h128.path = "$$INSTALL_ROOT$$PREFIX/share/icons/hicolor/128x128/apps/"
    icon_h128.files += "$$INSTALLFILESDIR/icons/hicolor/128x128/apps/notepadqq.png"
    icon_h256.path = "$$INSTALL_ROOT$$PREFIX/share/icons/hicolor/256x256/apps/"
    icon_h256.files += "$$INSTALLFILESDIR/icons/hicolor/256x256/apps/notepadqq.png"
    icon_h512.path = "$$INSTALL_ROOT$$PREFIX/share/icons/hicolor/512x512/apps/"
    icon_h512.files += "$$INSTALLFILESDIR/icons/hicolor/512x512/apps/notepadqq.png"
    icon_hscalable.path = "$$INSTALL_ROOT$$PREFIX/share/icons/hicolor/scalable/apps/"
    icon_hscalable.files += "$$INSTALLFILESDIR/icons/hicolor/scalable/apps/notepadqq.svg"

    system($${QMAKE_MKDIR} \"$$APPDATADIR/editor\")    # Make sure that the folder exists, otherwise qmake won't create the install rule
    misc_data.path = "$$INSTALL_ROOT$$PREFIX/share/notepadqq/"
    misc_data.files += "$$APPDATADIR/editor"

    launch.path = "$$INSTALL_ROOT$$PREFIX/bin/"
    launch.files += "$$DESTDIR/notepadqq"
    launch.CONFIG = no_check_exist     # Create the install rule even if the file doesn't exists when qmake is run

    shortcuts.path = "$$INSTALL_ROOT$$PREFIX/share/applications/"
    shortcuts.files += "$$INSTALLFILESDIR/shortcuts/notepadqq.desktop"

    # Dummy target used to fix permissions at the end of the install
    set_permissions.path = "$$INSTALL_ROOT$$PREFIX/bin/"  # A random path. Without one, qmake refuses to create the rule.
    unix:set_permissions.extra = (chmod 755 \"$$INSTALL_ROOT$$PREFIX/bin/notepadqq\" || \
                                  echo \"couldn\'t set permissions of $$INSTALL_ROOT$$PREFIX/bin/notepadqq\")

    # MAKE INSTALL
    INSTALLS += target \
         icon_h16 icon_h24 icon_h32 icon_h48 icon_h64 icon_h96 icon_h128 icon_h256 icon_h512 icon_hscalable \
         misc_data launch shortcuts \
         set_permissions

}
