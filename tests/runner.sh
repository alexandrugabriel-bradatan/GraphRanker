#! /bin/bash

EXEC_PATH=$1
TEST_DIR="${2:-$PROJECT_ROOT/tests/open_tests}"

i=0
for file in $TEST_DIR/input_*; do
	echo ">> Testing $file"
	$EXEC_PATH < $file > test_out || exit 1
	diff "$TEST_DIR/output_$i" test_out || exit 1
done
exit 0
