#!/bin/bash

INPUT=$1
SUPP=$2

if [ -z "$SUPP" ]
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
TMP=/tmp/

####################################
# first run gbolt
rm -f ${TMP}/gbolt_out*
time ${GBOLT} \
     -support ${SUPP} \
     -input_file ${INPUT} \
     -output_file /tmp/gbolt_out \
     -pattern
cat /tmp/gbolt_out* > ref_output

####################################
# run test program
rm -f test_output
time ../gspan --minsupp ${SUPP} --legacy --input $INPUT --output test_output

./match.sh ref_output test_output
if [ $? -eq 0 ]
then
    echo
    echo "Good, test passed"
    echo
else
    echo
    echo "Bad, test not passed"
    echo
fi
