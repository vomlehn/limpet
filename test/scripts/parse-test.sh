#!/bin/sh
#
# A simple shell script to parse limpet output and put it into files in
# a given directory. Input comes from stdin.

set -eu
usage='echo "usage: $0 test-name out-dir" 1>&2; exit 1'

# We expect the line to start with "> " as output from the limpet test
# test framework. If not, we don't know what to do with it
check_for_unexpected() {
    line="$1"

    if expr "$line" : "> " >/dev/null; then
        return 0
    fi

    echo "Input is not from limpet: $line" 1>&2
    exit 1
}

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

FILE_NAME="$1"
OUT_DIR="$2"

# Scan the file
state=scanning_for_start

while read line; do
    case $state in
    scanning_for_start)
        if expr "$line" : "Running limpet" >/dev/null; then
            state=scanning_for_name
        fi
        ;;

    scanning_for_name)
        if expr "$line" : "> Log for " >/dev/null; then
            test_name="$(echo "$line" | sed 's/> Log for //')"
            output=$OUT_DIR/$FILE_NAME.$test_name
            state=scanning_for_log
        elif expr "$line" : "> Ran " >/dev/null; then
            echo "$line" >$OUT_DIR/$FILE_NAME.summary
            state=scanning_for_end
        else
            check_for_unexpected "$line"
        fi
        ;;

    scanning_for_log)
        if expr "$line" : "> vv*\$" >/dev/null; then
            echo "$line" >>"$output"
            state=scanning_log
        else
            check_for_unexpected "$line"
        fi
        ;;

    scanning_log)
        if expr "$line" : "> ^^*\$" >/dev/null; then
            state=scanning_for_status
        fi
        echo "$line" >>"$output"
        ;;

    scanning_for_status)
        if expr "$line" : "^> Test complete: " >/dev/null; then
            echo "$line" >>"$output"
            state=scanning_for_sep
        else
            check_for_unexpected "$line"
        fi
        ;;

    scanning_for_sep)
        if expr "$line" : "---\$" >/dev/null; then
            state=scanning_for_name
        else
            check_for_unexpected "$line"
        fi
        ;;

    scanning_for_end)
        check_for_unexpected "$line"
        ;;
    esac
done

if [ $state != "scanning_for_end" ]; then
    echo "End state was '$state' not 'scanning_for_end'" 1>&2
    exit 1
fi
