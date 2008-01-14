#!/bin/sh
#
# make a debian package of radare
#

# configuration
PKGNAME="radare"
#VERSION="`src/radare -V | cut -d ' ' -f 2`"
VERSION="0.9.2b"
#`src/radare -V | cut -d ' ' -f 1`
ARCH="i386"
PKGSIZE="44684"
REPO="pool/stable/main/"

# workage
MAEMO=`gcc 2>&1 | grep arm`
TMPDIR=`mktemp /tmp/XXXXXXXX`
PRG=${PWD}
rm -f ${TMPDIR}
mkdir -p ${TMPDIR} || exit 1
cd ${TMPDIR} || exit 1
mkdir -p DEBIAN usr/lib usr/bin usr/libexec/radare/ usr/share/radare/magic 2>&1 > /dev/null
if [ -n "${MAEMO}" ]; then
	ARCH="armel"
	REPO="pool/mistral/user/"
	mkdir -p lib
fi

if [ -z "${VERSION}" ]; then
	echo "No version?"
	exit 1
fi

# install
cp ${PRG}/src/bug usr/bin/bug
cp ${PRG}/src/rdb/rdbdiff usr/bin/rdbdiff
cp ${PRG}/src/radare usr/bin/radare
cp ${PRG}/src/bindiff usr/bin/bindiff
cp ${PRG}/src/rabin/rabin usr/bin/rabin
cp ${PRG}/src/rasm/rasm usr/bin/rasm
cp ${PRG}/rasc/rasc usr/bin/rasc
cp ${PRG}/src/arch/arm/aasm/armasm /usr/bin/armasm
cp ${PRG}/src/rsc usr/bin/rsc
cp ${PRG}/src/rfile usr/bin/rfile
cp ${PRG}/src/hasher/hasher usr/bin/hasher
cp ${PRG}/src/xc usr/bin/xc
cp ${PRG}/src/javasm usr/bin/javasm
cp ${PRG}/src/xrefs usr/bin/xrefs
#cp ${PRG}/libps2fd/libps2fd.so usr/lib/libps2fd.so
#if [ -n "${MAEMO}" ]; then
#	cp /lib/libreadline.so.4 lib/
#	cp /usr/lib/libopcodes-2.16.91.so lib/
#	cp /usr/lib/libbfd-2.16.91.so lib/
#	cp /usr/bin/objdump usr/bin
#fi
chmod 0755 usr/bin/*
cd ${PRG}/libexec/
FILES=`make files`
for a in ${FILES}; do
	cp ${a} ${TMPDIR}/usr/libexec/radare
done
cd ${TMPDIR}
cp ${PRG}/magic/* usr/share/radare/magic/

print_control() {
# control
cat > DEBIAN/control << EOF
Package: ${PKGNAME}
Version: ${VERSION}
Architecture: ${ARCH}
Maintainer: pancake <pancake@youterm.com>
Filename: ${REPO}/radare_${VERSION}_${ARCH}.deb
Size: ${PKGSIZE}
Installed-size: ${SIZE}
Description: command line hex editor
 radare is a command line hexadecimal editor with advanced features like
 hashing, overlap of structures, visual mode, different view modes, scripts..
EOF
}

SIZE=`du -sb .|column -t |awk '{print $1}'`
print_control
echo "dpkg-deb -b . ../radare_${VERSION}_${ARCH}.deb" | fakeroot

SIZE=`du -sb . | column -t | awk '{ print int($1/1024); }'`
#PKGSIZE=`ls -l /tmp/radare_${VERSION}_${ARCH}.deb |column -t | awk '{ print $5; }'`
PKGSIZE=`du -b /tmp/radare_${VERSION}_${ARCH}.deb |column -t | awk '{ print $1; }'`
print_control

echo "--"
cat DEBIAN/control
echo "--"

echo "dpkg-deb -b . ../radare_${VERSION}_${ARCH}.deb 2>/dev/null" | fakeroot

cd ..
rm -rf ${TMPDIR}
echo "Package done: ($PWD)"
printf "  - "
echo $PWD
ls radare_* 2>/dev/null
exit 0
