#! /usr/bin/env python

import os
import shutil
import Options

VERSION='1.0-beta'
APPNAME='radare'
TARGET='i686-unknown-linux-gnu'
LIL_ENDIAN='0'
MAEMO='0'
HAVE_VALAC='0'
DEBUGGER=False
LIBDIR='/usr/lib'
LIBEXECDIR='/usr/libexec'
DOCDIR='/usr/share/doc'

srcdir = '.'
blddir = 'build'

def set_options(opt):
	opt.tool_options('compiler_cc')
	opt.tool_options('compiler_cxx')
	opt.add_option('--with-maemo',       action='store_true',  default=False, help='Build for maemo',          dest='MAEMO')
	opt.add_option('--with-sysproxy',    action='store_true',  default=False, help='Build with syscall proxy', dest='SYSPROXY')
	opt.add_option('--without-debugger', action='store_false', default=True,  help='Build without debugger',   dest='DEBUGGER')
	opt.add_option('--without-readline', action='store_false', default=True,  help='Build without readline',   dest='HAVE_READLINE')

def configure(conf):
	conf.check_tool('compiler_cc compiler_cxx cc vala perl lua')
	if not conf.env['CC']: fatal('C compiler not found')

        conf.check_pkg('glib-2.0', destvar='GLIB', vnum='2.10.0', mandatory=False)
        conf.check_pkg('gtk+-2.0', destvar='GTK', vnum='2.10.0', mandatory=False)
        conf.check_pkg('vte', destvar='GTK', vnum='0.16', mandatory=False)
	conf.checkEndian()

	# Generate GLOBAL.H
	conf.define('True', 1)
	conf.define('False', 0) # hack for booleans
	conf.define('VERSION', VERSION)
	conf.define('W32', False)
	conf.define('GUI', False)
	conf.define('SIZE_OFF_T', 8)
	conf.define('DARWIN', False)
	conf.define('DEBUGGER', Options.options.DEBUGGER)
	conf.define('SYSPROXY', Options.options.SYSPROXY)
	conf.define('_MAEMO_', Options.options.MAEMO)
	conf.define('CPU', 'i686') # XXX
	conf.define('TARGET', 'i686-unknown-linux-gnu') # XXX

	conf.define('HAVE_VALAC', False)
	conf.define('HAVE_LANG_LUA', False)
	conf.define('LIL_ENDIAN', False)
	conf.define('HAVE_LANG_PYTHON', False)
	conf.define('HAVE_LIB_EWF', False) # TODO
	conf.define('LIBDIR', '/usr/lib')
	conf.define('DOCDIR', '/usr/share/doc/radare')
	#conf.define('LIBEXECDIR', '/usr/share/doc/radare') # DEPRECATED


	# Check for libreadline
	rl = conf.create_library_configurator()
	rl.name = 'readline'
	rl.define = 'HAVE_READLINE'
	rl.libs = ['readline']
	rl.mandatory = False
	rl.run()
	if conf.env['HAVE_READLINE'] != 1:
		Options.options.HAVE_READLINE = False
	conf.define('HAVE_LIB_READLINE', Options.options.HAVE_READLINE)

	rl2 = conf.create_library_configurator()
	rl2.name = 'dl'
	rl2.libs = ['dl']
	rl2.mandatory = False
	rl2.run()

	# Write global.h and show report to stdout
	conf.write_config_header('global.h')
	shutil.copyfile("%s/default/global.h"%blddir, "%s/global.h"%srcdir)
	
	if Options.options.DEBUGGER:
		print " * Debugger: enabled"
	else:
		print " * Debugger: disabled"

	if Options.options.HAVE_READLINE:
		print " * Readline: enabled"
	else:
		print " * Readline: disabled"

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
