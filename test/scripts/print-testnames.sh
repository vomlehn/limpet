#!/bin/bash
#
# Print the test case information, one per line
set -eu
usage='echo "usage: $0 version [ runlist ]" 1>&2; exit 1'

while getopts "" OPT "$@"; do
    case "$OPT" in
    *)
        eval $usage
        ;;
    esac
done

shift $((OPTIND - 1))

case $# in
1)
    VERSION="$1"
    ;;

2)
    VERSION="$1"
    RUNLIST="$2"
    ;;

*)
    eval $usage
    ;;
esac

if [ -v RUNLIST ]; then
    print-testinfo.sh "$VERSION" "$RUNLIST"
else
    print-testinfo.sh "$VERSION"
fi | sed 's/.*://'