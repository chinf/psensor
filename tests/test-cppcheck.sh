#!/bin/sh

EIDS=performance,portability,information,unusedFunction,missingInclude

cppcheck ../src -I../src/lib -I../src -I.. --enable=$EIDS --quiet --error-exitcode=1 || exit 1