#!/bin/sh

printf "Checking debugger... "

FUCK=`printf '!echo FUCK\nq\n' | radare -nvd ls 2>/dev/null`
if [ -n "${FUCK}" ]; then
	echo Ok
else
	echo Failed
fi
