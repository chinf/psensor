#!/bin/sh

SCRIPT_DIR=`dirname $0`
cd $SCRIPT_DIR/..

./configure --prefix=/tmp CC=cgcc CPPFLAGS="-Wsparse-all -DCURL_DISABLE_TYPECHECK -Wno-old-initializer" || exit 1
make clean || exit 1
make all check install distcheck 3>&1 1>&2 2>&3 | grep -v "^/usr/include" | tee /tmp/err 

cat /tmp/err

echo Number of warnings: `cat /tmp/err|wc -l`

export G_DEBUG=fatal_warnings

$SCRIPT_DIR/../src/psensor
