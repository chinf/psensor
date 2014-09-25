#!/bin/sh

SCRIPT_DIR=`dirname $0`
cd $SCRIPT_DIR/..

./configure --prefix=/tmp CC=cgcc CPPFLAGS="-Wsparse-all -DCURL_DISABLE_TYPECHECK -Wno-old-initializer"
make clean all check install distcheck 3>&1 1>&2 2>&3 | grep -v "^/usr/include" | tee /tmp/err 

cat /tmp/err

echo Number of warnings: `cat /tmp/err|wc -l`
