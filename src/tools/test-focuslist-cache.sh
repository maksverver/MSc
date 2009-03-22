#!/bin/sh

source ../../env.sh release

TMP=`mktemp`
echo ' max-size   total refs   L1 misses   L2 misses  L1 miss rate  L2 miss rate'
for maxsize in 1000 1200 3000 4000 5000 6000 7000 8000 9000 10000
do
	(valgrind --tool=cachegrind ./main -i random --size 10000 -l focus:0:$maxsize) >"$TMP" 2>&1
	rm cachegrind.out.*
	totref=`grep 'D   refs:'      "$TMP" | cut -c 26-36`
	L1miss=`grep 'D1  misses:'    "$TMP" | cut -c 26-36`
	L2miss=`grep 'L2d misses:'    "$TMP" | cut -c 26-36`
	L1rate=`grep 'D1  miss rate:' "$TMP" | cut -c 32-36`
	L2rate=`grep 'L2d miss rate:' "$TMP" | cut -c 32-36`
	printf "%9d  %s %s %s        %s%%        %s%%\n" \
		 $maxsize "$totref" "$L1miss" "$L2miss" "$L1rate" "$L2rate"
done
rm -f "$TMP"
