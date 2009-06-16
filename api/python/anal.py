try:
	import r
except:
	import radare.remote
	print "Food"
import radare.api
import sys
sys.path.append('.')
from radare.analysis import *

print "---------------------------------"

print r.cmd("e scr.color=0")
print r.cmd("e graph.split=0")
p = Program()

print "File type: %s" % p.type
print "File size: %d bytes" % p.size
print "Entrypoint: 0x%x" % p.entrypoint
print "Virtual address: 0x%x" % p.vaddr
print "Physical address: 0x%x" % p.paddr
print "OperatingSystem: %s" % p.os
print "Architecture: %s" % p.arch
print "Endian: %s" % p.bigendian

print "Symbols:"
ss = Symbols()
for s in ss.list:
	print "0x%08x: size=%s name=%s"%(s.addr, s.size, s.name)
	Function.analyze(s.addr)

print "Functions:"
fs = Functions()
for f in fs.list:
	print "0x%08x: size=%s name=%s"%(f.addr, f.size, f.name)
	bb = BasicBlocks(f.addr)
	print "   ==> Basic blocks: %d"%len(bb.list)
	print "   ==> Disassembly:"
	print r.cmd("pd@%d:%d"%(f.addr,f.size))
	Graph.make_png(f.addr, "%s.png"%f.name)

print "Imports:"
ss = Imports()
for s in ss.list:
	print "0x%08x: size=%s name=%s"%(s.addr, s.size, s.name)
	for x in CodeXrefs(s.addr).list:
		print "  -> xref from 0x%08x"%(x.addr)

print "Xrefs:"
for x in CodeXrefs().list:
	print "  -> code xref from 0x%08x -> to 0x%08x"%(x.addr, x.endaddr)
for x in DataXrefs().list:
	print "  -> data xref from 0x%08x -> to 0x%08x"%(x.addr, x.endaddr)

print "Sections:"
ss = Sections()
for s in ss.list:
	print "0x%08x: size=%d %s"%(s.addr, s.size, s.name)

print "---------------------------------"
radare.api.quit(0)
