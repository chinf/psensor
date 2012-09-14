#!/bin/sh

EIDS=performance,portability,information,unusedFunction,missingInclude
ROOTDIR=$srcdir/..

cppcheck $ROOTDIR/src -I$ROOTDIR/src/lib -I$ROOTDIR/src -I$ROOTDIR --enable=$EIDS --quiet --error-exitcode=1 || exit 1
