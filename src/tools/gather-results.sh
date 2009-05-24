#!/bin/sh

DIR=$1

if [ ! -d "$DIR" ]
then
	echo "usage: gather-results <run-direcory>"
	exit 1
fi

get_time_used() {
	grep 'Time used to solve:' "$1" | sed -s 's/ \|s$//g' | cut -f2 -d:
}

get_failed() {
	grep 'Lifting attempts failed:' "$1" | sed -s 's/ //g' | cut -f2 -d:
}

get_succeeded() {
	grep 'Lifting attempts succeeded:' "$1" | sed -s 's/ //g' | cut -f2 -d:
}

get_total() {
	grep 'Total lifting attempts:' "$1" | sed -s 's/ //g' | cut -f2 -d:
}

for jobfile in "$DIR"/*.pbs
do
	job=`basename "$jobfile" .pbs`
	outfile=`echo "${DIR}/${job}".e*`
	if [ ! -f "$outfile" ]
	then
		echo "Missing output for job $job!"
		continue
	fi
	printf "%s\t%f\t%d\t%d\t%d\n" "$job" `get_time_used $outfile` `get_failed $outfile` `get_succeeded $outfile` `get_total $outfile`
done

