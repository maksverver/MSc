#!/usr/bin/env python

# Usage:
#   generate-scc-hard-game.py <N>
#
# Generates a game with 4*N vertices that is designed to take a long time to
# solve with solvers that decompose games into strongly-connected components
# with unlimited recursion (taking O(N*N) time in those cases).
#
# Output is in PGSolver format, i.e. higher priorities dominate lower ones.
#
# Design:
#
# For argument N, we have 4*N vertices numbered 0 through 4*N (exclusive).
#
#   player(4*i + 0) = player(4*i + 2) = 0
#   player(4*i + 1) = player(4*i + 3) = 1
#   priority(4*i + 0) = priority(4*i + 1) = 1
#   priority(4*i + 2) = priority(4*i + 3) = 2   (or 0 in a min. priority game)
#
#   successors(4*i + 0) = [ 4*i + 1, 4*i + 2]
#   successors(4*i + 1) = [ 4*j + 0, 4*j + 1]   (where j == (i + 1)%N)
#   successors(4*i + 2) = [ 4*i + 3 ]
#   successors(      3) = [ 2 ]
#   successors(4*i + 3) = [ 4*i + 2, 4*i - 4 ]  (for i > 0)
#
# Example for N=3 (vertex indices are between parentheses):
#
#   __________       _________       _________       __
#             \     /         \     /         \     /
#       (0)    v(1)/    (4)    v(5)/    (8)    v(9)/
#        +     +---+     +     +---+     +     +---+
#       / \    |   |    / \    |   |    / \    |   | 
#   -->+ 1 +-->| 1 |-->+ 1 +-->| 1 |-->+ 1 +-->| 1 |----
#       \ /<   |   |    \ /<   |   |    \ /    |   |
#        +  \  +---+     +  \  +---+     +     +---+
#        |   \           |   \__________ | ____
#        |    \_________ | _____         |     \
#        v               v      \       V       \
#        +     +---+     +     +---+   \ +     +---+
#       / \ -->|   |    / \ -->|   |    / \ -->|   |
#      + 2 +   | 2 |   + 2 +   | 2 |   + 2 +   | 2 |
#       \ / <--|   |    \ / <--|   |    \ / <--|   |
#        +     +---+     +     +---+     +     +---+
#       (2)     (3)     (6)     (7)    (10)     (11)
#
# Note that edges on the top level wrap around, but vertex 3 has no wrapping
# edge to vertex 8.  This is by design: the smallest connected component is now
# {2,3} which is extended to an attractor set of {0,2,3} which causes {6,7} to
# be the next smallest component, et cetera.  (Eventually, only vertices with
# index 1 modulo 4 are won by Odd.)
#
# This design is a bit more complicated than necessary to avoid special-case
# solvers ruining the case.  (Similarly, additional priorities could be
# introduced to avoid a special-case two-priority solver being invoked.)
#

import sys

try:
    N, = map(int, sys.argv[1:])
    assert N > 0
except:
    print('Usage: ' + sys.argv[0] + ' <N>')
    sys.exit(1)

# Each vertex is described by a triple <player, priority, indices of successors>
G = []
for i in range(N):
    j = (i + 1)%N
    G.append((0, 1, [4*i + 1, 4*i + 2]))                # 4*i + 0
    G.append((1, 1, [4*j + 0, 4*j + 1]))                # 4*i + 1
    G.append((0, 2, [4*i + 3]))                         # 4*i + 2
    G.append((1, 2, [4*i + 2] + [4*i - 4]*(i > 0)))     # 4*i + 3

# Write out game in PGSolver format:
print("parity " + str(4*N - 1) + ";")
for i, (player, priority, successors) in enumerate(G):
    print(str(i) + ' ' + str(priority) + ' ' + str(player) + ' ' +
          ','.join(map(str, successors)) + ";")
