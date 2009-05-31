#!/usr/bin/python
#
# Python implementation of the radare remote protocol
#

##===================================================0
## server api
##===================================================0

from socket import *
from struct import *
import traceback
import sys

RMT_OPEN   = 1
RMT_READ   = 2
RMT_WRITE  = 3
RMT_SEEK   = 4
RMT_CLOSE  = 5
RMT_SYSTEM = 6
RMT_CMD    = 7
RMT_REPLY  = 0x80

handle_cmd_system = None
handle_cmd_seek   = None
handle_cmd_read   = None
handle_cmd_write  = None
handle_cmd_open   = None
handle_cmd_close  = None

offset = 0
size = 0
buffer = None

def _handle_packet(c, key):
	global offset
	global size
	global buffer
	ret = ""
	if key == RMT_OPEN:
		buffer = c.recv(2)
		(flags, length) = unpack(">BB", buffer)
		file = c.recv(length)
		if handle_cmd_open != None:
			fd = handle_cmd_open(file, flags)
		else: 	fd = 3434
		buf = pack(">Bi", key|RMT_REPLY, fd)
		c.send(buf)
	elif key == RMT_READ:
		buffer = c.recv(4)
		(length,) = unpack(">I", buffer)
		if handle_cmd_read != None:
			ret = str(handle_cmd_read(length))
			try:
				lon = len(ret)
			except:
				ret = ""
				lon = 0
		else:
			ret = ""
			lon = 0;
		buf = pack(">Bi", key|RMT_REPLY, lon)
		c.send(buf+ret)
	elif key == RMT_WRITE:
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
		if handle_cmd_close != None:
			length = handle_cmd_close (fd)
	elif key == RMT_SYSTEM:
		buf = c.recv(4)
		(length,) = unpack(">i", buf)
		ret = c.recv(length)
		if handle_cmd_system != None:
			reply = handle_cmd_system(ret)
		else:	reply = ""
		buf = pack(">Bi", key|RMT_REPLY, len(str(reply)))
		c.send(buf+reply)
	else:
		print "Unknown command"
		c.close()

def _handle_client(c):
	while True:
		try:
			_handle_packet(c, ord(c.recv(1)))
		except KeyboardInterrupt:
			break
		#except TypeError, foo:
		#	traceback.print_stack()
		#	break
#		except:
#			print "HandleClientErrorOops (%s)"%(sys.exc_info()[0])
#			break

def listen_tcp(port):
	s = socket();
	s.bind(("0.0.0.0", port))
	s.listen(999)
	print "Listening at port %d"%port
	while True:
		(c, (addr,port)) = s.accept()
		print "New client %s:%d"%(addr,port)
		_handle_client(c)

#			try:
#			except:
#				print "Oops (%s)"%(sys.exc_info()[0])
#				s.close()
#				break
#	except:
#		print "Cannot listen at port %d (%s)"%(port, "unknown")



##===================================================0
## client api
##===================================================0

class RapClient():
	def __init__(self, host, port):
		self.connect_tcp(host, port)

	def connect_tcp(self, host, port):
		fd = socket();
		fd.connect((host, port))
		self.fd = fd

	def disconnect(self):
		self.fd.close()
		self.fd = None

	def open(self, file, flags):
		b = pack(">BBB", RMT_OPEN, flags, len(file))
		self.fd.send(b)
		self.fd.send(file)
		# response
		buf = self.fd.recv(5)
		(c,l) = unpack(">Bi", buf)
		if c != (RMT_REPLY|RMT_OPEN):
			print "rmt-open: Invalid response packet 0x%02x"%c
		return l

	# TODO. not yet implemented
	def read(self, len):
		b = pack(">Bi", RMT_READ, len(buf))
		self.fd.send(b+buf)
		# response
		buf = self.fd.recv(5)
		(c,l) = unpack(">Bi", buf)
		buf = self.fd.recv(l)
		return buf

	# TODO: not tested
	def write(self, buf):
		#self.fd.send(buf)
		b = pack(">Bi", RMT_WRITE, len(buf))
		self.fd.send(b+buf)
		# response
		buf = self.fd.recv(5)
		(c,l) = unpack(">Bi", buf)
		if c != (RMT_REPLY|RMT_WRITE):
			print "rmt-write: Invalid response packet 0x%02x"%c

	def lseek(self, type, addr):
		buf = pack(">BBQ", RMT_SEEK, type, addr)
		self.fd.send(buf)
		# read response
		buf = self.fd.recv(5)
		(c,l) = unpack(">Bi", buf)
		#print "Lseek : %d"%l
		return l

	def close(self, fd):
		buf = pack(">Bi", RMT_CLOSE, fd)
		self.fd.send(buf)
		# read response
		buf = self.fd.recv(5)
		(c,l) = unpack(">Bi", buf)
		if c != RMT_REPLY | RMT_CLOSE:
			print "rmt-close: Invalid response packet"

	def cmd(self, cmd):
		buf = pack(">Bi", RMT_CMD, len(str(cmd)))
		self.fd.send(buf + cmd)
		# read response
		buf = self.fd.recv(5)
		(c,l) = unpack(">Bi", buf)
		if c != RMT_CMD | RMT_REPLY:
			print "rmt-cmd: Invalid response packet"
			return ""
		buf = self.fd.recv(l)
		return buf

	def system(self, cmd):
		buf = pack(">Bi", RMT_SYSTEM, len(str(cmd)))
		self.fd.send(buf)
		self.fd.send(cmd)
		# read response
		buf = self.fd.recv(5)
		(c,l) = unpack(">Bi", buf)
		if c != RMT_CMD | RMT_REPLY:
			print "rmt-cmd: Invalid response packet"
			return ""
		buf = self.fd.recv(l)
		return buf
