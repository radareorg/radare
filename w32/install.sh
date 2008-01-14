#!/bin/bash

if [ -z "${HOMEPATH}" ]; then
	echo "Is this a windows box with cygwin installed?"
	exit 1
fi

echo
echo "****************************************"
echo "**     RADARE 0.9.1 INSTALLATION      **"
echo "****************************************"
echo

# Create Directory
echo "default install to (/cygdrive/c/radare/ aka C:\Radare): "
echo
echo "press intro to start installation"
read 

PREFIX="/cygdrive/c/radare"
mkdir -p "${PREFIX}"

echo "Copying executables..."
cd bin
for a in * ; do
	echo " + ${PREFIX}/$a"
	cp -f $a "${PREFIX}"
	chmod +x "${PREFIX}/$a"
done
cd ..

echo "Adding LNK to SendTo..."
cp -f Radare.lnk "${HOMEPATH}\\SendTo\\"
echo "${HOMEPATH}\\SendTo\\Radare.lnk"

echo "Copying manpages..."
mkdir -p "${PREFIX}/man"
rm -rf ${PREFIX}/man/*
cd man
for a in *.1 ; do
	cp -f $a "${PREFIX}/man"
	#printf 'c:/cygwin/bin/man.exe ' > ${PREFIX}/man/$a.bat
	#printf "./radare.1" >> ${PREFIX}/man/$a.bat
	echo " + ${PREFIX}/man/$a"
done
cd ..

echo "Copying documentation..."
mkdir -p "${PREFIX}/doc"
for a in doc/* doc/xtra/* ; do
	echo " + ${PREFIX}/$a.txt"
	cp -f $a "${PREFIX}/$a.txt" 2> /dev/null
done

echo "Generating batman pages..."
cp gen.bak ${PREFIX}/man/GenBatMan.bat
cd "${PREFIX}/man"
GenBatMan.bat
cd -

echo "Copying libexec..."
cp -rf libexec "${PREFIX}/"

echo "Copying raweb..."
cp -rf raweb "${PREFIX}/"

echo
echo "Installation done."

echo
echo "***********************************************"
echo "**    REMEMBER TO SETUP THE PATH VARIABLE    **"
echo "**    USE echo 	REMEMBER TO SETUP THE PATH VARIABLE    **"
echo "***********************************************"
echo
echo "Press intro to close"
read
