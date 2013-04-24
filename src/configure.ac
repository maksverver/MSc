AC_INIT(ParityGameSolver, 0.1)
AM_INIT_AUTOMAKE([-Wall -Werror foreign no-define])
AC_CONFIG_SRCDIR([main.cc])

AC_PROG_CC
AC_PROG_CXX
AC_LANG(C++)

AM_CONDITIONAL([WITH_MCRL2],   [false])
AM_CONDITIONAL([WITH_MPI],     [false])
AM_CONDITIONAL([WITH_THREADS], [false])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
