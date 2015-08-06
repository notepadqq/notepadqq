m4_include([ax_have_qt.m4])

AC_INIT([notepadqq],
        m4_esyscmd([printf $(grep '^#define POINTVERSION' src/ui/include/notepadqq.h | cut -d '"' -f2)]),
        [https://github.com/notepadqq/notepadqq/issues])
AC_CONFIG_SRCDIR([.])

AM_INIT_AUTOMAKE([dist-bzip2 dist-xz foreign])

AC_PROG_CXX
AX_HAVE_QT

AC_ARG_VAR([QMAKE], [Qt5 qmake command])
if test x"$QMAKE" = x ; then
    AC_CHECK_PROG([QMAKE], [qmake], [yes], [], [$PATH_SEPARATOR$QT_DIR/qt5/bin$PATH_SEPARATOR])
    if test x"$QMAKE" != x"yes" ; then
        AC_MSG_ERROR([Please set QMAKE or install qtchooser.])
    else
        QMAKE="$QT_DIR/qt5/bin/qmake"
    fi
fi
AC_MSG_RESULT([    QMAKE=$QMAKE])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

