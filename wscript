#! /usr/bin/env python

import os

VERSION='0.9.8'
APPNAME='radare'
LIL_ENDIAN="0"
MAEMO="0"
HAVE_VALAC="0"
DEBUGGER=False
LIBDIR="/usr/lib"
LIBEXECDIR="/usr/libexec"
DOCDIR="/usr/share/doc"

srcdir = '.'
blddir = 'build'

def set_options(opt):
	opt.tool_options('compiler_cc')
	opt.tool_options('compiler_cxx')
	opt.add_option('--without-readline', type='string', help='Build without readline', dest='HAVE_READLINE')
	opt.add_option('--without-debugger', action='store_false', default=True, help='Build without debugger', dest='DEBUGGER')

def configure(conf):
	conf.check_tool('compiler_cc compiler_cxx cc vala perl lua')
	if not conf.env['CC']: fatal('C compiler not found')

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
	if DEBUGGER:
		conf.env['CCFLAGS'].append('-DDEBUGGER=1')
		print " = DEBUGGER: 1"
	else:
		conf.env['CCFLAGS'].append('-DDEBUGGER=0')
		print " = DEBUGGER: 0"
	conf.env['CCFLAGS'].append('-DTARGET=\\"i686-unknown-linux-gnu\\"')
#	conf.env['CCFLAGS'].append('-DRADARE_CORE')
	conf.env['CCFLAGS'].append('-DHAVE_LIB_EWF=0')
	#conf.define('VERSION', VERSION)
	#conf.env.append_value('CCFLAGS', '')
	#conf.write_config_header('src/waf.h')

	rl = conf.create_library_configurator()
	rl.name = 'readline'
	rl.libs = ['readline']
	rl.mandatory = False
	rl.run()

	rl2 = conf.create_library_configurator()
	rl2.name = 'dl'
	rl2.libs = ['dl']
	rl2.mandatory = False
	rl2.run()
	

def build(bld):
	#bld.add_subdirs('vala')
	os.system("cp "+srcdir+"/src/utils.c "+srcdir+"/ut.c")
	bld.add_subdirs('src')
	#if Params.g_commands['clean']:
	#	os.system("echo unknown clean")
	#else:


def shutdown():
	pass

#        # display the graph of header dependencies
##       if 1: return
#        import Params, os, types
#
#        f = open('foo.dot', 'w')
#        f.write("digraph G {\n")
#        table = Params.g_build.node_deps
#        for a in table:
#                for x in table[a]:
#                        if type(table[a][x]) is types.ListType:
#                                for y in table[a][x]:
#                                        f.write('"%s" -> "%s";\n' % (x, y))
#                        else:
#                                f.write('"%s" -> "%s";\n' % (x, table[a][x]))
#        f.write('}\n')
#        f.close()
#
