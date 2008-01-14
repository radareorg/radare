#!/bin/sh
RADARE=../src/radare
[ ! -e "${RADARE}" ] && RADARE=radare

FILE=$1
if [ ! -e "${FILE}" ]; then
	echo "Usage: CafeBabe.sh [file.class]"
	exit 1;
fi

PORT=9876

${RADARE} -L ${PORT} ${FILE} 2>&1 >/dev/null
if [ $? = 1 ]; then
	echo "error: Cannot start the radare daemon.";
	exit 1
fi

query="${RADARE} -q localhost:${PORT}"

# Go little endian
$query "e 1" >/dev/null
cafe=`$query "v/w"`
if [ "${cafe}" = "CAFE" ]; then
	printf "CAFE"
else
	echo "This is not a cafe."
	$query "q"
	exit 1
fi
$query "s 2" >/dev/null
$query "v/w"
$query "q"

exit 0
