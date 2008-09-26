#!/usr/bin/python
# pancake/sexy-pandas

import cpu
import dbg
import bx
import radapy
from string import *

PORT=9998

# physical memory
phys=True

def fun_read(len):
	global phys
	if phys:
		return dbg.read_memory_block_physical(radapy.offset, len)
	return dbg.read_memory_block_linear(radapy.offset, len)

def fun_write(buf):
	global phys
	if phys:
		return dbg.write_memory_block_physical(radapy.offset, buf)
	return dbg.write_memory_block_linear(radapy.offset, buf)

def fun_system(str):
	global phys
	foo = str.split(' ')
	str = ""
	try:
		if foo[0] == "help":
			str  = "Bochs-python remote debugger\n"
			str += " !regs                  : show registers\n"
			str += " !st                    : print stack\n"
			str += " !bp                    : breakpoints\n"
			str += " !cont                  : continue execution\n"
			str += " !step [n]              : perforn N steps\n"
			str += " !stepo [n]             : step over\n"
			str += " !mem [physical|linear] : select memory addressing\n"
		elif foo[0] == "bp":
			# TODO: rm bps + hw
			try:
				bx.pbreakpoint(int(foo[1]))
			except:
				return "Oops"
			return bx.info_bpoints()
		elif foo[0] == "st":
			return bx.print_stack()
		elif foo[0] == "regs":
			return bx.info_registers()
		elif foo[0] == "cont":
			bx.cont()
			return ""
		elif foo[0] == "step":
			try:
				bx.stepN(int(foo[1]))
			except:
				bx.stepN(1)
				return ""
		elif foo[0] == "stepo":
			try:
				bx.step_over()
			except:
				bx.step_over()
			return ""
		elif foo[0] == "mem":
			try:
				if str[1] == "physical":
					phys = True
					return "Using physical memory addressing\n"
				if str[1] == "linear":
					phys = True
					return "Using linear memory addressing\n"
			except:
				return "Use !mem [physical|linear]\n"
	except:
		str = "Oops"
	return str

radapy.handle_cmd_system = fun_system
radapy.handle_cmd_read = fun_read
radapy.handle_cmd_write = fun_write

radapy.listen_tcp (PORT)
