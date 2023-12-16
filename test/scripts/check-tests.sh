#!/bin/bash

set -eu
usage='echo "usage: $0 -v out-dir \"test-list\"" 1>&2; exit 1'

verbose=false

while getopts "v" OPT "$@"; do
    case "$OPT" in
    v)
        verbose=true
        ;;

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
    actuals="$(find $ACTUAL -name "$test.*" | xargs -n1 basename)"
    canonicals="$(find $CANONICAL -name "$test.*")"
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
            set +e
            if $verbose; then
                diff $canonical $actual
            else
                diff $canonical $actual >/dev/null
            fi
            status=$?
            set -e
            if [ $status -ne 0 ]; then
                echo "Test $file failed. See $actual"
                errors=$((errors + 1))
            else
                echo "Test $test passed"
            fi
        fi
    done
done

echo "Output in $ACTUAL"

if [ $errors -eq 0 ]; then
    echo "Tests PASSED"
else
    echo "Tests FAILED"
    exit 1
fi
