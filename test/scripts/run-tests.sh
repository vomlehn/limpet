#!/bin/bash

set -eu

source "$(dirname "$0")/test-common.sh"

usage='echo "usage: $0 version out-dir bin-dir \"test-list\"" 1>&2; exit 1'

while getopts "" OPT "$@"; do
    case "$OPT" in
    *)
        eval $usage
        ;;
    esac
done

shift $((OPTIND - 1))

case $# in
3)
    VERSION="$1"
    ACTUAL="$2"
    BIN="$3"
    ;;

4)
    VERSION="$1"
    ACTUAL="$2"
    BIN="$3"
    RUNLIST="$4"
    ;;

*)
    eval $usage
    ;;
esac

cleanup() {
    rm -f $tmpfile
}

CANONICAL=test/canonical

trap "set -x; cleanup; exit 1" TERM HUP
tmpfile="$(mktemp)"

# Write the test info into a file. It would be nice to pipe from
# print-testinfo.sh but that won't propagate tests back to this shell
declare -a tests
print-testinfo.sh $VERSION "$RUNLIST" >$tmpfile
while read test; do
    tests+=("$test")
done <$tmpfile
rm $tmpfile

SEP=""

for test in "${tests[@]}"; do
    printf "$SEP"
    TEST_NAME="$(echo "$test" | sed 's/^.*://')"
    TEST_CMD="$(echo "$test" |
        sed -e "s/^[^:]*$/$BIN\/&/" \
        -e "s/^\(.*\):/\1 $BIN\//g" \
        -e 's/:/ /g')"
    running_string="Test comand $TEST_CMD"
    echo "$running_string"
    eval "$TEST_CMD | parse-test.sh $TEST_NAME $ACTUAL"
    SEP=""
done
