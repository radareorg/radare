#!/bin/sh

printf "Checking 0x7f at first byte.. "
FUCK=`echo '? 0x7f == [1:0x0] && ?? q && !echo FUCK' | radare -nv /bin/true`
if [ -n "${FUCK}" ]; then
	echo "Oops"
else
	echo "Ok"
fi
