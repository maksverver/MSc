# N.B. this Makefile uses gmake conditionals

# set this to 1 to compile in support for mCRL2 (and link to mCRL2 libs)
# WITH_MCRL2=

# set this to 1 to compile in support for MPI
# WITH_MPI=

# set this to 1 to compile in support for concurrency
# WITH_THREADS=

bin_PROGRAMS = solver

solver_SOURCES = \
	Abortable.cc Abortable.h \
	attractor.h attractor_impl.h \
	compatibility.cc compatibility.h \
	ComponentSolver.cc ComponentSolver.h \
	DecycleSolver.cc DecycleSolver.h \
	DeloopSolver.cc DeloopSolver.h \
	DenseMap.h DenseSet.h \
	FocusListLiftingStrategy.cc FocusListLiftingStrategy.h \
	GamePart.cc GamePart.h \
	Graph.cc Graph.h Graph_impl.h \
	GraphOrdering.cc GraphOrdering.h \
	LiftingStrategy.cc LiftingStrategy.h \
	LinearLiftingStrategy.cc LinearLiftingStrategy.h \
	LinPredLiftingStrategy.h \
	Logger.cc Logger.h \
	main.cc \
	MaxMeasureLiftingStrategy.cc MaxMeasureLiftingStrategy.h \
	OldMaxMeasureLiftingStrategy.cc OldMaxMeasureLiftingStrategy.h \
	ParityGame.cc ParityGame.h ParityGame_impl.h ParityGame_IO.cc ParityGame_verify.cc \
	ParityGameSolver.cc ParityGameSolver.h ParityGameSolver_impl.h \
	PredecessorLiftingStrategy.cc PredecessorLiftingStrategy.h \
	RecursiveSolver.cc RecursiveSolver.h \
	RefCounted.h \
	SCC.h SCC_impl.h \
	SmallProgressMeasures.cc SmallProgressMeasures.h SmallProgressMeasures_impl.h \
	Timer.h timing.c \
	VertexPartition.h

#	ConcurrentRecursiveSolver.cc ConcurrentRecursiveSolver.h

#	AsyncMpiAttractorAlgorithm.cc AsyncMpiAttractorAlgorithm.h \
#	MpiAttractorAlgorithm.h \
#	MpiRecursiveSolver.cc MpiRecursiveSolver.h \
#	MpiSpmSolver.cc MpiSpmSolver.h \
#	MpiUtils.cc MpiUtils.h \
#	SyncMpiAttractorAlgorithm.h
#	SyncMpiAttractorAlgorithm.cc 

#	ParityGame_pbes.h

solver_LIBS = -lrt

#CFLAGS=-Wall -O2 -g  # default flags
#CFLAGS+=-O0 -D_GLIBCXX_DEBUG  # additional debug flags
#CFLAGS=-Wall -O3 -DNDEBUG  # for optimal performance
#LDLIBS+=-L$(HOME)/google-perftools/lib -lprofiler -lunwind
#LDLIBS+=-L$(HOME)/mpiP/lib -lmpiP -lm -lbfd -liberty -lunwind # mpiP

#ifdef WITH_THREADS
#CFLAGS+=-DWITH_THREADS -I../../tbb/include -fopenmp
#LDLIBS+=-L../../tbb/lib -ltbb
#OBJS+=ConcurrentRecursiveSolver.o
#endif

#ifdef WITH_MPI
#CFLAGS+=-DWITH_MPI
#OBJS+=	AsyncMpiAttractorAlgorithm.o \
#	GamePart.o \
#	MpiRecursiveSolver.o \
#	MpiSpmSolver.o \
#	MpiUtils.o \
#	SyncMpiAttractorAlgorithm.o
#endif

#ifndef WITH_MCRL2
#CFLAGS+=-Wextra -DUSE_TIMER
#endif

#ifdef WITH_MCRL2
#CFLAGS+=-DWITH_MCRL2=$(WITH_MCRL2) \
#	-I../../mcrl2/include \
#	-I../../mcrl2/include/dparser
#LDLIBS+=-L../../mcrl2/lib/mcrl2 -lmcrl2_atermpp -lmcrl2_bes -lmcrl2_core -lmcrl2_data -lmcrl2_lps -lmcrl2_lts -lmcrl2_pbespgsolve -lmcrl2_pbes -lmcrl2_process -lmcrl2_syntax -lmcrl2_utilities -ldparser -lsvc -ldot_syntax -lfsm_syntax
#endif

#ifdef WITH_MPI
#CC=mpicc
#CXX=mpicxx
#endif

#docs:
#	doxygen

# mcrl2-example: $(OBJS) mcrl2-example.o
# 	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
