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

AC_ARG_WITH( tbb,
    [AS_HELP_STRING( [--with-tbb<:@=DIR@:>@],
        [compile with Intel Thread Building Blocks (DIR: installation prefix)])],
    , [with_tbb=auto] )
AC_ARG_WITH( openmp,
    [AS_HELP_STRING( [--with-openmp], [compile with OpenMP])],
    , [with_openmp=auto])
AC_ARG_ENABLE( threads,
    [AS_HELP_STRING( [--enable-threads],
                     [support multithreaded solving using OpenMP and TBB])],
    [with_threads=$enableval], [with_threads=auto] )

dnl These checks will set CXXFLAGS, so any modifications to these variables
dnl should be done earlier, or I won't be able to identify values set by the user.
AX_PROG_CXX_MPI([test x"$with_mpi" != xno], [
    AS_IF([test x"$with_mpiP" != xno], [
        AS_IF([test x"$with_mpiP" != xyes], [
            AS_IF([test -d "${with_mpiP}/lib"], [
                LDFLAGS="-L${with_mpiP}/lib ${LDFLAGS}"
            ], [
                AC_MSG_WARN([--with-mpiP expected $with_mpiP/lib to be a directory])
            ])
        ])
        LIBS="-lm -lbfd -liberty -lunwind ${LIBS}"
        AC_CHECK_LIB([mpiP], [mpiPi], , [AC_MSG_ERROR([mpiP not found])])
    ])
], [
    AS_IF([test x"$with_mpi" = xyes], [
        AC_MSG_FAILURE([MPI requested, but no usable MPI C++ compiler was found])
    ], [test x"$with_mpi" != xno], [
        AC_MSG_WARN([MPI disabled (no usable MPI C++ compiler found)])
    ])
    AS_IF([test x"$with_mpiP" != xno], [
        AC_MSG_ERROR([mpiP requires MPI])
    ])
    with_mpi=no
])

AS_IF([test x"$with_threads" != xno], [

    dnl Check for OpenMP:
    AS_IF([test x"$with_openmp" != xno], [
        AX_OPENMP( , [
            AS_IF([test x"$with_openmp" != xauto], [
                AC_MSG_FAILURE([no OpenMP support detected])
            ], [
                AC_MSG_WARN([no OpenMP support detected])
            ])
            with_openmp=no
        ])
    ])

    dnl Check for TBB
    AS_IF([test x"$with_tbb" != xno], [

        dnl Add any user-specified header and library paths:
        AS_IF([test x"$with_tbb" != xyes && test x"$with_tbb" != xauto], [
            AS_IF([test -d "${with_tbb}/include"], [
                CXXFLAGS="-I${with_tbb}/include ${CXXFLAGS}"
            ], [
                AC_MSG_WARN([--with-tbb expected $with_tbb/include to be a directory])
            ])
            AS_IF([test -d "${with_tbb}/lib"], [
                LDFLAGS="-L${with_tbb}/lib ${LDFLAGS}"
            ], [
                AC_MSG_WARN([--with-tbb expected $with_tbb/lib to be a directory])
            ])
        ])

        AC_CHECK_LIB([tbb], [TBB_runtime_interface_version], [
            TBB_LIBS="-ltbb"
        ], [
            AS_IF([test x"$with_tbb" != xauto], [
                AC_MSG_FAILURE([libtbb was not found])
            ], [
                AC_MSG_WARN([libtbb was not found])
            ])
            with_tbb=no
        ])
    ])

    AS_IF([test x"$with_tbb" = xno], [
        AS_IF([test x"$with_threads" != xauto], [
            AC_MSG_FAILURE([threads require libtbb])
        ], [
            AC_MSG_WARN([threads disabled (because libtbb is disabled)])
        ])
        with_threads=no
    ], [test x"$with_openmp" = xno], [
        AS_IF([test x"$with_threads" != xauto], [
            AC_MSG_FAILURE([threads require OpenMP])
        ], [
            AC_MSG_WARN([threads disabled (because OpenMP is disabled)])
        ])
        with_threads=no
    ], [
        CXXFLAGS="${OPENMP_CXXFLAGS} ${CXXFLAGS}"
        LIBS="${TBB_LIBS} ${LIBS}"
    ])
])

AM_CONDITIONAL([WITH_MCRL2],   [false])  dnl TODO
AM_CONDITIONAL([WITH_MPI],     [test x"$with_mpi" != xno])
AM_CONDITIONAL([WITH_THREADS], [test x"$with_threads" != xno])
AM_CONDITIONAL([USE_TIMER],    [true])   dnl TODO

AM_COND_IF([WITH_MCRL2],   AC_DEFINE([WITH_MCRL2]))
AM_COND_IF([WITH_MPI],     AC_DEFINE([WITH_MPI]))
AM_COND_IF([WITH_THREADS], AC_DEFINE([WITH_THREADS]))
AM_COND_IF([USE_TIMER],    AC_DEFINE([USE_TIMER]))

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
