"""
Example radare python script

 pancake <youterm.com>
"""

import string

from radare import *

seek(0x8048000)


eval_set("asm.lines", "false")
eval_set("asm.comments", "false")
#eval_set("asm.bytes", "false")

flag_space_set("sections")
list = flag_list()

# enumerate flags
for i in range (0, len(list)):
	q = list[i]
	print "%s"%q["name"]
	#print "%s"%q["offset"]

print flag_get("entrypoint")
print hex(3)
write("90 90 90")
print hex(3)

print dis(1)

print asm("mov eax,33")

op = analyze_opcode()
print "opcode string: %s"%op["opcode"]
print "opcode size: %s"%op["size"]

dbg_step(1)
#dbg_bp_set(0x8049412)
#dbg_continue()

# check if we're there
#eip = dbg_register_get("eip")
#print eip

quit()
