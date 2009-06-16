import sys
sys.path.append('.')
import radare.remote

c = radare.remote.RapClient('localhost', 9999)
#c = RapClient('localhost', 9999)
fd = c.open("/bin/ls", 0)
print c.cmd("px")
print c.system("ls")
c.close(fd)
c.disconnect()
