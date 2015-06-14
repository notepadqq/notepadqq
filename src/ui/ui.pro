#-------------------------------------------------
#
# Project created by QtCreator 2014-08-13T23:31:36
#
#-------------------------------------------------

QT       += core gui svg widgets webkitwidgets printsupport network

CONFIG += c++11

TARGET = notepadqq-bin
TEMPLATE = app

RCC_DIR = ../../out/build_data
UI_DIR = ../../out/build_data
MOC_DIR = ../../out/build_data
OBJECTS_DIR = ../../out/build_data

QMAKE_CXXFLAGS_WARN_ON += -Wold-style-cast

# clear "rpath" so that we can override Qt lib path via LD_LIBRARY_PATH
QMAKE_RPATH=

# Avoid automatic casts from QString to QUrl
DEFINES += QT_NO_URL_CAST_FROM_STRING

unix: CMD_FULLDELETE = rm -rf
win32: CMD_FULLDELETE = del /F /S /Q

isEmpty(DESTDIR) {
    CONFIG(debug, debug|release) {
        message(Debug build)
        DESTDIR = ../../out/debug/lib
    }
    CONFIG(release, debug|release) {
        message(Release build)
        DESTDIR = ../../out/release/lib
    }
}

APPDATADIR = "$$DESTDIR/../appdata"
BINDIR = "$$DESTDIR/../bin"

INSTALLFILESDIR = ../../support_files

SOURCES += main.cpp\
    mainwindow.cpp \
    topeditorcontainer.cpp \
    editortabwidget.cpp \
    docengine.cpp \
    frmabout.cpp \
    notepadqq.cpp \
    frmpreferences.cpp \
    iconprovider.cpp \
    EditorNS/editor.cpp \
    EditorNS/bannerfilechanged.cpp \
    EditorNS/bannerbasicmessage.cpp \
    EditorNS/bannerfileremoved.cpp \
    EditorNS/customqwebview.cpp \
    clickablelabel.cpp \
    frmencodingchooser.cpp \
    EditorNS/bannerindentationdetected.cpp \
    frmindentationmode.cpp \
    singleapplication.cpp \
    localcommunication.cpp \
    Search/filesearchresultswidget.cpp \
    Search/frmsearchreplace.cpp \
    Search/searchinfilesworker.cpp \
    Search/replaceinfilesworker.cpp \
    Search/dlgsearching.cpp \
    Search/searchresultsitemdelegate.cpp \
    Extensions/extension.cpp \
    frmlinenumberchooser.cpp \
    Extensions/extensionsserver.cpp \
    Extensions/Stubs/stub.cpp \
    Extensions/runtimesupport.cpp \
    Extensions/Stubs/windowstub.cpp \
    Extensions/Stubs/notepadqqstub.cpp \
    Extensions/Stubs/editorstub.cpp \
    Extensions/extensionsloader.cpp \
    globals.cpp \
    Extensions/Stubs/menuitemstub.cpp \
    Extensions/installextension.cpp

HEADERS  += include/mainwindow.h \
    include/topeditorcontainer.h \
    include/editortabwidget.h \
    include/docengine.h \
    include/frmabout.h \
    include/notepadqq.h \
    include/frmpreferences.h \
    include/iconprovider.h \
    include/EditorNS/editor.h \
    include/EditorNS/bannerfilechanged.h \
    include/EditorNS/bannerbasicmessage.h \
    include/EditorNS/bannerfileremoved.h \
    include/EditorNS/customqwebview.h \
    include/clickablelabel.h \
    include/frmencodingchooser.h \
    include/EditorNS/bannerindentationdetected.h \
    include/frmindentationmode.h \
    include/singleapplication.h \
    include/localcommunication.h \
    include/Search/filesearchresultswidget.h \
    include/Search/filesearchresult.h \
    include/Search/frmsearchreplace.h \
    include/Search/searchinfilesworker.h \
    include/Search/replaceinfilesworker.h \
    include/Search/searchhelpers.h \
    include/Search/dlgsearching.h \
    include/Search/searchresultsitemdelegate.h \
    include/Extensions/extension.h \
    include/frmlinenumberchooser.h \
    include/Extensions/extensionsserver.h \
    include/Extensions/Stubs/stub.h \
    include/Extensions/runtimesupport.h \
    include/Extensions/Stubs/windowstub.h \
    include/Extensions/Stubs/notepadqqstub.h \
    include/Extensions/Stubs/editorstub.h \
    include/Extensions/extensionsloader.h \
    include/globals.h \
    include/Extensions/Stubs/menuitemstub.h \
    include/Extensions/installextension.h

