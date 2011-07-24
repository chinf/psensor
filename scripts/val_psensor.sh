#/bin/sh

export G_SLICE=always-malloc
export G_DEBUG=gc-friendly 

DUMP_FILE=psensor_`date +"%Y_%m_%d_%H_%M_%S"`.log

valgrind --tool=memcheck --leak-check=full --leak-resolution=high --num-callers=20 --log-file=$DUMP_FILE `dirname $0`/../src/psensor