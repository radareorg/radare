#!/bin/sh
[ -z "$2" ] && exit 0
FILE=$1
PFX=$2
nm --defined-only -B ${FILE} | grep -v ${PFX}_ | awk '{print $3}' > /tmp/list
#if [ -n "`cat /tmp/list`" ]; then
echo "=> Stripping unnecessary symbols for ${FILE}..."
objcopy --strip-symbols /tmp/list ${FILE}
strip -s ${FILE}
rm /tmp/list
#fi
