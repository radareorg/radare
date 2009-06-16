hijack=1

import radare
import ranal
import radapy

# r.cmd() hijacking
if hijack:
	class Food:
		def cmd(str):
			global c
			print "Command to run is (%s)"%str
			return c.cmd(str)
		cmd = staticmethod(cmd)
	global r
	radare.r = Food


c = radapy.RapClient("localhost", 9999)

fd = c.open("/bin/ls", 0)
print c.cmd("px")
#r = Food
#r.cmd("#test")
print radare.r.cmd("pd 20")
radare.seek(33)
print radare.disasm(0, 10)

# close
c.close(fd)
c.disconnect()
