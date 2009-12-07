# set this to 1 to compile in support for mCRL2 (and link to mCRL2 libs)
WITH_MCRL2=

OBJS= \
	compatibility.o \
	ComponentSolver.o \
	FocusListLiftingStrategy.o \
	Graph.o \
	GraphOrdering.o \
	LiftingStrategy.o \
	LinearLiftingStrategy.o \
	MaxMeasureLiftingStrategy.o \
	OldMaxMeasureLiftingStrategy.o \
	ParityGame.o \
	ParityGame_IO.o \
	ParityGame_verify.o \
	PredecessorLiftingStrategy.o \
	RecursiveSolver.o \
	SmallProgressMeasures.o \
	logging.o \
	main.o \
	timing.o

CFLAGS=-Wall -Wextra -O2
LDLIBS=

ifdef WITH_MCRL2
CFLAGS:=$(CFLAGS) -DWITH_MCRL2=$(WITH_MCRL2) \
	-I../../mcrl2/include \
	-I../../mcrl2/include/mcrl2/aterm \
	-I../../mcrl2/include/mcrl2/atermpp
LDLIBS:=$(LDLIBS) -L../../mcrl2/lib/mcrl2 -lmcrl2
endif

CXXFLAGS:=$(CFLAGS)
# For debugging:
#CXXFLAGS:=$(CFLAGS) -O0 -g -fmudflap -lmudflap

all: main

docs:
	doxygen

main: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f *.o

distclean: clean
	rm -f main
	rm -rf html

.PHONY: all clean distclean docs
