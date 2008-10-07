#!/bin/sh

printf "Checking radiff -rd... "
cp /bin/true .
radiff -rd true /bin/false | radare -vnw true
RET=`radiff -d true /bin/false`
if [ -n "${RET}" ]; then
	echo "Failed"
else
	echo "Success"
fi
rm -f true
