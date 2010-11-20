# set this to 1 to compile in support for mCRL2 (and link to mCRL2 libs)
WITH_MCRL2=

OBJS= \
	Abortable.o \
	compatibility.o \
	ComponentSolver.o \
	DecycleSolver.o \
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

CFLAGS=-Wall -O0
LDLIBS=-lrt

ifndef WITH_MCRL2
CFLAGS:=$(CFLAGS) -Wextra -DUSE_TIMER
endif

ifdef WITH_MCRL2
CFLAGS:=$(CFLAGS) -DWITH_MCRL2=$(WITH_MCRL2) \
	-I../../mcrl2/include \
	-I../../mcrl2/include/aterm \
	-I../../mcrl2/include/mcrl2/atermpp
LDLIBS:=$(LDLIBS) -L../../mcrl2/lib/mcrl2 -lmcrl2_core -lmcrl2_data
endif

CXXFLAGS:=$(CFLAGS)
# For debugging:
CXXFLAGS:=$(CFLAGS) -g
#CXXFLAGS:=$(CFLAGS) -O0 -g -fmudflap -lmudflap

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
