#!/bin/sh
acr -p
if [ ! $? = 0 ]; then
	echo "No 'acr' found. You can download it from:"
	echo "  http://www.lolcathost.org/b/acr-0.7.2.tar.gz"
	exit 1
else
	exit $?
fi
