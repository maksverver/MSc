AC_INIT(ParityGameSolver, 0.1)
AM_INIT_AUTOMAKE([-Wall -Werror foreign no-define])
AC_CONFIG_SRCDIR([main.cc])
AC_LANG(C++)

AC_ARG_ENABLE( debug,
    [AS_HELP_STRING( [--enable-debug], 
                     [enable debugging (slower but safer)] )],
    [   # Only override CXXFLAGS if they haven't been set by the user:
        CXXFLAGS=${CXXFLAGS:--O0 -g}
        DEBUG_FLAGS=-D_GLIBCXX_DEBUG ])
AC_SUBST(DEBUG_FLAGS)

AC_ARG_WITH( mpi,
    [AS_HELP_STRING( [--with-mpi],
                     [support distributed solving using MPI] )],
                     , [with_mpi=auto] )

AC_ARG_WITH( mpiP,
    [AS_HELP_STRING( [--with-mpiP@<:@=DIR@:>@],
                     [use mpiP profiler (DIR: installation prefix)])],
                     , [with_mpiP=no] )

# These checks will set CXXFLAGS, so any modifications to these variables
# should be done earlier, or I won't be able to identify values set by the user.
AX_PROG_CXX_MPI([test x"$with_mpi" != xno], [
    AS_IF([test x"$with_mpiP" != xno], [
        AS_IF([test x"$with_mpiP" != xyes], [
            AS_IF([test -d "${with_mpiP}/lib"], [
                LDFLAGS="-L${with_mpiP}/lib ${LDFLAGS}"
            ], [
                AC_MSG_WARN([--with-mpiP expected ${with_mpiP}/lib to be a directory])
            ])
        ])
        LIBS="-lm -lbfd -liberty -lunwind ${LIBS}"
        AC_CHECK_LIB([mpiP],[mpiPi],,[AC_MSG_ERROR([mpiP not found])])
    ])
    with_mpi=yes
], [
    AS_IF([test x"$with_mpi" = xyes], [
        AC_MSG_FAILURE([MPI C++ compiler requested, but couldn't use MPI])
    ], [test x"$with_mpi" != xno], [
        AC_MSG_WARN([No MPI C++ compiler found, won't use MPI])
    ])
    AS_IF([test x"$with_mpiP" != xno], [
        AC_MSG_ERROR([mpiP requires MPI])
    ])
    with_mpi=no
])

AM_CONDITIONAL([WITH_MCRL2],   [false])  # TODO
AM_CONDITIONAL([WITH_MPI],     [test x"$with_mpi" = xyes])
AM_CONDITIONAL([WITH_THREADS], [false])  # TODO
AM_CONDITIONAL([USE_TIMER],    [true])   # TODO

AM_COND_IF([WITH_MCRL2],   AC_DEFINE([WITH_MCRL2]))
AM_COND_IF([WITH_MPI],     AC_DEFINE([WITH_MPI]))
AM_COND_IF([WITH_THREADS], AC_DEFINE([WITH_THREADS]))
AM_COND_IF([USE_TIMER],    AC_DEFINE([USE_TIMER]))

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
