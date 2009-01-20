#!/bin/sh
[ -z "$2" ] && exit 0
FILE=$1
PFX=$2
LIST=`mktemp /tmp/$FILE.XXXX`

nm --defined-only -B ${FILE} | grep -v ${PFX}_ | awk '{print $3}' > ${LIST}
#if [ -n "`cat /tmp/list`" ]; then
echo "=> Stripping unnecessary symbols for ${FILE}..."
objcopy --strip-symbols ${LIST} ${FILE}
strip -s ${FILE}
#fi

rm -f ${LIST}
