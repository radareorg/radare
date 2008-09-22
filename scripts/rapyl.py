from socket import *
#from array import *
from struct import *

PORT = 9997

RMT_OPEN=1
RMT_READ=2
RMT_WRITE=3
RMT_SEEK=4
RMT_CLOSE=5
RMT_SYSTEM=6
RMT_REPLY=0x80
FD=3434

offset = 0

def handle_packet(c, key):
	if key == RMT_OPEN:
		print "OPEN command"
		flags = c.recv(1)
		len = c.recv(1)
		file = c.recv(len)
		buf = pack(">Bi", key|RMT_REPLY, FD)
		c.send(buf)
	elif key == 2:
		print "READ command"
		buf = c.recv(4) # handle endian here length
		len = unpack(">i", buf)
		str = "patata"
		len = len(str)
		buf = pack(">Bis", key|RMT_REPLY, len, str)
		c.send(buf)
	elif key == RMT_WRITE:
		print "WRITE command"
	elif key == RMT_SEEK:
		print "SEEK command"
		seek = c.recv(4)
		buf = pack(">Bi", key|RMT_REPLY, seek);
		c.send(buf)
	elif key == RMT_CLOSE:
		print "CLOSE command"
		# XXX
	elif key == RMT_SYSTEM:
		buf = c.recv(4)
		len = unpack(">i", buf)
		str = c.recv(len)
		print "SYSTEM command (%s)"%str
		reply = "LOL"
		buf = pack(">Bis", key|RMT_REPLY, len(reply), reply)
	elif key == 0x80:
		print "WTF command"

def handle_client(c):
	while True:
		handle_packet(c, c.recv(1))

def listen_for_clients(port):
	s = socket();
	try:
		s.bind(("0.0.0.0", PORT))
		s.listen(999)
		print "Listening at port %d"%port
		while True:
			try:
				c = s.accept()
				#print "New client %s"%c
				handle_client(c)
			except:
				print "Oops"
	except:
		print "Cannot listen at port %d (%s)"%(port, "unknown")

listen_for_clients (PORT)
