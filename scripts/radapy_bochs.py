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
bps=list()

def reg_resolver(reg):
	return eval("cpu.%s"%upper(reg))

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
		if foo[0] == "help" or foo[0] == "?" or foo[0] == "h":
			str  = "Bochs-python remote debugger\n"
			str += " !?                     : alias for !help\n"
			str += " !reg [reg] ([value])   : get/set CPU registers\n"
			str += " !regs[*]               : show CPU registers\n"
			str += " !cregs                 : show control registers\n"
			str += " !fpregs                : show FPU registers\n"
			str += " !st                    : print stack\n"
			str += " !bp [[-]addr]          : breakpoints\n"
			str += " !cont                  : continue execution\n"
			str += " !step [n]              : perforn N steps\n"
			str += " !stepo [n]             : step over\n"
			str += " !mem [physical|linear] : select memory addressing\n"
			str += " !exec [python-expr]    : execute python expression remotely\n"
		elif foo[0] == "bp":
			try:
				if foo[1][0] == '-':
					addr = eval(foo[1][1:])
					bx.del_breakpoint(addr)
					str = "Breakpoint removed at 0x%08x\n"%addr
					bps.remove(addr)
				else:
					try:
						addr = eval(foo[1])
						if phys:
							bx.pbreakpoint(0, addr)
						else:
							bx.lbreakpoint(0, addr)
						str = "Breakpoint add at 0x%08x\n"%addr
						bps.append(addr)
					except:
						bx.info_bpoints()
			except:
				bx.info_bpoints()
				i = 0
				str = ""
				for bp in bps:
					str += " %d  0x%08x\n"%(i,bp)
					i = i + 1
		elif foo[0] == "st":
			# TODO
			bx.print_stack()
		elif foo[0] == "exec":
			exec(join(foo[1:]))
		elif foo[0] == "reg":
			try:
				reg = reg_resolver(foo[1])
				try:
					cpu.set(reg, eval(foo[2]))
					return ""
				except:
					return "0x%08x\n"%cpu.get(reg)
			except:
				return "Usage: !reg eax 33\n"
		elif foo[0] == "cregs":
			bx.info_control_regs()
			str  = "CR0 = 0x%x\n"%cpu.get(cpu.CR0)
			str += "CR2 = 0x%x\n"%cpu.get(cpu.CR2)
			str += "CR3 = 0x%x\n"%cpu.get(cpu.CR3)
			str += "CR4 = 0x%x\n"%cpu.get(cpu.CR4)
		elif foo[0] == "regs":
			str  = "  eax  0x%08x    esi  0x%08x    eip     0x%08x\n"%(cpu.get(cpu.EAX),cpu.get(cpu.ESI),cpu.get(cpu.EIP))
			str += "  ebx  0x%08x    edi  0x%08x    oeax    0x%08x\n"%(cpu.get(cpu.EBX),cpu.get(cpu.EDI),cpu.get(cpu.EAX))
			str += "  ecx  0x%08x    esp  0x%08x    eflags  0x%08x\n"%(cpu.get(cpu.ECX),cpu.get(cpu.ESP),cpu.get(cpu.EFLAGS))
			str += "  edx  0x%08x    ebp  0x%08x    cr0     0x%08x\n"%(cpu.get(cpu.EDX),cpu.get(cpu.EBP),cpu.get(cpu.CR0))
			str += "  dr0 0x%08x   dr1 0x%08x   dr2 0x%08x   dr3 0x%08x\n"%(cpu.get(cpu.DR0),cpu.get(cpu.DR1),cpu.get(cpu.DR2),cpu.get(cpu.DR3))
		elif foo[0] == "regs*":
			str = "f eip @ 0x%x\n"%(cpu.get(cpu.EIP)+cpu.get(cpu.CS_base))
			str += "f esp @ 0x%x\n"%cpu.get(cpu.ESP)
			str += "f ebp @ 0x%x\n"%cpu.get(cpu.EBP)
			str += "f eax @ 0x%x\n"%cpu.get(cpu.EAX)
			str += "f ebx @ 0x%x\n"%cpu.get(cpu.EBX)
			str += "f ecx @ 0x%x\n"%cpu.get(cpu.ECX)
			str += "f edx @ 0x%x\n"%cpu.get(cpu.EDX)
			str += "f esi @ 0x%x\n"%cpu.get(cpu.ESI)
			str += "f edi @ 0x%x\n"%cpu.get(cpu.EDI)
			return str
		elif foo[0] == "fpregs":
			# TODO
			bx.info_registers(2)
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
			bx.step_over()
			return ""
		elif foo[0] == "mem":
			try:
				if foo[1] == "physical":
					phys = True
					return "Using physical memory addressing\n"
				if foo[1] == "linear":
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

print "Listening at port %d\n"%PORT
radapy.listen_tcp (PORT)
