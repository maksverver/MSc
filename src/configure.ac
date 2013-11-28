AC_INIT(ParityGameSolver, 1.0)

echo '                                  +      +---+'
echo '  PARITY GAME SOLVER v1.0        / \ --->| 3 |<'
echo '                            --> + 0 +    +---+ \'
echo '                           /     \ /      / \__/'
echo '                        +---+     +      /'
echo '                        | 2 |<-----------'
echo '                        +---+'
echo ''

AM_INIT_AUTOMAKE([-Wall -Werror foreign no-define])
AC_CONFIG_SRCDIR([main.cc])
AC_CONFIG_MACRO_DIR([m4])
AC_LANG(C++)

dnl Check for --enable-debug / enable-optimized first, because we want to
dnl override the default CXXFLAGS before they are set by AC_PROG_CXX.

AC_ARG_ENABLE( debug, [ AS_HELP_STRING( [--enable-debug], 
        [create a debug build (slower but safer)] )], [
    # Only override CXXFLAGS if they haven't been set by the user:
    CXXFLAGS=${CXXFLAGS:--O0 -g}
    DEBUG_FLAGS="-D_GLIBCXX_DEBUG -DDEBUG"
    enable_debug=yes
], [
    enable_debug=no
])

AC_ARG_ENABLE( optimized, [AS_HELP_STRING( [--enable-optimized],
        [create a heavily optimized build (faster)] )], [
    AS_IF([test x"$enable_debug" != xno], [
        AC_MSG_FAILURE([can specify at most one of --enable-debug and --enable-optimized])
    ])
    # Only override CXXFLAGS if they haven't been set by the user:
    CXXFLAGS=${CXXFLAGS:--O3}
    DEBUG_FLAGS="-DNDEBUG"
    enable_optimized=yes
], [
    enable_optimized=no
])

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
    [AS_HELP_STRING( [--with-tbb@<:@=DIR@:>@],
        [compile with Intel Thread Building Blocks (DIR: installation prefix)])],
    , [with_tbb=auto] )
AC_ARG_WITH( openmp,
    [AS_HELP_STRING( [--with-openmp], [compile with OpenMP])],
    , [with_openmp=auto])
AC_ARG_ENABLE( threads,
    [AS_HELP_STRING( [--enable-threads],
                     [support multithreaded solving using OpenMP and TBB])],
    [with_threads=$enableval], [with_threads=auto] )

AC_ARG_WITH( mcrl2,
    [AS_HELP_STRING( [--with-mcrl2@<:@=DIR@:>@],
                     [support reading PBES files with mCRL2])],
    , [with_mcrl2=no] )

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

AS_IF([test x"$with_mcrl2" != xno], [

    dnl mCRL2 requires building with C++11 support (just the PBES module,
    dnl technically, but it's easier to build everything with the same setting)
    AX_CXX_COMPILE_STDCXX_11

    dnl Add any user-specified header and library paths:
    AS_IF([test x"$with_mcrl2" != xyes && test x"$with_mcrl2" != xauto], [
        AS_IF([test -d "${with_mcrl2}/include"], [
            CXXFLAGS="-I${with_mcrl2}/include -I${with_mcrl2}/include/dparser ${CXXFLAGS}"
        ], [
            AC_MSG_WARN([--with-mcrl2 expected $with_mcrl2/include to be a directory])
        ])
        AS_IF([test -d "${with_mcrl2}/lib/mcrl2"], [
            LDFLAGS="-L${with_mcrl2}/lib/mcrl2 ${LDFLAGS}"
        ], [
            AC_MSG_WARN([--with_mcrl2 expected with_mcrl2/lib to be a directory])
        ])
    ])

    dnl Prepend required mCRL2 libraries.
    LIBS="-lmcrl2_atermpp -lmcrl2_core -lmcrl2_data -lmcrl2_pbes -lmcrl2_utilities ${LIBS}"

    AC_MSG_CHECKING([for mCRL2 support])
    AC_LINK_IFELSE([
        AC_LANG_PROGRAM([
#include <mcrl2/pbes/pbes.h>
#include <mcrl2/pbes/parity_game_generator.h>
        ], [
mcrl2::pbes_system::pbes pbes;
pbes.load("/dev/null");
mcrl2::pbes_system::parity_game_generator pgg(pbes, true, true);
        ])
    ], [
        AC_MSG_RESULT([yes])
    ], [
        AC_MSG_RESULT([no])
        AC_MSG_FAILURE([building with mCRL2 support failed])
    ])
])

AM_CONDITIONAL([WITH_MCRL2],   [test x"$with_mcrl2" != xno])
AM_CONDITIONAL([WITH_MPI],     [test x"$with_mpi" != xno])
AM_CONDITIONAL([WITH_THREADS], [test x"$with_threads" != xno])
AM_CONDITIONAL([USE_TIMER],    [true])   dnl TODO

AM_COND_IF([WITH_MCRL2],   AC_DEFINE([WITH_MCRL2]))
AM_COND_IF([WITH_MPI],     AC_DEFINE([WITH_MPI]))
AM_COND_IF([WITH_THREADS], AC_DEFINE([WITH_THREADS]))
AM_COND_IF([USE_TIMER],    AC_DEFINE([USE_TIMER]))

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

AC_DEFUN([ECHO_ENABLED], [
    if test "x$[$2]" = xno; then
        echo ["$1" disabled]
    else
        echo ["$1" enabled]
    fi
])
echo ""
echo "  Build configuration summary:"
echo ""
ECHO_ENABLED([    Debug build:                    ], [enable_debug])
ECHO_ENABLED([    Optimized build:                ], [enable_optimized])
ECHO_ENABLED([    PBES input using mCRL2:         ], [with_mcrl2])
ECHO_ENABLED([    Multithreaded solving:          ], [with_threads])
ECHO_ENABLED([    Distributed solving using MPI:  ], [with_mpi])
echo ""
echo "  If this is not as expected, consult the output of the configure script"
echo "  or the detailed information in config.log to see what went wrong!"
echo ""
