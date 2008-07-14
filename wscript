#! /usr/bin/env python

VERSION='0.9.8'
APPNAME='radare'
LIL_ENDIAN="0"
MAEMO="0"
HAVE_VALAC="0"
LIBDIR="/usr/lib"
LIBEXECDIR="/usr/libexec"
DOCDIR="/usr/share/doc"

srcdir = './'
blddir = 'build'

def set_options(opt):
	opt.tool_options('compiler_cc')
	opt.tool_options('compiler_cxx')

def configure(conf):
	conf.check_tool('compiler_cc compiler_cxx cc vala')
	conf.check_tool('perl')
	conf.env['CCFLAGS'].append('-DVERSION=\\"'+VERSION+'\\"')
	conf.env['CCFLAGS'].append('-DLIL_ENDIAN="'+LIL_ENDIAN+'"')
	conf.env['CCFLAGS'].append('-D_MAEMO_="'+MAEMO+'"')
	conf.env['CCFLAGS'].append('-DHAVE_VALAC='+HAVE_VALAC)
	conf.env['CCFLAGS'].append('-DLIBDIR=\\"'+LIBDIR+'\\"')
	conf.env['CCFLAGS'].append('-DLIBEXECDIR=\\"'+LIBEXECDIR+'\\"')
	conf.env['CCFLAGS'].append('-DDOCDIR='+DOCDIR)
	conf.env['CCFLAGS'].append('-DHAVE_LIB_READLINE=1')
	conf.env['CCFLAGS'].append('-DSIZE_OFF_T=8')
	conf.env['CCFLAGS'].append('-DDEBUGGER=1')
	conf.env['CCFLAGS'].append('-DTARGET="i686-unknown-linux-gnu"')
	conf.env['CCFLAGS'].append('-DRADARE_CORE')
	conf.env['CCFLAGS'].append('-DHAVE_LIB_EWF=1')
	conf.env['CCFLAGS'].append('-I. -Iinclude')

def build(bld):
	bld.add_subdirs('src')
	#bld.add_subdirs('src/dbg')

def shutdown():
	pass
