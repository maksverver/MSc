#!/usr/bin/env python

# Generates a Jurdzinsky game in PGSolver format with l levels and b blocks.

# Structure:
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
# Expected results: odd player wins from the odd level (first 2*B + 1 vertices)
# and even player wins from the even levels (the rest of the vertices). Lifting
# the vertices with priority 1 to top should take (b + 1)**l lifts.

import sys

try:
    (l, b) = map(int, sys.argv[1:])
    assert l > 0
    assert b > 0
except:
    print('Usage: ' + sys.argv[0] + ' <l> <b>')
    sys.exit(1)

N = (2*b + 1) + (3*b + 1)*(l - 1)   # number of vertices
V = [None]*N                        # per vertex: priority, player, edges

# Generate odd level `0'
for i in range(2*b + 1):    V[i] = (i%2, i%2, [])
for i in range(2*b):        V[i+1][2].append(i+0)
for i in range(2*b):        V[i+0][2].append(i+1)

for k in range(1, l):
    # Generate even level `k'
    j = (2*b + 1) + (3*b + 1)*(k - 1)
    for i in range(b + 1): V[j + 3*i + 0] = (2*k,     1, [])
    for i in range(b):     V[j + 3*i + 1] = (2*k + 1, 0, [])
    for i in range(b):     V[j + 3*i + 2] = (2*k,     0, [])
    for i in range(b):
        for (v,w) in [(0,1),(1,2),(0,2),(2,0),(2,3),(3,2)]:
            V[j + 3*i + v][2].append(j + 3*i + w)

print('parity ' + str(N - 1) + ';')
for (i, (priority, player, edges)) in enumerate(V):
    print i, priority, player, ','.join(map(str, edges)) + ';'
