#!/usr/bin/env bash

# FIXME: change this to use start-job.sh?

# These are intended to show the effectiveness of various lifting strategies on structured test cases from Solving Parity Games in Practice.

run_on() {
	for lift in -llinear:0 -llinear:1 -Lpredecessor:0 -Lpredecessor:1 -Lminmeasure:0 -Lminmeasure:1 -Lminmeasure:2 -Lmaxmeasure:0 -Lmaxmeasure:1 -Lmaxmeasure:2 -Lmaxstep:0 -Lmaxstep:1 -Lmaxstep:2
	do
		for alternate in '' -a
		do
			name=pgsolver-$(basename "$1" .pgsolver)$lift$alternate
			if test -f "$name".o*
			then
				true  # echo "$name exists!"
			else
				echo "starting $name"
				name=$name run ../run-test.sh -i pgsolver $lift $alternate --timeout=3600 --stats --verify "$1" --reorder shuffle
				sleep 1
			fi
		done
	done
}

source $HOME/utils/lib/torque.sh || exit 1
cd output-spm-spgip-shuffled || exit 1

for file in ../spgip/*.pgsolver
do
	run_on "$file"
done
