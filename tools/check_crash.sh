#!/usr/bin/env sh

IOTJS_BIN=$1

trap ':' SEGV

for file in $(ls ./test/crash/*.js)
do
  echo "running $file"
  $IOTJS_BIN $file > /dev/null 2>&1
done

IOTJS_DUMP_FILES=$(ls /tmp/iotjs.*)
for file in $IOTJS_DUMP_FILES
do
  echo "iotjs dump files are not cleanup" $file
  exit 1
done
echo "successfully test done"
