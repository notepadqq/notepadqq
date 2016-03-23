AC_INIT([notepadqq],
        m4_esyscmd([printf $(grep '^#define POINTVERSION' src/ui/include/notepadqq.h | cut -d '"' -f2)]),
        [https://github.com/notepadqq/notepadqq/issues])
AC_CONFIG_SRCDIR([.])


m4_include([ax_cxx_compile_stdcxx_0x.m4])
m4_include([ax_have_qt.m4])


# "1.9 tar-ustar" is required because we have paths with more than 99 characters
AM_INIT_AUTOMAKE([1.9 foreign dist-bzip2 dist-xz tar-ustar])


AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_0X


# check for Qt5
PKG_CHECK_MODULES([Qt5Core], [Qt5Core], [], [
    AC_CHECK_LIB([Qt5Core], [main], [], [exit 1])
])
PKG_CHECK_MODULES([Qt5Gui], [Qt5Gui], [], [
    AC_CHECK_LIB([Qt5Gui], [main], [], [exit 1])
])
PKG_CHECK_MODULES([Qt5Network], [Qt5Network], [], [
    AC_CHECK_LIB([Qt5Network], [main], [], [exit 1])
])
PKG_CHECK_MODULES([Qt5PrintSupport], [Qt5PrintSupport], [], [
    AC_CHECK_LIB([Qt5PrintSupport], [main], [], [exit 1])
])
PKG_CHECK_MODULES([Qt5Svg], [Qt5Svg], [], [
    AC_CHECK_LIB([Qt5Svg], [main], [], [exit 1])
])
PKG_CHECK_MODULES([Qt5WebKit], [Qt5WebKit], [], [
    AC_CHECK_LIB([Qt5WebKit], [main], [], [exit 1])
])
PKG_CHECK_MODULES([Qt5WebKitWidgets], [Qt5WebKitWidgets], [], [
    AC_CHECK_LIB([Qt5WebKitWidgets], [main], [], [exit 1])
])
PKG_CHECK_MODULES([Qt5Widgets], [Qt5Widgets], [], [
    AC_CHECK_LIB([Qt5Widgets], [main], [], [exit 1])
])

AX_HAVE_QT
if test x"$have_qt" != x"yes" ; then
    exit 1
fi

AC_ARG_VAR([QMAKE], [Qt5 qmake command])
if test x"$QMAKE" = x ; then
    AC_CHECK_PROG([QMAKE], [qmake], [yes:], [], [$PATH_SEPARATOR$QT_DIR/qt5/bin$PATH_SEPARATOR])
    if test x"$QMAKE" != x"yes:" ; then
        AC_MSG_ERROR([No Qt5 qmake found.])
    else
        QMAKE="$QT_DIR/qt5/bin/qmake"
        AC_MSG_RESULT([    QMAKE=$QMAKE])
    fi
else
    AC_MSG_RESULT([    QMAKE=$QMAKE])
fi


AC_CONFIG_FILES([Makefile])
AC_OUTPUT

