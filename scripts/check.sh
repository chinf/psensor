#!/bin/sh

SCRIPT_DIR=`dirname $0`
cd $SCRIPT_DIR/..

./configure CC=cgcc CPPFLAGS="-DCURL_DISABLE_TYPECHECK -Wno-old-initializer"
make clean all check 3>&1 1>&2 2>&3 | tee /tmp/err

cat /tmp/err

echo Number of warnings: `cat /tmp/err|wc -l`
