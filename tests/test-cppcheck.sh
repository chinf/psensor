#!/bin/sh

EIDS=performance,portability,information,unusedFunction,missingInclude
ROOTDIR="$srcdir/.."
INCLUDE_OPTS="-I$ROOTDIR/src/lib -I$ROOTDIR/src -I$ROOTDIR -I$ROOTDIR/.."

cppcheck $ROOTDIR/src $INCLUDE_OPTS --enable=$EIDS --quiet --check-config --error-exitcode=1 || exit 1
cppcheck $ROOTDIR/src $INCLUDE_OPTS --enable=$EIDS --quiet --error-exitcode=1 || exit 1
