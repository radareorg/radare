# Example custom makefile for creating .deb without using dpkg tools.
#
# Author: Tim Wegener <twegener@madabar.com>
#
# This requires deb_hand.mak

# Sources:
# SOURCE_DIR - directory containing files to be packaged
# ICON_SOURCE - 26x26 icon file for maemo
# description.txt - description with summary on first line
# preinst, postinst, prerm, postrm - optional control shell scripts

# These fields are used to build the control file.
PACKAGE = radare
VERSION = 0.9.3
ARCH = armel
SECTION = user/shell
PRIORITY = optional
MAINTAINER = pancake <pancake@youterm.com>
# todo: For python2.4 it also needs python2.4-elementtree and python2.4-sqlite
DEPENDS =
# todo: Recommends
PACKAGE_DIR=${PWD}

#SOURCE_DIR = .
ICON_SOURCE =
#ICON_SOURCE =${SOURCE_DIR}/maemo_deb/exaile26.png
PRG=../../


${PACKAGE_DIR}/data: ${SOURCE_DIR}
	rm -rf $@
	mkdir $@
	mkdir -p $@/usr/bin $@/usr/share/doc/radare $@/usr/libexec/radare $@/usr/libexec/gradare $@/usr/share/radare $@/usr/lib
	cp ../../src/radare $@/usr/bin
	cp ${PRG}/src/bug $@/usr/bin/bug
	cp ${PRG}/src/rdb/rdbdiff $@/usr/bin/rdbdiff
	cp ${PRG}/src/radare $@/usr/bin/radare
	cp ${PRG}/src/bindiff $@/usr/bin/bindiff
	cp ${PRG}/src/rabin/rabin $@/usr/bin/rabin
	cp ${PRG}/src/rasm/rasm $@/usr/bin/rasm
	cp ${PRG}/rasc/rasc $@/usr/bin/rasc
	cp ${PRG}/src/arch/arm/aasm/armasm $@/usr/bin/armasm
	cp ${PRG}/src/rsc $@/usr/bin/rsc
	cp ${PRG}/src/rfile $@/usr/bin/rfile
	cp ${PRG}/src/hasher/hasher $@/usr/bin/hasher
	cp ${PRG}/src/xc $@/usr/bin/xc
	cp ${PRG}/src/javasm $@/usr/bin/javasm
	cp ${PRG}/src/xrefs $@/usr/bin/xrefs
	cp ${PRG}/gui/gradare $@/usr/bin/gradare
	cp /usr/lib/libvte.so.9 $@/usr/lib
	cp -rf ${PRG}/gui/grsc/* $@/usr/libexec/gradare
	cp -rf ${PRG}/libexec/* $@/usr/libexec/radare
	-cp -rf ${PRG}/magic/* $@/usr/share/radare/magic/
	-cp -rf ${PRG}/doc/* $@/usr/share/doc/radare/

include deb_hand.mak