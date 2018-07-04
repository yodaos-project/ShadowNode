#!/usr/bin/env sh

IOTJS_BIN=$1

trap ':' SEGV

for file in $(ls ./test/crash/*.js)
do
  echo "running $file"
  pids="$(sh -c "echo \$\$; exec $IOTJS_BIN $file > /dev/null 2>&1") $pids"
done
echo "successfully test done"
