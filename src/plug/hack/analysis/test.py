import r
import radare
import sys
sys.path.append('.')
import ranal

print "---------------------------------"

print r.cmd("e scr.color=0")
p = ranal.Program()

print "File type: %s" % p.type
print "File size: %s" % p.size
print "OperatingSystem: %s" % p.os
print "Architecture: %s" % p.arch

f = ranal.Functions()
for f in f.list:
	print "%s: %s"%(f.addr,f.name)

print "---------------------------------"
radare.quit(0)
