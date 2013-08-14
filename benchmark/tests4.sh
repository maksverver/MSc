#!/usr/bin/env bash

# FIXME: change this to use start-job.sh?

# These are intended to show the effectiveness of various lifting strategies on (clustered) random games.

run_on() {
	for seed in {1..30}
	do
		for lift in -llinear:0 -llinear:1 -Lpredecessor:0 -lpredecessor:0 -lpredecessor:1 -Lpredecessor:1 -Lminmeasure:0 -Lminmeasure:1 -Lminmeasure:2 -Lmaxmeasure:0 -Lmaxmeasure:1 -Lmaxmeasure:2 -Lmaxstep:0 -Lmaxstep:1 -Lmaxstep:2
		do
			for alternate in '' -a
			do
				name=random-seed=$seed-size=$1-clustersize=${2:-0}$lift$alternate run \
				../run-test.sh -i random --priorities=10 --seed=$seed $lift $alternate --size=$1 ${2:+--clustersize=$2} --maxlifts=1e9 --stats --verify
			done
		done
	done
}

source $HOME/utils/lib/torque.sh || exit 1
cd output-tests4b || exit 1

run_on   4000
run_on   8000
run_on  16000

run_on  4000    16
run_on  4000    64
