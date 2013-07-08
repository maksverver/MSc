#!/bin/bash

# Takes arguments that are passed to the solver.
# If there is an input file, it must be the last argument.

DIR=$(dirname "$(readlink -f "$0")")
SOLVER=$HOME/solver/bin/solver

if [ ! -x "$SOLVER" ]
then
	echo "Solver not found: $SOLVER"
	exit
fi

eval 'input=$'$#
winners=`mktemp`
strategy=`mktemp`
trap 'rm -f "$winners" "$strategy"' EXIT
echo "## hostname = $(hostname)"
echo "## date = $(date)"
echo "## args = $*"
echo "## md5.solver = $(md5sum "$SOLVER" | cut -f1 -d' ')"
if [ -f "$input" ]
then
	echo "## arg.input = $(basename "$input")"
	echo "## md5.input = $(md5sum "$input" | cut -f1 -d' ')"
fi
${SOLVER} "$@" --winners "$winners" --strategy "$strategy" --verify 2>&1
echo "## exit = $?"
if [ -s "$winners" ]
then
	echo "## md5.winners = $(md5sum "$winners" | cut -f1 -d' ')"
fi
if [ -s "$strategy" ]
then
	echo "## md5.strategy = $(md5sum "$strategy" | cut -f1 -d' ')"
fi
