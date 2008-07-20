#!/bin/sh

printf "Checking flags in mov instructions ... "
STR=`echo "f LOL @ 0x8048596 && wx c7042496850408 && pd" | radare -wn malloc://7 2>/dev/null`
if [ $? = 0 ]; then
	STR=`echo "$STR" | grep LOL 2>/dev/null`
	if [ -n "${STR}" ]; then
		echo "Ok"
		exit 0
	fi
	echo "Failed"
	exit 1
else
	echo Oops. error running radare
	exit 1
fi
