import sys
sys.path.append('.')
import radapy

c = radapy.RapClient('localhost', 9999)
#c = RapClient('localhost', 9999)
fd = c.open("/bin/ls", 0)
print c.cmd("px")
#c.system("x")
c.close(fd)
c.disconnect()
