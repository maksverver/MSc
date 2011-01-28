#!/usr/bin/env python

# Usage:
#   generate-friedmann-game.py <N>
#
# Generates the N-th order game from Oliver Friedmann's "Recursive Solving
# Of Parity Games Requires Exponential Time", which requires at least fib(n)
# iterations to solve (where fib(n) is the n-th Fibonacci number).

# Result is in PGSolver format, i.e. higher priorities dominate lower ones.

import sys

try:
    N, = map(int, sys.argv[1:])
    assert N > 0
except:
    print('Usage: ' + sys.argv[0] + ' <N>')
    sys.exit(1)

# There are 5*N vertices in five groups of N vertices each, mapped to integers
# between 0 and 5*N (exclusive) as follows:

def a(i): return 0*N + i - 1
def b(i): return 1*N + i - 1
def c(i): return 2*N + i
def d(i): return 3*N + i
def e(i): return 4*N + i

# Each vertex is described by a triple <player, priority, indices of successors>
G = [(i&1^1, i&1^1, [b(i),d(i-1)])                      for i in range(1,N+1)] \
  + [(i&1,   i&1^1, [a(i)]+[c(i)]*(i<N))                for i in range(1,N+1)] \
  + [(i&1^1, 3*i+5, [b(i+1),d(i)])                          for i in range(N)] \
  + [(i&1,   3*i+4, [c(i)]+[d(i-1)]*(i>0)+[d(i+1)]*(i+1<N)) for i in range(N)] \
  + [(i&1^1, 3*i+3, [b(i+1),d(i)])                          for i in range(N)]

# Write out game in PGSolver format:
print("parity " + str(5*N - 1) + ";")
for i, (player, priority, successors) in enumerate(G):
    print(str(i) + ' ' + str(priority) + ' ' + str(player) + ' ' +
          ','.join(map(str, successors)) + ";")