FORMS    += mainwindow.ui \
    frmabout.ui \
    frmpreferences.ui \
    frmencodingchooser.ui \
    frmindentationmode.ui \
    Search/dlgsearching.ui \
    Search/frmsearchreplace.ui \
    frmlinenumberchooser.ui \
    Extensions/installextension.ui

RESOURCES += \
    resources.qrc


### EXTRA TARGETS ###

# Copy the editor in the "shared" folder
editorTarget.target = make_editor
editorTarget.commands = (cd \"$$PWD\" && \
                         $${CMD_FULLDELETE} \"$$APPDATADIR/editor\" && \
                         cd \"../editor\" && \
                         $(MAKE) DESTDIR=\"$$APPDATADIR/editor\")

# Copy the extension_tools in the "shared" folder
extensionToolsTarget.target = make_extensionTools
extensionToolsTarget.commands = (cd \"$$PWD\" && \
                           $${CMD_FULLDELETE} \"$$APPDATADIR/extension_tools\" && \
                           cd \"../extension_tools\" && \
                           $(MAKE) DESTDIR=\"$$APPDATADIR/extension_tools\")

launchTarget.target = make_launch
launchTarget.commands = (cd \"$$PWD\" && \
                         $${QMAKE_MKDIR} \"$$BINDIR/\" && \
                         $${QMAKE_COPY} \"$$INSTALLFILESDIR/launch/notepadqq\" \"$$BINDIR/\" && \
                         chmod 755 \"$$BINDIR/notepadqq\")

QMAKE_EXTRA_TARGETS += editorTarget extensionToolsTarget launchTarget
PRE_TARGETDEPS += make_editor make_extensionTools make_launch

### INSTALL ###
unix {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }

    target.path = "$$INSTALL_ROOT$$PREFIX/lib/notepadqq/"
    target.files += "$$DESTDIR/$$TARGET"

    icon_h16.path = "$$INSTALL_ROOT$$PREFIX/share/icons/hicolor/16x16/apps/"
    icon_h16.files += "$$INSTALLFILESDIR/icons/hicolor/16x16/apps/notepadqq.png"
    icon_h22.path = "$$INSTALL_ROOT$$PREFIX/share/icons/hicolor/22x22/apps/"
    icon_h22.files += "$$INSTALLFILESDIR/icons/hicolor/22x22/apps/notepadqq.png"
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

    # Make sure that the folders exists, otherwise qmake won't create the misc_data install rule
    system($${QMAKE_MKDIR} \"$$APPDATADIR/editor\")
    system($${QMAKE_MKDIR} \"$$APPDATADIR/extension_tools\")

    misc_data.path = "$$INSTALL_ROOT$$PREFIX/share/notepadqq/"
    misc_data.files += "$$APPDATADIR/editor"
    misc_data.files += "$$APPDATADIR/extension_tools"

    launch.path = "$$INSTALL_ROOT$$PREFIX/bin/"
    launch.files += "$$BINDIR/notepadqq"
    launch.CONFIG = no_check_exist     # Create the install rule even if the file doesn't exists when qmake is run

    shortcuts.path = "$$INSTALL_ROOT$$PREFIX/share/applications/"
    shortcuts.files += "$$INSTALLFILESDIR/shortcuts/notepadqq.desktop"

    # == Dummy target used to fix permissions at the end of the install ==
    # A random path. Without one, qmake refuses to create the rule.
    set_permissions.path = "$$INSTALL_ROOT$$PREFIX/bin/"
    # We want to keep $$INSTALL_ROOT as a variable in the makefile, so we use $(INSTALL_ROOT)
    unix:set_permissions.extra = chmod 755 $(INSTALL_ROOT)\"$$PREFIX/bin/notepadqq\"

    # MAKE INSTALL
    INSTALLS += target \
         icon_h16 icon_h22 icon_h24 icon_h32 icon_h48 icon_h64 icon_h96 icon_h128 icon_h256 icon_h512 icon_hscalable \
         misc_data launch shortcuts \
         set_permissions

}
