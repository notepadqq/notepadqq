#-------------------------------------------------
#
# Project created by QtCreator 2010-11-26T16:20:15
#
#-------------------------------------------------

QT       += core gui \
            network

TARGET = notepadqq
TEMPLATE = app

DESTDIR = ../build


SOURCES += main.cpp\
        mainwindow.cpp \
    qsciscintillaqq.cpp \
    frmabout.cpp \
    userlexer.cpp \
    qtabwidgetqq.cpp \
    generalfunctions.cpp

HEADERS  += mainwindow.h \
    qsciscintillaqq.h \
    frmabout.h \
    constants.h \
    userlexer.h \
    qtabwidgetqq.h \
    generalfunctions.h

FORMS    += mainwindow.ui \
    frmabout.ui

win32 {

    OTHER_FILES += \
        ../../qscintilla/include/Scintilla.iface \
        ../../qscintilla/include/HFacer.py \
        ../../qscintilla/include/Face.py

    # this is needed in order to set the correct export declaration on QScintilla headers
    DEFINES += QSCINTILLA_DLL
}

LIBS += -lqscintilla2

OTHER_FILES += \
    qscintilla/include/Scintilla.iface \
    qscintilla/include/HFacer.py \
    qscintilla/include/Face.py

RESOURCES += \
    resources.qrc

TRANSLATIONS += L10n/notepadqq_en.ts \
                L10n/notepadqq_it.ts \


unix {
    # MAKE INSTALL
    INSTALLS += target \
        vfiles
        #desktop \
        #icon64

    target.path = /usr/bin/
    target.files += $$DESTDIR/$$TARGET

    vfiles.path = /
    vfiles.files += sys_files/*
    #desktop.path = /usr/share/applications/
    #desktop.files += $$SYS_FILES/usr/share/applications/notepadqq.desktop
    #icon64.path = $$DATADIR/icons/hicolor/64x64/apps
    #icon64.files += fsudoku.png
}
