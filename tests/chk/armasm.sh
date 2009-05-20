#!/bin/sh

printf "Checking rsc armasm... "
echo 'MOV R0, #33' > a.s
rsc armasm -h a.o a.s > /dev/null
if [ -n "`cat a.o | grep '21 00 A0 E3'`" ]; then
	echo Ok
else
	echo Failed
fi
rm -f a.o a.s

