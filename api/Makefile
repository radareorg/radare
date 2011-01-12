include ../config.mk

all:
	@echo Nothing to do

install:
	# ruby
	-mkdir -p ${DESTDIR}${LIBDIR}/ruby/1.8
	-cp -rf ruby/radare ${DESTDIR}${LIBDIR}/ruby/1.8
	# python
	-mkdir -p ${DESTDIR}${LIBDIR}/python2.5/site-packages
	-mkdir -p ${DESTDIR}${LIBDIR}/python2.6/site-packages
	-mkdir -p ${DESTDIR}${LIBDIR}/python2.7/site-packages
	-cp -rf python/radare ${DESTDIR}${LIBDIR}/python2.5/site-packages
	-cp -rf python/radare ${DESTDIR}${LIBDIR}/python2.6/site-packages
	-cp -rf python/radare ${DESTDIR}${LIBDIR}/python2.7/site-packages
	# XXX lua
	-cp -rf lua/radare/api.lua ${DESTDIR}${LIBDIR}/radare/radare.lua

.PHONY: all install
