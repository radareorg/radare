#! /usr/bin/env python

import os
import os.path
import shutil
import Options

VERSION='1.1'
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
	opt.add_option('--without-lua',      action='store_false', default=True,  help='Build without LUA',        dest='HAVE_LIBLUA')
	opt.add_option('--without-python',   action='store_false', default=True,  help='Build without Python',     dest='HAVE_PYTHON')
	opt.add_option('--without-ruby',     action='store_false', default=True,  help='Build without Ruby',       dest='HAVE_RUBY')
	opt.add_option('--prefix',
		help    = "installation prefix [Default: '%s']" % prefix,
		default = prefix,
		dest    = 'PREFIX')

def configure(conf):
	# Check for compiler tools
	conf.check_tool('compiler_cc')
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

	if Options.options.HAVE_PYTHON == True:
		try:
			conf.check_tool('python')
			conf.check_python_version((2,4,2));
			conf.check_python_headers()
			conf.env['HAVE_PYTHON']=1 # XXX
		except: 
			print " ==> Use --without-python"
			conf.env['HAVE_PYTHON']=0
		pass
	else:
		conf.env['HAVE_PYTHON']=0

	if Options.options.MAEMO == True:
        	conf.check_pkg('hildon-1',      destvar='HILDON', vnum='0.1', mandatory=False)

	# Generate GLOBAL.H
	conf.env['GUI'] = False
	# Vala is no longer required for building the GUI
	if conf.env['HAVE_GTK'] == 1 and conf.env['HAVE_VTE'] == 1 and Options.options.GUI:
		conf.env['GUI'] = True
	# if conf.env['HAVE_GTK'] == 1 and conf.env['HAVE_VTE'] == 1 and have_valac and Options.options.GUI:
	#	conf.env['GUI'] = True
	conf.env['OS'] = os.uname()[0]
	conf.env['CPU'] = os.uname()[4]
	if conf.env['CPU'] == 'Power Macintosh': conf.env['CPU'] = 'powerpc' # osx
	if conf.env['CPU'] == 'ppc': conf.env['CPU'] = 'powerpc' # linux
	if conf.env['CPU'] == 'i86pc': conf.env['CPU'] = 'i386'
	if conf.env['CPU'] == 'i686': conf.env['CPU'] = 'i386'
	if conf.env['CPU'] == 'i586': conf.env['CPU'] = 'i386'
	if conf.env['CPU'] != 'arm' and conf.env['CPU'] != 'powerpc' and conf.env['CPU'] != 'i386' and conf.env['CPU'] != 'powerpc' and conf.env['CPU'] != 'x86_64' and conf.env['CPU'] != 'mips64':
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
	endian = True
	try:
		if conf.env['IS_BIGENDIAN'] == 1:
			endian = False
	except x:
		endian = True
	conf.define('LIL_ENDIAN', endian)
	conf.define('HAVE_LANG_PYTHON', False)
	conf.define('PREFIX',  Options.options.PREFIX)
	conf.define('LIBDIR',  "%s/lib"%Options.options.PREFIX)
	conf.define('SHAREDIR', "%s/share/"%Options.options.PREFIX)
	conf.define('DOCDIR',  "%s/share/doc/radare"%Options.options.PREFIX)
	conf.define('GRSCDIR', "%s/share/radare/gradare"%Options.options.PREFIX)
	#conf.define('LIBEXECDIR', '/usr/share/doc/radare') # DEPRECATED

	# Check for ruby
	print "Checking for ruby mkmf\t\t\t    :",
	if os.system("ruby src/plug/hack/chkruby.rb") == 0:
		print "ok"
	else:
		print "not found"
		Options.options.HAVE_RUBY = False
	conf.define('HAVE_RUBY', Options.options.HAVE_RUBY)

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

	# Check for liblua
	rl = conf.create_library_configurator()
	rl.name = 'lua'
	rl.define = 'HAVE_LIBLUA'
	rl.libs = ['lua']
	rl.mandatory = False
	rl.run()
	if conf.env['HAVE_LIBLUA'] != 1:
		rl2 = conf.create_library_configurator()
		rl2.name = 'lua5.1'
		rl2.define = 'HAVE_LIBLUA51'
		rl2.libs = ['lua5.1']
		rl2.mandatory = False
		rl2.run()
		if conf.env['HAVE_LIBLUA51'] != 1:
			Options.options.HAVE_LIBLUA = False
		else:
			conf.env['HAVE_LIBLUA_LD'] = "-llua5.1"
			conf.env['HAVE_LIBLUA_CC'] = "-I /usr/include/lua5.1"
	else:
		conf.env['HAVE_LIBLUA_LD'] = "-llua"
	conf.define('HAVE_LIBLUA', Options.options.HAVE_LIBLUA)
	conf.define('HAVE_GUI', conf.env['GUI'])

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
	
	print " * Prefix    : %s"%conf.env['PREFIX']
	#print " * InstDir   : %s"%inst_var
	print " * Target    : %s"%conf.env['TARGET']
	print " * LilEndian : %s"%conf.env['LIL_ENDIAN']
	print " * HaveRuby  : %s"%conf.env['HAVE_RUBY']

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

	if Options.options.SYSPROXY:
		print " * SysProxy  : enabled"
	else:
		print " * SysProxy  : disabled"

	if conf.env['GUI'] == 1:
		print " * GUI       : enabled"
	else:
		print " * GUI       : disabled"
	print "Use --without-gui : vala-waf support is not yet complete"

