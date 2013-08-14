#!/bin/bash

dir1=output-tests4b-nodecM
dir2=output-tests4b

collect() {
	total1=0
	total2=0
	count=0
	excl=0
	for file in "$dir1"/random-seed=*-size=$1-clustersize=$2-Lpredecessor:0$3.o*
	do
		if grep -q '## solution.result = success' "$file"
		then
			other=$dir2/${file#$dir1/}
			other=${other%.o*}
			lifts1=$(grep -o '## lifts.total *= *[0-9]*' "$file"     | grep -o '[0-9]*')
			lifts2=$(grep -o '## lifts.total *= *[0-9]*' "$other".o* | grep -o '[0-9]*')
			#echo "$lifts1 $lifts2"
			total1=$[$total1 + $lifts1]
			total2=$[$total2 + $lifts2]
			count=$[count + 1]
		else
			excl=$[excl + 1]
		fi
	done
	echo "== size $1  clustersize $2  $3 =="
	echo "$total1 $total2 $(dc -e "6k $total2 $total1 / p") ($count cases, $excl excluded)"
}

collect   4000    0
collect   8000    0
collect  16000    0

#collect  4000    16  # none solved
collect  4000    64

collect   4000    0 -a
collect   8000    0 -a
collect  16000    0 -a

collect  4000    16 -a
collect  4000    64 -a
