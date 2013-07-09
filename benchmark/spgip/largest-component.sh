#!/bin/sh

# Usage: largest-component.sh < game-in-pgsolver-format
# Reports the largest subgame to be solved after (recursive) SCC decomposition.

$HOME/solver/bin/solver -i pgsolver -z --scc -v5 |& grep -o 'Solving subgame of size [0-9]*' | cut -d ' ' -f 5 | sort -n | tail