def build(bld):
	#shutil.copyfile(srcdir+"/src/utils.c", srcdir+"/src/ut.c")
	if bld.env['GUI']:
		bld.add_subdirs('vala')
	bld.add_subdirs('src')
	#if Params.g_commands['clean']:
	#	os.system("echo unknown clean")

	bld.install_files('${PREFIX}/lib/python2.5', 'src/plug/hack/radare.py')
	bld.install_files('${PREFIX}/lib/radare', 'src/plug/hack/radare.lua')
	bld.install_files('${PREFIX}/lib/radare', 'src/plug/hack/radare.rb')
	# RSC scripts
	#if bld.env['HAVE_RUBY'] == 1:
	try:
		if os.path.exists('src/plug/hack/ruby.so'):
			bld.install_files('${PREFIX}/lib/radare', 'src/plug/hack/ruby.so', chmod=0755)
		else:
			print "no ruby?"
	except:
		print "WTF with ruby!!"
	bld.install_files('${PREFIX}/lib/radare/bin', 'src/rsc/pool/*', chmod=0755)
	bld.install_files('${PREFIX}/lib/radare/bin', 'src/radiff/bindiff-ng/bindiff-ng', chmod=0755)
	bld.install_files('${PREFIX}/lib/radare/bin', 'src/javasm/javasm', chmod=0755)
	bld.install_files('${PREFIX}/lib/radare/bin', 'src/arch/arm/armasm', chmod=0755)
	bld.install_files('${PREFIX}/lib/radare/bin', 'src/lsbstego', chmod=0755)
	# Documentation
	bld.install_files('${PREFIX}/share/man/man1', 'man/*.1')
	bld.install_files('${PREFIX}/share/man/man5', 'man/*.5')
	bld.install_files('${PREFIX}/share/doc/radare', 'README')
	bld.install_files('${PREFIX}/share/doc/radare', 'COPYING')
	bld.install_files('${PREFIX}/share/doc/radare', 'AUTHORS')
	bld.install_files('${PREFIX}/share/doc/radare', 'HISTORY')
	bld.install_files('${PREFIX}/share/doc/radare', 'TODO')
	bld.install_files('${PREFIX}/share/doc/radare', 'doc/csr')
	bld.install_files('${PREFIX}/share/doc/radare', 'doc/flags')
	bld.install_files('${PREFIX}/share/doc/radare', 'doc/fortunes')
	bld.install_files('${PREFIX}/share/doc/radare', 'doc/gdb.scripts')
	bld.install_files('${PREFIX}/share/doc/radare', 'doc/map-struct')
	bld.install_files('${PREFIX}/share/doc/radare', 'doc/shell')
	bld.install_files('${PREFIX}/share/doc/radare/xtra', 'doc/xtra/*')
	bld.install_files('${PREFIX}/share/doc/radare/', 'src/radiff/ida2rdb.idc')
	if bld.env['GUI']:
		bld.install_files('${PREFIX}/share/radare/gradare', 'gui/grsc/Shell', chmod=0755)
		for dir in "Config Debugger Disassembly Flags Hacks Movement Search Visual".split(" "):
			bld.install_files('${PREFIX}/share/radare/gradare/%s'%dir, 'gui/grsc/%s/*'%dir, chmod=0755)

def install():
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
