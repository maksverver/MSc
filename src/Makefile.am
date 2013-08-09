ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS   = $(COMMON_CFLAGS)
AM_CXXFLAGS = $(COMMON_CFLAGS)

COMMON_CFLAGS = $(DEBUG_FLAGS) -Wall

bin_PROGRAMS = solver

solver_LDADD = 
solver_SOURCES = \
	Abortable.cc Abortable.h \
	attractor.h attractor_impl.h \
	compatibility.cc compatibility.h \
	ComponentSolver.cc ComponentSolver.h \
	Decimal.cc Decimal.h \
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
	VertexPartition.h

if USE_TIMER
solver_SOURCES += Timer.h timing.cc
solver_LDADD += -lrt
endif

if WITH_MPI
solver_SOURCES += \
	AsyncMpiAttractorAlgorithm.cc AsyncMpiAttractorAlgorithm.h \
	MpiAttractorAlgorithm.h \
	MpiRecursiveSolver.cc MpiRecursiveSolver.h \
	MpiSpmSolver.cc MpiSpmSolver.h \
	MpiUtils.cc MpiUtils.h \
	SyncMpiAttractorAlgorithm.h SyncMpiAttractorAlgorithm.cc
endif

if WITH_THREADS
solver_SOURCES += \
	ConcurrentRecursiveSolver.cc ConcurrentRecursiveSolver.h
endif

#CFLAGS=-Wall -O2 -g  # default flags
#CFLAGS+=-O0 -D_GLIBCXX_DEBUG  # additional debug flags
#CFLAGS=-Wall -O3 -DNDEBUG  # for optimal performance
#LDLIBS+=-L$(HOME)/google-perftools/lib -lprofiler -lunwind

#docs:
#	doxygen

# mcrl2-example: $(OBJS) mcrl2-example.o
# 	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
