#!/bin/sh
#
# A simple shell script to parse limpet output and put it into files in
# a given directory. Input comes from stdin.

set -eu
usage='echo "usage: $0 test-name out-dir" 1>&2; exit 1'

not_from_limpet() {
    line="$1"
    echo "Input is not from limpet: $line" 1>&2
    exit 1
}

read_until_match() {
    eof_ok=$1
    out="$2"
    match="$3"

    while read line; do
        if expr "$line" : "$match" >/dev/null; then
            echo "$line"
            return 0
        else
            echo "$line" >>$out
        fi
    done

    if $eof_ok; then
        echo 0
    else
        echo "Unexpected end of file" 1>&2
        exit 1
    fi
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

# Scan for the start of output
dummy="$(read_until_match false /dev/null "Running limpet")"

while read line; do
    if expr "$line" : "> Ran " >/dev/null; then
        echo "$line" >$OUT_DIR/$FILE_NAME.summary
        break
    elif expr "$line" : "> Log for " >/dev/null; then
        test_name="$(echo "$line" | sed 's/> Log for //')"
        output=$OUT_DIR/$FILE_NAME.$test_name
        echo "$line" >$output
    else
        not_from_limpet "$line"
    fi

    result="$(read_until_match false "$output" "---")"
    echo "$result" >>$output
done
