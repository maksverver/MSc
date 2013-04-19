#!/usr/bin/env python

# Generates a Jurdzinski game in PGSolver format with l levels and b blocks.
# The resulting game has (2b + 1) + (3b + 1)(l - 1) vertices and 2l priorities.

# Usage:
#   generate-jurdzinski-game.py <levels> [<blocks>]
# If <blocks> is omitted, it defaults to levels - 1.

# Expected result is that player odd wins only on the odd level (vertices
# numbered from 0 to 2b, inclusive) and lifting each of these vertices to top
# requires (b + 1)**l lifts.

# Game structure:
#
#                          0        1          2         3         2b-1      2b
# One odd level:          <0>  <=>  [1]  <=>  [0]  <=>  [1] <=..=> [1] <=>  <0>
#                                    ^                   ^          ^
#                                    |                   |          |
#                                    v                   v          v
# (l-1) even levels:    [2k]  <=>  <2k> <=> [2k]  <=>  <2k> <=..=> <2k> <=> [2k]
# where k in [1..l)       \          ^        \          ^
#                          > (2k+1) /          > (2k+1) /
#
#
# (Note: since this is in PGSolver format, high numbers are used for high
#  priorities, which is the opposite of the convention used in Jurdzinski's
#  paper.)
#
# (Note: PGSolver includes a `jurdzinskigame` generator too, but the games it
#  generates aren't exactly like Jurdzinski described.  I don't know how or why
#  they are different, although they seem equally hard to solve.)

import sys

try:
    try:
      l, b = map(int, sys.argv[1:])
    except:
      l, = map(int, sys.argv[1:])
      b = l - 1
    assert l > 0
    assert b > 0
except:
    print('Usage: ' + sys.argv[0] + ' <levels> [<blocks>]')
    sys.exit(1)

N = (2*b + 1) + (3*b + 1)*(l - 1)   # number of vertices
V = [None]*N                        # per vertex: priority, player, edges

def uni_edge(i, j):  # add a unidirectional edge
    assert i != j and j not in V[i][2]
    V[i][2].append(j)

def bi_edge(i, j):  # add a bidirectional edge
    uni_edge(i, j)
    uni_edge(j, i)

# Generate odd level `0'
for i in range(2*b + 1): V[i] = (i%2, i%2, [])
for i in range(2*b):     bi_edge(i, i+1)

# Generate even levels
begin, end = 0, 2*b + 1
for k in range(1, l):

    # Generate even level `k'
    begin, end = end, end + (3*b + 1)

    for i in range(begin + 0, end, 3): V[i] = (2*k + 0, 1, [])
    for i in range(begin + 1, end, 3): V[i] = (2*k + 1, 0, [])
    for i in range(begin + 2, end, 3): V[i] = (2*k + 0, 0, [])

    for i in range(begin, end - 1, 3):
        uni_edge(i + 0, i + 1)
        uni_edge(i + 1, i + 2)
        bi_edge (i + 0, i + 2)
        bi_edge (i + 2, i + 3)

    # Connect level `k' to level `0'
    for j in range(b):
        bi_edge(begin + 3*j + 2, 2*j + 1)

assert N == end

print('parity ' + str(N - 1) + ';')
for (i, (priority, player, edges)) in enumerate(V):
    print i, priority, player, ','.join(map(str, edges)) + ';'
