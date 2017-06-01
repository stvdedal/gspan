#!/bin/bash

INPUT=$1
SUPP=$2

if [ -z $SUPP ]
then
    echo "Usage: $0 input_file support"
    exit 1
fi

echo
echo "---------------------"
echo "INPUT   : " ${INPUT}
echo "SUPPORT : " ${SUPP}
echo "---------------------"
echo

GBOLT=~/thirdparty/DataMining-gSpan/build/gbolt

####################################
# first run gbolt
# gbolt recompiled with
# omp_set_num_threads(1); in main()

# clear olds
rm -f /tmp/gbolt_out*
# run gbolt
time ${GBOLT} -support ${SUPP} -input_file ${INPUT} -output_file /tmp/gbolt_out -pattern
# collect outputs to single file
cat /tmp/gbolt_out* > ref_output

####################################
# run test program
#
time ../gspan -minsupport ${SUPP} -tgf <$INPUT >test_output

ls -l ref_output test_output
md5sum ref_output test_output

echo
if cmp --quiet ref_output test_output
then
    echo "PASS"
else
    echo "NOT PASS"    
fi
echo
