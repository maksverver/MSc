#!/usr/bin/env python3

# Tool to generate a parity game that consists of a large cycle of nodes of
# alternating players.  Nodes in the cycle consist of two vertices, each with
# two edges to the next two vertices, to defeat cycle removal tools.

# The reorder() function reorders vertex indices such that forward and backward
# edges alternate in the output, resulting in a graph with 50% forward edge
# ratio that takes many passes of linear-lifting SPM to solve.  This is useful
# to test the effectiveness of --reorder bfs/dfs.

import sys

vertices   = 100
priorities =  10

if len(sys.argv) > 1: vertices   = int(sys.argv[1])
if len(sys.argv) > 2: priorities = int(sys.argv[2])

assert vertices%2 == 0

def reorder(i):
    assert 0 <= i < vertices
    return (i//4)*2 + (i%2) + (i//2%2)*(vertices//2)
    #return i if i < vertices//2 else 3*vertices//2 - 1 - i

game = [ ( reorder(i), (i//2)%priorities, (i//2)%2,
           [reorder(((i&-2) + 2)%vertices), reorder(((i&-2) + 3)%vertices)]
         ) for i in range(vertices) ]

print('parity {};'.format(len(game) - 1))
for index, priority, player, edges in sorted(game):
    edges = ",".join(str(edge) for edge in edges)
    print("{} {} {} {};".format(index, priority, player, edges))
