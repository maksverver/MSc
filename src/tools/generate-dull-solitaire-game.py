#!/usr/bin/env python

# Usage:
#  generate-dull-solitaire-game.py <N>
#
# Generates the N-th order dull/solitaire game, as described in Section 4.2
# of Gazda & Willemse "Zielonka's recursive algorithm: dull, weak and solitaire
# games and tighter bounds".
#
# These games have 3N vertices, 4N edges, 2N+1 priorities and are supposed to
# require 2**N recursive invocations of Zielonka's recursive algorithm to solve,
# unless optimizations are implemented (like SCC decomposition).
#
# All vertices are controlled by Even (hence solitaire) and all priorities
# are nondecreasing along edges in the graph (hence dull).

import sys

try:
    N, = map(int, sys.argv[1:])
    assert N > 0
except:
    print('Usage: ' + sys.argv[0] + ' <N>')
    sys.exit(1)

# Each vertex is described by a triple <player, priority, indices of successors>
G = [ (0, 2+i, [max(i-1,0)])   for i in range(2*N) ] \
  + [ (0, 1,   [2*N+i,2*i+1])  for i in range(N)   ]

# Write out game in PGSolver format:
print("parity " + str(3*N) + ";")
for i, (player, priority, successors) in enumerate(G):
    print(str(i) + ' ' + str(priority) + ' ' + str(player) + ' ' +
          ','.join(map(str, successors)) + ";")
