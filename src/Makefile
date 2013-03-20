# N.B. this Makefile uses gmake conditionals

# set this to 1 to compile in support for mCRL2 (and link to mCRL2 libs)
WITH_MCRL2=1

# set this to 1 to compile in support for MPI
WITH_MPI=

# set this to 1 to compile in support for concurrency
WITH_THREADS=

OBJS= \
	Abortable.o \
	compatibility.o \
	ComponentSolver.o \
	DecycleSolver.o \
	DeloopSolver.o \
	FocusListLiftingStrategy.o \
	Graph.o \
	GraphOrdering.o \
	LiftingStrategy.o \
	LinearLiftingStrategy.o \
	Logger.o \
	MaxMeasureLiftingStrategy.o \
	OldMaxMeasureLiftingStrategy.o \
	ParityGame.o \
	ParityGame_IO.o \
	ParityGameSolver.o \
	ParityGame_verify.o \
	PredecessorLiftingStrategy.o \
	RecursiveSolver.o \
	SmallProgressMeasures.o \
	timing.o

CFLAGS=-Wall -O2 -g  # default flags
#CFLAGS+=-O0 -D_GLIBCXX_DEBUG  # additional debug flags
#CFLAGS=-Wall -O3 -DNDEBUG  # for optimal performance
LDLIBS=-lrt
#LDLIBS+=-L$(HOME)/google-perftools/lib -lprofiler -lunwind
#LDLIBS+=-L$(HOME)/mpiP/lib -lmpiP -lm -lbfd -liberty -lunwind # mpiP


ifdef WITH_THREADS
CFLAGS+=-DWITH_THREADS -I../../tbb/include -fopenmp
LDLIBS+=-L../../tbb/lib -ltbb
OBJS+=ConcurrentRecursiveSolver.o
endif

ifdef WITH_MPI
CFLAGS+=-DWITH_MPI
OBJS+=	AsyncMpiAttractorAlgorithm.o \
	GamePartition.o \
	MpiRecursiveSolver.o \
	MpiSpmSolver.o \
	MpiUtils.o \
	SyncMpiAttractorAlgorithm.o
endif

ifndef WITH_MCRL2
CFLAGS+=-Wextra -DUSE_TIMER
endif

ifdef WITH_MCRL2
CFLAGS+=-DWITH_MCRL2=$(WITH_MCRL2) \
	-I../../mcrl2/include \
	-I../../mcrl2/include/dparser
LDLIBS+=-L../../mcrl2/lib/mcrl2 -lmcrl2_atermpp -lmcrl2_bes -lmcrl2_core -lmcrl2_data -lmcrl2_lps -lmcrl2_lts -lmcrl2_pbespgsolve -lmcrl2_pbes -lmcrl2_process -lmcrl2_syntax -lmcrl2_utilities -ldparser -lsvc -ldot_syntax -lfsm_syntax
endif

ifdef WITH_MPI
CC=mpicc
CXX=mpicxx
endif

CXXFLAGS=$(CFLAGS)

all: main

docs:
	doxygen

main: $(OBJS) main.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f *.o

distclean: clean
	rm -f main
	rm -rf html

mcrl2-example: $(OBJS) mcrl2-example.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: all clean distclean docs
