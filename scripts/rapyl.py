#!/usr/bin/python
#
# Python implementation of the radare remote protocol
#

# TODO: use 64 bit here

from socket import *
import sys
from struct import *

PORT = 9997
FD=3434

RMT_OPEN   = 1
RMT_READ   = 2
RMT_WRITE  = 3
RMT_SEEK   = 4
RMT_CLOSE  = 5
RMT_SYSTEM = 6
RMT_REPLY  = 0x80

offset = 0

def handle_packet(c, key):
	print "Handling packet %x",key
	if key == RMT_OPEN:
		print "OPEN command"
		flags = c.recv(1)
		lun = c.recv(1)
		print dir(ord(lun[0]))
		file = c.recv(ord(lun[0]))
		buf = pack(">Bi", key|RMT_REPLY, FD)
		c.send(buf)
	elif key == 2:
		print "READ command"
		buf = c.recv(4) # handle endian here length
		lun = unpack(">i", buf)
		str = "patata"
		lon = 6;
		buf = pack(">Bis", key|RMT_REPLY, lon, str)
		c.send(buf)
	elif key == RMT_WRITE:
		print "WRITE command"
	elif key == RMT_SEEK:
		print "SEEK command"
		type = c.recv(1)
		seek = c.recv(4)
		buf = pack(">BBBBB", key|RMT_REPLY, ord(seek[0]), ord(seek[1]), ord(seek[2]), ord(seek[3]));
		c.send(buf)
	elif key == RMT_CLOSE:
		print "CLOSE command"
		# XXX
	elif key == RMT_SYSTEM:
		buf = c.recv(4)
		lun = unpack(">i", buf)
		str = c.recv(lun)
		print "SYSTEM command (%s)"%str
		reply = "LOL"
		buf = pack(">Bis", key|RMT_REPLY, len(reply), reply)
	elif key == 0x80:
		print "WTF command"
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
#		except:
#			print "HandleClientErrorOops (%s)"%(sys.exc_info()[0])
#			break

def listen_for_clients(port):
	s = socket();
	s.bind(("0.0.0.0", PORT))
	s.listen(999)
	print "Listening at port %d"%port
	while True:
		(c, addr) = s.accept()
		print "New client" # %s"%addr
		handle_client(c)

#			try:
#			except:
#				print "Oops (%s)"%(sys.exc_info()[0])
#				s.close()
#				break
#	except:
#		print "Cannot listen at port %d (%s)"%(port, "unknown")

listen_for_clients (PORT)
