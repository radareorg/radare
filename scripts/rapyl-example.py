import rapyl
from string import *

PORT = 9999

def fun_system(str):
	print "CURRENT SEEK IS %d"%rapyl.offset
	return str

#def fun_seek(off,type):
#	return str

def fun_write(buf):
	print "WRITING %d bytes (%s)"%(len(buf),buf)
	return 6

def fun_read(len):
	print "READ %d bytes from %d\n"% (len, rapyl.offset)
	str = "patata"
	str = str[rapyl.offset:]
	return str


# main

rapyl.handle_cmd_system = fun_system
rapyl.handle_cmd_read = fun_read
rapyl.handle_cmd_write = fun_write
rapyl.size = 10
rapyl.listen_for_clients (PORT)
