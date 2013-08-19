#!/usr/bin/env python

# Usage:
#  generate-zielonka-hard-game.py <N>
#
# Generates the N-th order games from "Zielonka's recursive algorithm: dull,
# weak and solitaire games and tighter bounds".  These are supposed to give
# a tighter lower bound on Zielonka's algorithm than Friedmann's games, and are
# difficult even with optimizations like SCC decomposition enabled.
#
# These games have 3N vertices, 6N-N edges, and N+2 priorities.

import sys

try:
    N, = map(int, sys.argv[1:])
    assert N > 0
except:
    print('Usage: ' + sys.argv[0] + ' <N>')
    sys.exit(1)

# Each vertex is described by a triple <player, priority, indices of successors>
G = [ (i&1,    i+2,    [   N+i ] + [i+1]*(i+1<N))   for i in range(N) ] \
  + [ (i&1,    i&1^1,  [ 2*N+i ] + [i+1]*(i+1<N))   for i in range(N) ] \
  + [ (i&1^1,  i&1^1,  [   N+i ] + [2*N+i-1]*(i>0)) for i in range(N) ] \

# Write out game in PGSolver format:
print("parity " + str(3*N) + ";")
for i, (player, priority, successors) in enumerate(G):
    print(str(i) + ' ' + str(priority) + ' ' + str(player) + ' ' +
          ','.join(map(str, successors)) + ";")
