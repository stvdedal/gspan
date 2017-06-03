#!/bin/bash

DATAFILE=${1:-"../../data/Chemical_340"}
START=0.1
STEP=0.1
CMD="../gspan -i $DATAFILE -o /dev/null -l "

ngraphs=$(grep 't ' "$DATAFILE" | wc -l)

echo
echo "Data File    : $DATAFILE"
echo "Graph Count  : $ngraphs"
echo
uname -srm
uname -p
grep 'MemTotal:' /proc/meminfo
echo
echo '--------------------------------------------------'
echo -e \
     '  Supp\tUTime\tKTime\tElapsed\tMem(K)\tPatterns'
echo '--------------------------------------------------'
export TIME='%U\t%S\t%e\t%M\t'

for SUPP in `seq $START $STEP 1.0`
do
    echo -ne "  $SUPP\t"
    
    if [ ${USE_CHECK:-0} -ne 0 ]; then
	./test_by_gbolt.sh $DATAFILE $SUPP >/dev/null 2>&1
	if [ $? -ne 0 ]; then
	    echo "ERROR: test not passed"
	    exit 1
	fi
    fi

    times_file=/tmp/times.$$
    stat_file=/tmp/stat.$$
    
    \time -o "$times_file" \
	  ${CMD} -s $SUPP 2>"$stat_file"

    T=`cat "$times_file"`
    S=`cat "$stat_file"`    

    echo -ne "$T"
    echo "$S" | grep '# mined' | sed 's/# mined \([0-9]\+\).*/\1/'
done

echo
