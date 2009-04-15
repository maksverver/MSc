OBJS= \
	Compatibility.o \
	ComponentSolver.o \
	FocusListLiftingStrategy.o \
	Graph.o \
	GraphOrdering.o \
	LiftingStrategy.o \
	LinearLiftingStrategy.o \
	MaxMeasureLiftingStrategy.o \
	ParityGame.o \
	ParityGame_IO.o \
	PredecessorLiftingStrategy.o \
	SmallProgressMeasures.o \
	logging.o \
	main.o \
	timing.o

CFLAGS=-Wall -O2 -g -DWITH_MCRL2 \
	-I../../mcrl2/include \
	-I../../mcrl2/include/mcrl2/aterm \
	-I../../mcrl2/include/mcrl2/atermpp
LDLIBS=-L../../mcrl2/lib/mcrl2 -lmcrl2

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
