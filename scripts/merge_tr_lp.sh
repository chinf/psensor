#!/bin/sh

SDIR=`dirname $0`

(cd /tmp;tar -xzvf $1)

cd $SDIR/../po
for i in *po
do
    meld $i /tmp/po/psensor-$i
done

