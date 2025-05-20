TEMPLATE = subdirs
SUBDIRS = src/ui \
    src/ui-tests
QMAKE_CXXFLAGS += -Wall -Wextra -Werror -fPIC
QMAKE_DISTCLEAN += Makefile && rm -rf out
