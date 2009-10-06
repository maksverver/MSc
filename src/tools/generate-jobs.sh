#!/bin/sh

TESTCASES=$1
TRANSFORMS=$2
STRATEGIES=$3
TIMEOUT=$4
OUTDIR=$5

print_usage() {
	echo "generate-jobs.sh <testcases> <transformations> <strategies> <timeout> <output-dir>"
}

if [ ! -f "$TESTCASES" ] || [ ! -f "$TRANSFORMS" ] || [ ! -f "$STRATEGIES" ] || [ "$TIMEOUT" -le 0 ] || [ -z "$OUTDIR" ]
then
	print_usage
	exit 0
fi

if [ -e "$OUTDIR" ]
then
	echo "Output directory $OUTDIR exists!"
	exit 1
fi

if [ -z "`which mcrl22lps`" ]
then
	echo 'mcrl2 is not in path?'
	exit 1
fi

mkdir -p "$OUTDIR"

solver=`pwd`/main
for testcase in `cat $TESTCASES`
do
	testcase_file="testcases-cache/$testcase.raw"
	if [ ! -f "$testcase_file" ]
	then
		if [ `echo "$testcase" | cut -f1 -d-` =  random ]
		then
			# Random case given
			random_size=`echo "$testcase" | cut -f2 -d- | sed s/k/000/`
			random_prio=`echo "$testcase" | cut -f3 -d- | sed s/^lo$/5/ | sed s/^hi$/25/`
			random_seed=`echo "$testcase" | cut -f4 -d-`
			testcase_args="--input random --size $random_size --priorities $random_prio --seed $random_seed"
		else
			echo "Test case file $testcase_file does not exist!"
			exit 1
		fi
	else
		# Cached file found
		testcase_args="--input raw < `pwd`/$testcase_file"
	fi
	
	for transform in `cat $TRANSFORMS`
	do
		transform_file="transforms/$transform"
		if [ ! -f "$transform_file" ]
		then
			echo "Transform file $transform_file does not exist!"
			exit 1
		fi
		transform_args=`cat $transform_file`

		for strategy in `cat $STRATEGIES`
		do
			strategy_file="strategies/$strategy"
			if [ ! -f "$strategy_file" ]
			then
				echo "Strategy file $strategy_file does not exist!"
				exit 1
			fi
			strategy_args=`cat $strategy_file`

			job_name="$testcase--$transform--$strategy"
			job_file="$OUTDIR/${job_name}.pbs"
			command="$solver $testcase_args $transform_args $strategy_args --timeout $TIMEOUT"

			cat >"$job_file" <<EOF
#PBS -N $job_name
#PBS -l nodes=1:E5335
#PBS -W x=NACCESSPOLICY:SINGLEJOB

export PATH='$PATH'
export LD_LIBRARY_PATH='$LD_LIBRARY_PATH'
$command
EOF

		done
	done
done
