#!/bin/sh

SDIR=`dirname $0`

cd $SDIR/..

./configure --build=i686-pc-linux-gnu "CFLAGS=-mx32" "CXXFLAGS=-mx32" "LDFLAGS=-mx32"

make clean all
