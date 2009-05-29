hijack=0

import radare

# r.cmd() hijacking
if hijack:
	class Food:
		def cmd(str):
			print "Command to run is (%s)"%str
		cmd = staticmethod(cmd)
	radare.r = Food

#r = Food
#r.cmd("#test")
radare.seek(33)
radare.disasm(0, 10)
