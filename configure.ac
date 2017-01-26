
AC_PREREQ([2.59])

AC_INIT([Bravo], [0.0.0], [bugs AT bravotools.org])
AM_CONDITIONAL([LINUX], [test $(uname -s) = "Linux"])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_PROG_AR

AC_PROG_CXX
AC_PROG_RANLIB
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_TESTDIR([.], [apps/baretta:apps/serrano])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
 apps/baretta/Makefile
 apps/serrano/Makefile
 tests/Makefile
])
AC_OUTPUT


