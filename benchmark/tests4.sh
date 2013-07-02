#!/usr/bin/env bash

# FIXME: change this to use start-job.sh?

# These are intended to show the effectiveness of various lifting strategies on (clustered) random games.

run_on() {
	for seed in {1..10}
	do
		for lift in -llinear:0 -llinear:1 -Lpredecessor:0 -lpredecessor:0 -lpredecessor:1 -Lpredecessor:1 -Lminmeasure:0 -Lminmeasure:1 -Lminmeasure:2 -Lmaxmeasure:0 -Lmaxmeasure:1 -Lmaxmeasure:2
		do
			for alternate in '' -a
			do
				name=random-seed=$seed-size=$1-clustersize=${2:-0}$lift$alternate run \
				../run-test.sh -i random --priorities=10 --seed=$seed $lift $alternate --size=$1 ${2:+--clustersize=$2} --timeout=600 --stats --verify
			done
		done
	done
}

source $HOME/utils/lib/torque.sh || exit 1
cd output-tests4 || exit 1

run_on  1000
run_on  4000
run_on  8000

run_on  1000    10
run_on  4000   200
run_on  8000    20
