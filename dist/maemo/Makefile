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
VERSION = 1.0-beta
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
PRG=../../build/default/
SPRG=../../


${PACKAGE_DIR}/data: ${SOURCE_DIR}
	rm -rf $@
	mkdir $@
	mkdir -p $@/usr/bin $@/usr/share/doc/radare $@/usr/share/radare/ $@/usr/libexec/gradare $@/usr/share/radare/rsc $@/usr/lib $@/usr/share/radare/magic
	#radare
	-cp ${PRG}/src/radiff/radiff $@/usr/bin/radiff
	-cp ${PRG}/src/radiff/bindiff $@/usr/bin/bindiff
	-cp ${PRG}/src/radare $@/usr/bin/radare
	-cp ${PRG}/src/rabin/rabin $@/usr/bin/rabin
	-cp ${PRG}/src/rasm/rasm $@/usr/bin/rasm
	-cp ${PRG}/src/rasc/rasc $@/usr/bin/rasc
	-cp ${PRG}/src/arch/arm/aasm/armasm $@/usr/bin/armasm
	-cp ${PRG}/src/rsc $@/usr/bin/rsc
	-cp ${PRG}/src/rfile $@/usr/bin/rfile
	-cp ${PRG}/src/hasher/hasher $@/usr/bin/hasher
	-cp ${PRG}/src/xc $@/usr/bin/xc
	-cp ${PRG}/src/xrefs $@/usr/bin/xrefs
	-cp ${PRG}/src/javasm/javasm $@/usr/bin/javasm
	-cp ${PRG}/gui/gradare $@/usr/bin/gradare
	-cp /usr/bin/gtkdialog $@/usr/bin/gtkdialog
	-cp /usr/lib/libvte.so.9 $@/usr/lib
	-cp -rf ${SPRG}/gui/grsc/* $@/usr/share/radare/gradare
	-cp -rf ${SPRG}/libexec/* $@/usr/share/radare/rsc
	-cp -rf ${SPRG}/magic/* $@/usr/share/radare/magic/
	-cp -rf ${SPRG}/doc/* $@/usr/share/doc/radare/
	-chmod +x $@/usr/bin/*
	mkdir -p $@/usr/share/applications/hildon/
	-cp ${SPRG}/gui/gradare.desktop $@/usr/share/applications/hildon/

include deb_hand.mak
