OBJS=Graph.o ParityGame.o SmallProgressMeasures.o LinearLiftingStrategy.o \
     PredecessorLiftingStrategy.o main.o
CXXFLAGS=-Wall -Wextra -O2 -g
# For debugging:
#CXXFLAGS=-Wall -Wextra -O0 -g -fmudflap -lmudflap

all: main

docs:
	doxygen

main: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f *.o

distclean: clean
	rm -f main
	rm -rf html

.PHONY: all clean distclean docs
