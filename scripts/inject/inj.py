#
# This is a small python script to inject hand-made
# shellcodes, symbols, or array of bytes in a simple
# way into binaries at runtime or on-disk-patching
# in a simple way.
#

from radare import *

def inject_trampoline(addr, code_addr, code):
	seek(addr)
	op = analyze_opcode()
	if op['size'] != 5:
		print "Oops: The opcode at "+addr+" is not 5 byte long"
		return 0

	# Inject our trampoline
	r.cmd("wa jmp %s @ 0x%08x"%(code_addr, addr))
	r.cmd("wx %s"%code)

	return 1
