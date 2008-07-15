#! /usr/bin/env python

VERSION='0.9.8'
APPNAME='radare'
LIL_ENDIAN="0"
MAEMO="0"
HAVE_VALAC="0"
LIBDIR="/usr/lib"
LIBEXECDIR="/usr/libexec"
DOCDIR="/usr/share/doc"

srcdir = '.'
blddir = 'build'

def set_options(opt):
	opt.tool_options('compiler_cc')
	opt.tool_options('compiler_cxx')
	opt.add_option('--without-readline', type='string', help='Build without readline', dest='HAVE_READLINE')

def configure(conf):
	conf.check_tool('compiler_cc compiler_cxx cc vala perl lua')
        conf.check_pkg('glib-2.0', destvar='GLIB', vnum='2.10.0', mandatory=False)
        conf.check_pkg('gtk+-2.0', destvar='GTK', vnum='2.10.0', mandatory=False)
        conf.check_pkg('vte', destvar='GTK', vnum='0.16', mandatory=False)
	conf.env['CCFLAGS'].append('-DVERSION=\\"'+VERSION+'\\"')
	conf.env['CCFLAGS'].append('-DLIL_ENDIAN="'+LIL_ENDIAN+'"')
	conf.env['CCFLAGS'].append('-D_MAEMO_="'+MAEMO+'"')
	conf.env['CCFLAGS'].append('-DHAVE_VALAC='+HAVE_VALAC)
	conf.env['CCFLAGS'].append('-DLIBDIR=\\"'+LIBDIR+'\\"')
	conf.env['CCFLAGS'].append('-DLIBEXECDIR=\\"'+LIBEXECDIR+'\\"')
	conf.env['CCFLAGS'].append('-DDOCDIR=\\"'+DOCDIR+'\\"')
	conf.env['CCFLAGS'].append('-DHAVE_LIB_READLINE=0')
	conf.env['CCFLAGS'].append('-DSIZE_OFF_T=8')
	conf.env['CCFLAGS'].append('-DDEBUGGER=0')
	conf.env['CCFLAGS'].append('-DTARGET=\\"i686-unknown-linux-gnu\\"')
	conf.env['CCFLAGS'].append('-DRADARE_CORE')
	conf.env['CCFLAGS'].append('-DHAVE_LIB_EWF=0')

	rl = conf.create_library_configurator()
	rl.name = 'readline'
	rl.libs = ['readline']
	rl.mandatory = False
	rl.run()
	

def build(bld):
	#bld.add_subdirs('vala')
	bld.add_subdirs('src')

def shutdown():
	pass
