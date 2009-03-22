#!/bin/sh

runs='
linear linear_B
predecessor predecessor_B predecessor_S predecessor_BS 
focus focus_B
linear_D linear_BD 
predecessor_D predecessor_BD predecessor_SD predecessor_BSD
focus_D focus_BD
'

#format=pbes
#input=../../tests/onebit_1_infrw.pbes
format=raw
input=onebit_1_infrw.raw
tmp=`mktemp`
solver='./main'

for run in $runs
do
	strat=`echo $run | cut -f1 -d_`
	if [ -z "`echo $run | grep B`" ]
	then
		strat="$strat:0"
	else
		strat="$strat:1"
	fi

	if [ -z "`echo $run | grep S`" ]
	then
		strat="$strat:0"
	else
		strat="$strat:1"
	fi

	args=""
	if [ -n "`echo $run | grep D`" ]
	then
		args="$args --scc"
	fi

	$solver --input "$format" --strategy $strat $args < "$input" 2> "$tmp"
	lift_total=`grep 'Total lifting attempts:' "$tmp" | cut -c40-`
	lift_success=`grep 'Lifting attempts succeeded:' "$tmp" | cut -c40-`
	lift_failed=`grep  'Lifting attempts failed:' "$tmp" | cut -c40-`
	time_used=`grep 'Time used to solve' "$tmp" | cut -c35-`
	printf "%-15s  %'14d  %'14d  %'14d  %9s\n" \
		"`echo $run | tr _ \ `" $lift_failed $lift_success $lift_total "$time_used"
done

rm -f "$tmp"
