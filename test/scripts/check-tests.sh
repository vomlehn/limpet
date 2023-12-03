#!/bin/bash

set -eu
usage='echo "usage: $0 out-dir \"test-list\"" 1>&2; exit 1'

while getopts "" OPT "$@"; do
    case "$OPT" in
    *)
        eval $usage
        ;;
    esac
done

shift $((OPTIND - 1))

if [ $# -ne 2 ]; then
    eval $usage
fi

ACTUAL="$1"
TESTS="$2"

CANONICAL=test/canonical

errors=0
for test in $TESTS; do
    actuals="$(ls $ACTUAL/$test.* | xargs -n1 basename)"
    canonicals="$(ls $CANONICAL/$test.*)"
    all="$(echo "$actuals" "$canonicals" | xargs -n1 basename | sort -u)"

    for file in $all; do
        actual=$ACTUAL/$file
        canonical=$CANONICAL/$file

        if [ ! -f $actual ]; then
            echo "Expected file $file was not produced"
            errors=$((errors + 1))
        elif [ ! -f $canonical ]; then
            echo "File $file was not expected"
            errors=$((errors + 1))
        else
            echo "Compare actual output $actual with expected output"
            set +e
            diff $canonical $actual
            status=$?
            set -e
            if [ $status -ne 0 ]; then
                errors=$((errors + 1))
            fi
        fi
    done
done

if [ $errors -eq 0 ]; then
    echo "Tests PASSED"
else
    echo "Tests FAILED"
    exit 1
fi
