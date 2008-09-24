#!/usr/bin/python
#
# Python implementation of the radare remote protocol
#

from traceback import *
from socket import *
import sys
from struct import *

PORT = 9999
FD=3434

RMT_OPEN   = 1
RMT_READ   = 2
RMT_WRITE  = 3
RMT_SEEK   = 4
RMT_CLOSE  = 5
RMT_SYSTEM = 6
RMT_REPLY  = 0x80

handle_cmd_system = None
handle_cmd_seek = None
handle_cmd_read = None
handle_cmd_write = None
handle_cmd_open = None

offset = 0
size = 0
buffer = None

def handle_packet(c, key):
	global offset
	global size
	global buffer
#	print "Handling packet %x",key
	if key == RMT_OPEN:
		buffer = c.recv(2)
		(flags, length) = unpack(">BB", buffer)
		file = c.recv(length)
		buf = pack(">Bi", key|RMT_REPLY, FD)
		c.send(buf)
	elif key == RMT_READ:
		#print "READ command at %d ----------"%offset
		buffer = c.recv(4)
		(length,) = unpack(">I", buffer)
		if handle_cmd_read != None:
			str = handle_cmd_read(length)
			lon = len(str)
		else:
			str = ""
			lon = 0;
		buf = pack(">Bi", key|RMT_REPLY, lon)
		c.send(buf+str)
	elif key == RMT_WRITE:
		#print "WRITE command"
		buffer = c.recv(4)
		(length,) = unpack(">I", buffer)
		buffer = c.recv(length)
		# TODO: get buffer and length
		if handle_cmd_write != None:
			length = handle_cmd_write (buffer)
			
		buf = pack(">Bi", key|RMT_REPLY, length)
		c.send(buf)
	elif key == RMT_SEEK:
		buffer = c.recv(9)
		(type, off) = unpack(">BQ", buffer)
		if handle_cmd_seek != None:
			seek = handle_cmd_seek(off, type)
		else:
			if   type == 0: # SET
				seek = off;
			elif type == 1: # CUR
				seek = seek + off 
			elif type == 2: # END
				seek = size;
		
		offset = seek
		#print "SEEK command (type=%d) (seek=%d)"%(type,offset)
		buf = pack(">BQ", key|RMT_REPLY, seek)
		c.send(buf)
	elif key == RMT_CLOSE:
		print "CLOSE command"
		# XXX
	elif key == RMT_SYSTEM:
		buf = c.recv(4)
		(length,) = unpack(">i", buf)
		str = c.recv(length)
		print "SYSTEM command (%s)"%str

		if handle_cmd_system != None:
			reply = handle_cmd_system(str)
		else:	reply = ""

		buf = pack(">Bi", key|RMT_REPLY, len(reply))
		print "SYSTEM reply (%s)"%reply
		c.send(buf+reply)
	else:
		print "UNKNOWN COMMAND"
		c.close()
		exit(1)

def handle_client(c):
	while True:
		try:
			handle_packet(c, ord(c.recv(1)))
		except KeyboardInterrupt:
			break
		#except TypeError, foo:
		#	print_stack()
		#	break
#		except:
#			print "HandleClientErrorOops (%s)"%(sys.exc_info()[0])
#			break

def listen_for_clients(port):
	s = socket();
	s.bind(("0.0.0.0", PORT))
	s.listen(999)
	print "Listening at port %d"%port
	while True:
		(c, (addr,port)) = s.accept()
		print "New client %s:%d"%(addr,port)
		handle_client(c)

#			try:
#			except:
#				print "Oops (%s)"%(sys.exc_info()[0])
#				s.close()
#				break
#	except:
#		print "Cannot listen at port %d (%s)"%(port, "unknown")


#listen_for_clients (PORT)
