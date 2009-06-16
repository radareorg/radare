include ../config.mk

all:
	@echo Nothing to do

install:
	# ruby
	-mkdir -p ${LIBDIR}/ruby/1.8
	-cp -rf ruby/radare ${LIBDIR}/ruby/1.8
	# python
	-mkdir -p ${LIBDIR}/python2.5/site-packages
	-mkdir -p ${LIBDIR}/python2.6/site-packages
	-cp -rf python/radare ${LIBDIR}/python2.5/site-packages
	-cp -rf python/radare ${LIBDIR}/python2.6/site-packages
	# XXX lua
	-cp -rf lua/radare/api.lua ${LIBDIR}/radare/radare.lua

.PHONY: all install
