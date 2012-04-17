#/bin/bash

SDIR=`dirname $0`

export G_SLICE=always-malloc
export G_DEBUG=gc-friendly 

DUMP_FILE=psensor_`date +"%Y_%m_%d_%H_%M_%S"`.log

for i in $SDIR/*supp
do
    SUPP="$SUPP --suppressions=$i "
done

echo starts psensor with options: $*

valgrind -v $SUPP --tool=memcheck --leak-check=full --track-origins=yes --leak-resolution=high --num-callers=20 --log-file=$DUMP_FILE $SDIR/../src/psensor $*