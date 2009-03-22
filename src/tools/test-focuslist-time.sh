#!/bin/sh

source ../../env.sh release

TMP=`mktemp`
echo ' max-size  time          # failed    # succeeded  # total lifts'
for maxsize in 1000 1200 3000 4000 5000 6000 7000 8000 9000 10000
do
	(time ./main -i random --size 10000 -l focus:0:$maxsize) >"$TMP" 2>&1
	usertime=`grep user "$TMP" | cut -f 2 | cut -c 3-`
	lifttotal=`grep Total\ lifting\ attempts "$TMP" | cut -c41-`
	liftsucc=`grep Lifting\ attempts\ succeeded "$TMP"| cut -c41-`
	liftfail=`grep Lifting\ attempts\ failed "$TMP"| cut -c41-`
	printf "%9d  %s %s %s %s\n" $maxsize "$usertime" "$liftfail" "$liftsucc" "$lifttotal"
done
rm -f "$TMP"
