#!/bin/sh

printf "Checking rax bin2hex/hex2bin conversion... "
rax -s "`cat /bin/true | rax -`" > /tmp/true.new
A="`md5sum /bin/true | cut -d ' ' -f 1`"
B="`md5sum /tmp/true.new | cut -d ' ' -f 1`"
if [ "$A" = "$B" ]; then
	echo Ok
else
	echo Oops
fi
rm -f /tmp/true.new
