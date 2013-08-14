#!/usr/bin/env bash

# FIXME: change this to use start-job.sh?

run_on() {
	for seed in {1..30}
	do
		for lift in -Lpredecessor:0
		do
			for alternate in '' -a
			do
				if grep '## solution.result = success' ../output-tests4b/random-seed=$seed-size=$1-clustersize=${2:-0}$lift$alternate.o*
				then
					name=random-seed=$seed-size=$1-clustersize=${2:-0}$lift$alternate run \
					../run-test.sh -i random --priorities=10 --seed=$seed $lift $alternate --size=$1 ${2:+--clustersize=$2} --maxlifts=1e10 --stats --verify
				fi
			done
		done
	done
}

source $HOME/utils/lib/torque.sh || exit 1
cd output-tests4b-nodecM || exit 1

run_on   4000
run_on   8000
run_on  16000

run_on  4000    16
run_on  4000    64
