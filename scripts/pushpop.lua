--
-- debugger helpers for pushing and poping
-- data from/to stack/registers/memory
--

function dbg_push(hex)
	r.cmd("!set esp esp-4")
	r.cmd("wx "..hex.." @ "..esp)
end

function dbg_push_reg(flag)
	r.cmd("!set esp esp-4")
	r.cmd("wv "..reg.." @ "..esp)
end

function dbg_pop(addr)
	r.cmd("m 4 "..addr.." @ "..esp)
	r.cmd("!set esp esp+4")
end

function dbg_pop_reg(reg)
	r.cmd("!set reg = [esp]")
	r.cmd("!set esp esp+4")
end

function dbg_call(addr)
	dbg_push_reg("eip")
	r.cmd("!jmp "..addr)
end

function dbg_jmp(addr)
	r.cmd("!jmp "..addr)
end
