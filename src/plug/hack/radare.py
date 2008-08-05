"""
from radare import *

seek(0x1024)
print hex(3)
write("90 90 90")
print hex(3)

quit()

"""
import r
import string
import array

def hex2bin(str):
	return a2b_hex(str.replace(' ',''))

def bin2hex(binstr):
	str = lower(b2a_hex(binstr))
	return str

def __str_to_hash(str):
	list = str.split("\n")
	w = []
	t = {}
	for i in range(1, len(list)):
		w = list[i].split("=")
		if (len(w)>1):
			a = w[0].strip()
			b = w[1].strip()
			if (b[0:2] == '0x'):
				t[a] = long(b,16)
			elif (b[0]>='0' and b[0]<='9'):
				t[a] = long(b,10)
			else:
				t[a] = b
	return t

def analyze_opcode():
	return __str_to_hash(r.cmd("ao"))

def analyze_block():
	return __str_to_hash(r.cmd("ab"))

def endian_set(big):
	r.cmd("eval cfg.endian=%d"%big)

def write(hexpair):
	r.cmd("wx %s"%hexpair)

def write_asm(opcode):
	r.cmd("wa %s"%opcode)

def write_string(str):
	r.cmd("w %s"%str)

def write_wide_string(str):
	r.cmd("ww %s"%str)

def write_from_file(file):
	r.cmd("wf %s"%file)

def write_from_hexpair_file(file):
	r.cmd("wF %s"%file)


def seek_undo():
	r.cmd("undo")

def seek_redo():
	r.cmd("uu")

def seek_history():
	ret = []
	list = r.cmd("u*").split("\n")
	for i in range(1, len(list)):
		w = list[i].split(" ")
		if len(w) > 3:
			t = {}
			t["addr"] = w[0].strip()
			ret.append(t)
	return ret

def seek_history_reset():
	r.cmd("u!")

def write_undo(num):
	return r.cmd("uw %d"%num)

def write_redo(num):
	return r.cmd("uw -%d"%num)

def write_history():
	ret = []
	list = r.cmd("wu").split("\n")
	for i in range(1, len(list)):
		w = list[i].split(" ")
		if len(w) > 3:
			t = {}
			t["size"] = long(w[2].strip(),10)
			t["addr"] = long(w[3].strip(),16)
			# TODO moar nfo here
			ret.append(t)
	return ret

def flag_space_set(name):
	r.cmd("fs %s"%name)

def flag_list():
	ret = []
	list = r.cmd("f").split("\n")
	for i in range(1, len(list)):
		w = list[i].split(" ")
		if len(w) > 3:
			t = {}
			t["addr"] = long(w[1].strip(),16)
			t["size"] = long(w[3].strip(),10)
			t["name"] = w[4].strip()
			ret.append(t)
	return ret

def flag_set(name):
	r.cmd("f %s"%name)

def flag_set_at(name, addr):
	r.cmd("f %s @ 0x%x"%(name,addr))

def flag_rename(old_name, new_name):
	r.cmd("fr %s %s"%(old_name,new_name))

def flag_unset(name):
	r.cmd("f -%s"%name)

def flag_get(name):
	return r.cmd("? %s"%name).split(" ")[0]

def meta_comment_add(msg):
	r.cmd("CC %s"%msg)

def type_code(len):
	r.cmd("Cc %d"%len)

def type_data(len):
	r.cmd("Cd %d"%len)

def type_string(len):
	r.cmd("Cs %d"%len)

def copy(num):
	r.cmd("y %d"%num)

def copy_at(num,addr):
	r.cmd("y %d @ 0x%x"%(num,addr))

def paste():
	r.cmd("yy")

def paste_at(addr):
	r.cmd("yy @ 0x%x"%addr)

def asm(opcode):
	return r.cmd("!rasm '%s'"%opcode)

def dis(num):
	return r.cmd("pd %d"%num)

def dis_at(num,addr):
	return r.cmd("pd %d @ 0x%x"%(num,addr))

def str():
	return r.cmd("pz").strip()

def str_at(addr):
	return r.cmd("pz @ %s"%addr).strip()

def hex(num):
	return r.cmd("pX %d"%num).strip()

def hex_at(num,addr):
	return r.cmd("pX %d @ 0x%x"%(num,addr)).strip()

def eval_get(key):
	return r.cmd("eval %s"%key).strip()

def eval_set(key,value):
	r.cmd("eval %s = %s"%(key,value))

def eval_hash_get():
	return __str_to_hash("e")

def eval_hash_set(hash):
	list = hash.keys()
	for i in range (0, len(list)):
		key = list[i]
		value = hash[key]
		r.cmd("e %s=%s"%(key,value))

def seek(addr):
	r.cmd("s %s"%addr)

def cmp(hexpairs, addr):
	r.cmd("c %s @ 0x%x"%(hexpairs,addr))

def cmp_file(file, addr):
	r.cmd("cf %s @ 0x%x"%(file,addr))

def dbg_attach(pid):
	print r.cmd("!attach %d"%pid)

def dbg_detach(pid):
	print r.cmd("!detach %d"%pid)

def dbg_continue():
	print r.cmd("!cont")

def dbg_step(num):
	if num < 1:
		num = 1
	r.cmd("!step %d"%num)

def dbg_step_over(num):
	if num < 1:
		num = 1
	r.cmd("!stepo %d",num)

def dbg_jmp(addr):
	r.cmd("!jmp "+addr)

def dbg_call(addr):
	r.cmd("!call "+addr)

def dbg_bp_set(addr, type):
	r.cmd("!bp "+addr)

def dbg_bp_unset(addr, type):
	r.cmd("!bp -"+addr)

def dbg_alloc(size):
	return r.cmd("!alloc %s"%size)

def dbg_free(addr):
	r.cmd("!free %s"%addr)

def dbg_backtrace():
	ret = []
	list = r.cmd("!bt").split("\n")
	for i in range(1, len(list)):
		w = list[i].split(" ")
		if len(w) > 3:
			t = {}
			t["addr"]    = long(w[1].strip(),16)
			t["framesz"] = long(w[2].strip(),10)
			t["varsz"]   = long(w[3].strip(),10)
			ret.append(t)
	return ret

def dbg_dump(name):
	r.cmd("!dump %s"%name)

def dbg_restore(name):
	r.cmd("!restore %s"%name)

def dbg_register_get(name):
	r.cmd("!reg %s"%(name))

def dbg_register_set(name, value):
	r.cmd("!reg %s=%s"%(name,value))

def hash(algo,size):
	return r.cmd("#%s %d"%(algo,size))

def graph():
	r.cmd("ag")

def graph_at(at):
	r.cmd("ag @ %s"%at)

def quit():
	r.cmd("q")
