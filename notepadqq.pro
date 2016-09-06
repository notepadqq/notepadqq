TEMPLATE = subdirs
SUBDIRS = src/ui \
    src/ui-tests
QMAKE_DISTCLEAN += Makefile && rm -rf out
