#!/bin/sh

DATADIR=testcases
CACHEDIR=testcases-cache
TOOL=./main
CASES=$@

if [ ! -d "$DATADIR" ]
then
	echo "Not a directory: $DATADIR"
	exit 1
fi

if [ ! -d "$CACHEDIR" ]
then
	echo "Not a directory: $CACHEDIR"
	exit 1
fi

if [ -z "$CASES" ]
then
	CASES=`ls "$DATADIR"`
fi

for case in $CASES
do
	if [ ! -f "$DATADIR/$case" ]
	then
		echo "No such file: $DATADIR/$case"
		exit 1
	else
		if [ -n "`echo $case | grep .pgsolver`" ]
		then
			format=pgsolver
			newcase=`basename $case .pgsolver`.raw
		elif [ -n "`echo $case | grep .pbes`" ]
		then
			format=pbes
			newcase=`basename $case .pbes`.raw
		else
			echo "Unknown file format ($case); skipping!"
			continue
		fi
		if [ ! "$CACHEDIR/$newcase" -nt "$DATADIR/$case" ]
		then
			echo "Regenerating $newcase..."
			./main --input $format < "$DATADIR/$case" --raw "$CACHEDIR/$newcase"
		fi
	fi
done

