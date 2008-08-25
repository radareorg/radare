#! /usr/bin/env python

import os
import shutil
import Options

VERSION='1.0-beta'
APPNAME='radare'

srcdir = '.'
blddir = 'build'
prefix = '/usr'

def set_options(opt):
	opt.tool_options('compiler_cc')
	opt.tool_options('compiler_cxx')
	opt.add_option('--with-maemo',       action='store_true',  default=False, help='Build for maemo',          dest='MAEMO')
	opt.add_option('--with-sysproxy',    action='store_true',  default=False, help='Build with syscall proxy', dest='SYSPROXY')
	opt.add_option('--without-gui',      action='store_false', default=True,  help='Build without GUI',        dest='GUI')
	opt.add_option('--without-debugger', action='store_false', default=True,  help='Build without debugger',   dest='DEBUGGER')
	opt.add_option('--without-readline', action='store_false', default=True,  help='Build without readline',   dest='HAVE_READLINE')
	opt.add_option('--prefix',
		help    = "installation prefix [Default: '%s']" % prefix,
		default = prefix,
		dest    = 'prefix')

def configure(conf):
	# Check for compiler tools
	conf.check_tool('compiler_cc cc')
	try: conf.check_tool('compiler_cxx')
	except: pass
	have_valac = False
	try:
		conf.check_tool('vala')
		have_valac = True
	except: pass
	try: conf.check_tool('perl')
	except: pass
	try: conf.check_tool('lua')
	except: pass
	if not conf.env['CC']: fatal('C compiler not found')

	# Check libraries
        conf.check_pkg('glib-2.0', destvar='GLIB', vnum='2.10.0', mandatory=False)
        conf.check_pkg('gtk+-2.0', destvar='GTK', vnum='2.10.0', mandatory=False)
        conf.check_pkg('vte',      destvar='VTE', vnum='0.16', mandatory=False)
	conf.checkEndian()

	# Generate GLOBAL.H
	conf.env['GUI'] = False
	if conf.env['HAVE_GTK'] and conf.env['HAVE_VTE'] and have_valac and Options.options.GUI:
		conf.env['GUI'] = True
	conf.env['OS']= os.uname()[0]
	conf.env['CPU']= os.uname()[4]
	if conf.env['CPU'] == 'i686': conf.env['CPU'] = 'i386'
	if conf.env['CPU'] == 'i586': conf.env['CPU'] = 'i386'
	if conf.env['CPU'] != 'i386' and conf.env['CPU'] != 'powerpc' and conf.env['CPU'] != 'x86_64' and conf.env['CPU'] != 'mips64':
		print "Unknown CPU. Disabling debugger"
		Options.options.DEBUGGER = False

	conf.define('True', 1)
	conf.define('False', 0) # hack for booleans
	conf.define('VERSION', VERSION)
	conf.define('W32', False)
	conf.define('SIZE_OFF_T', 8)
	conf.define('DARWIN', False)
	conf.define('DEBUGGER', Options.options.DEBUGGER)
	conf.define('SYSPROXY', Options.options.SYSPROXY)
	conf.define('_MAEMO_', Options.options.MAEMO)
	conf.define('CPU', conf.env['CPU'])
	conf.define('OS', conf.env['OS'])
	conf.define('TARGET', "%s-%s"%(conf.env['CPU'],conf.env['OS']))

	conf.define('HAVE_VALAC', conf.env['GUI'])
	#conf.define('HAVE_GTK', conf.env['HAVE_GTK'])
	conf.define('HAVE_LANG_LUA', False)
	try:
		conf.env['IS_BIGENDIAN']
		endian = True
	except x:
		endian = False
	conf.define('LIL_ENDIAN', endian)
	conf.define('HAVE_LANG_PYTHON', False)
	conf.define('PREFIX', prefix)
	conf.define('LIBDIR', "%s/lib"%prefix)
	conf.define('DOCDIR', "%s/share/doc/radare"%prefix)
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

	rl2 = conf.create_library_configurator()
	rl2.name = 'ewf'
	rl2.libs = ['ewf']
	rl2.mandatory = False
	rl2.run()
	if conf.env['HAVE_EWF'] != 1:
		conf.env['HAVE_EWF'] = False
	conf.define('HAVE_LIB_EWF', conf.env['HAVE_EWF'])

	# Write global.h and show report to stdout
	conf.write_config_header('global.h')
	shutil.copyfile("%s/default/global.h"%blddir, "%s/global.h"%srcdir)
	
	print " * Prefix    : %s"%prefix
	#print " * InstDir   : %s"%inst_var
	print " * Target    : %s"%conf.env['TARGET']
	print " * LilEndian : %s"%conf.env['LIL_ENDIAN']

	if conf.env['HAVE_LIB_EWF'] == 1:
		print " * EWF       : enabled"
	else:
		print " * EWF       : disabled"

	if Options.options.DEBUGGER:
		print " * Debugger  : enabled"
	else:
		print " * Debugger  : disabled"

	if Options.options.HAVE_READLINE:
		print " * Readline  : enabled"
	else:
		print " * Readline  : disabled"

	if conf.env['GUI'] == 1:
		print " * GUI       : enabled"
	else:
		print " * GUI       : disabled"
	print "Use --without-gui : vala-waf support is not yet complete"

def build(bld):
	shutil.copyfile(srcdir+"/src/utils.c", srcdir+"/src/ut.c")
	if bld.env['GUI']:
		bld.add_subdirs('vala')
	bld.add_subdirs('src')
	#if Params.g_commands['clean']:
	#	os.system("echo unknown clean")
	#else:

def install(self):
	print "Installing..."


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
