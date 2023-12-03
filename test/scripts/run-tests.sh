#!/bin/bash

set -eu
usage='echo "usage: $0 out-dir bin-dir \"test-list\"" 1>&2; exit 1'

while getopts "" OPT "$@"; do
    case "$OPT" in
    *)
        eval $usage
        ;;
    esac
done

shift $((OPTIND - 1))

if [ $# -ne 3 ]; then
    eval $usage
fi

ACTUAL="$1"
BIN="$2"
TESTS="$3"

CANONICAL=test/canonical

SEP=""
declare -a tests
tests=( $TESTS )

for test in "${tests[@]}"; do
    printf "$SEP"
    TEST_NAME="$(echo "$test" | sed 's/^.*://')"
    TEST_CMD="$(echo "$test" |
        sed -e "s/^[^:]*$/$BIN\/&/" -e "s/^\(.*\):/\1 $BIN\//g")"
    running_string="Running test $TEST_NAME"
    echo "$running_string"
    eval "$TEST_CMD | parse-test.sh $TEST_NAME $ACTUAL"
    SEP=""
done
