#!/usr/bin/python
#
# python example using the radapy (remote radare API for python)
#
# -- pancake // nopcode .org
#

import radapy
from string import *

PORT = 9999

def fun_system(str):
	print "CURRENT SEEK IS %d"%radapy.offset
	return str

def fun_open(file,flags):
	return str

def fun_seek(off,type):
	return str

def fun_write(buf):
	print "WRITING %d bytes (%s)"%(len(buf),buf)
	return 6

def fun_read(len):
	print "READ %d bytes from %d\n"% (len, radapy.offset)
	str = "patata"
	str = str[radapy.offset:]
	return str


# main

#radapy.handle_cmd_open = fun_open
#radapy.handle_cmd_close = fun_close
rs = radapy.RapServer()
rs.handle_cmd_system = fun_system
rs.handle_cmd_read = fun_read
rs.handle_cmd_write = fun_write
rs.size = 10
rs.listen_tcp (PORT)
