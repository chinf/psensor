#!/bin/sh

SMATCH_DIR=$HOME/soft/smatch

SCRIPT_DIR=`dirname $0`

cd $SCRIPT_DIR/..
make clean
make CHECK="$SMATCH_DIR/smatch --full-path" CC=$SMATCH_DIR/cgcc | tee warns.txt