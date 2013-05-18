#!/bin/sh

EIDS=performance,portability
ROOTDIR="$srcdir/.."
INCLUDE_OPTS="-q -f -I$ROOTDIR/src/lib -I$ROOTDIR/src -I$ROOTDIR -I$ROOTDIR/.."

cppcheck $ROOTDIR/src $INCLUDE_OPTS --enable=$EIDS --quiet --error-exitcode=1 || exit 1
