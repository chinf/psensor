#!/bin/bash

SDIR=`dirname $0`/..

cppcheck $SDIR/src  -I $SDIR/src -I $SDIR/src/lib --enable=all >cppcheck.out