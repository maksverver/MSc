AC_INIT(ParityGameSolver, 0.1)
AM_INIT_AUTOMAKE([-Wall -Werror foreign no-define])
AC_CONFIG_SRCDIR([main.cc])

AC_ARG_ENABLE(debug,
    AS_HELP_STRING( [--enable-debug],
                    [enable debugging (slower but safer)]),
    [   # Only override CFLAGS/CXXFLAGS if they haven't been set by the user:
        CFLAGS=${CFLAGS:--O0 -g}
        CXXFLAGS=${CXXFLAGS:--O0 -g}
        DEBUG_FLAGS=-D_GLIBCXX_DEBUG ] )
AC_SUBST(DEBUG_FLAGS)

# These checks will set CFLAGS/CXXFLAGS, so any modifications to these variables
# should be done earlier, or I won't be able to identify values set by the user.
AC_PROG_CC
AC_PROG_CXX

AC_LANG(C++)

AM_CONDITIONAL([WITH_MCRL2],   [false])  # TODO
AM_CONDITIONAL([WITH_MPI],     [false])  # TODO
AM_CONDITIONAL([WITH_THREADS], [false])  # TODO
AM_CONDITIONAL([USE_TIMER],    [true])   # TODO

AM_COND_IF([WITH_MCRL2],   AC_DEFINE([WITH_MCRL2]))
AM_COND_IF([WITH_MPI],     AC_DEFINE([WITH_MPI]))
AM_COND_IF([WITH_THREADS], AC_DEFINE([WITH_THREADS]))
AM_COND_IF([USE_TIMER],    AC_DEFINE([USE_TIMER]))

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
