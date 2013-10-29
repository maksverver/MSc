#!/usr/bin/env bash

# FIXME: change this to use start-job.sh?

# These are intended to show the effectiveness of Cycle Removal before solving.

run_pgsolver() {
	for decycle in '' --decycle
	do
		for scc in '' --scc
		do
			name=pgsolver-$(basename "$1" .pgsolver)$2$decycle$scc run \
                        ../run-test.sh -i pgsolver $2 --timeout=3600 --stats --verify --reorder shuffle "$1" $decycle $scc
		done
	done
}

run_random() {
	size=--size=$1
	clustersize=${2:+--clustersize=$2}
	for decycle in '' --decycle
	do
		for scc in '' --scc
		do
			for seed in `seq 1 10`
			do
				seed=--seed=$seed
				name=random$size$clustersize$decycle$scc$seed run \
                                ../run-test.sh -i random --zielonka --timeout=3600 --verify $size $clustersize $decycle $scc $seed
			done
		done
	done
}

source $HOME/utils/lib/torque.sh || exit 1
cd output-decycle || exit 1

for file in ../spgip/{phi8,chi1000,elevator8-fair,elevator6-unfair}.pgsolver
do
	run_pgsolver "$file" "--lifting2=predecessor"
done

for file in ../spgip/{phi8,chi1000,elevator8-fair,elevator8-unfair}.pgsolver
do
	run_pgsolver "$file" "--zielonka"
done

run_random 1e7
run_random 1e7 100
