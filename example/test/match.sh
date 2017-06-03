#!/bin/bash

REFR="$1"
TEST="$2"

if [ -z "$REFR" -o -z "TEST" ]
then
    echo "Usage: match.sh REFERENCE_PATTERNS_FILE TEST_PATTERNS_FILE"
    exit
fi

PATTERN_DIR=/tmp/patterns

rm -rf $PATTERN_DIR
mkdir -p $PATTERN_DIR

function break_patterns
{
    local prefix="$1"
    local suffix=1
    while read -r line
    do
	if [ -z "$line" ]; then
	    suffix=$((suffix+1))
	else
	    echo "$line" |
		sed 's/\(t # \)[^*]\+/\1/' >>"${prefix}.${suffix}"
	fi
    done
}

cat "$REFR" | break_patterns "$PATTERN_DIR/refr" &
cat "$TEST" | break_patterns "$PATTERN_DIR/test" &
wait

find $PATTERN_DIR -type f \( -name 'refr.*' -and ! -name '*.md5*' \) |
    xargs md5sum >> $PATTERN_DIR/refr.md5_
find $PATTERN_DIR -type f \( -name 'test.*' -and ! -name '*.md5*' \) |
    xargs md5sum >> $PATTERN_DIR/test.md5_

sort -k 1 $PATTERN_DIR/refr.md5_ > $PATTERN_DIR/refr.md5
sort -k 1 $PATTERN_DIR/test.md5_ > $PATTERN_DIR/test.md5

rm -f $PATTERN_DIR/refr.md5_
rm -f $PATTERN_DIR/test.md5_

diff <(cut -d ' ' -f1 $PATTERN_DIR/refr.md5) <(cut -d ' ' -f1 $PATTERN_DIR/test.md5)
