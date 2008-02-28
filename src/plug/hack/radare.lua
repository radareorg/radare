-- radare lua api interface
--
-- 2008 pancake <youterm.com>


-- define namespaces
Radare = {}
Radare.Print = {}
Radare.Search = {}
Radare.Config = {}
Radare.Hash = {}
Radare.Debugger = {}

-- define aliases
r = Radare
p = Radare.Print
c = Radare.Config
k = Radare.Hash
s = Radare.Search
d = Radare.Debugger

-- General use functions

-- show help for radare lua api
function help(table)
	if table == nil then
		print "Use help(Radare), help(Radare.Debugger) or help(Radare.Print)"
		print "These namespaces has been aliased as 'r', 'd' and 'p'."
	else
		for key,val in pairs(table) do print("  "..key) end
	end
	return 0
end

function list(table)
	local i
	i = 0
	if table == nil then
		print "List the contents of a table"
	else
		--for key,val in pairs(table) do print("  "..key) end
		for k,v in pairs(table) do
			if v == nil then
				print("  "..k) -- XXX crash
			else
				print("  "..k..": "..v)
			end
			i = i + 1
		end
	end
	return n
end


-- split a string by a separator
function split(text, sep)
	sep = sep or "\n"
	local lines = {}
	local pos = 1
	while true do
		local b,e = text:find(sep, pos)
		if not b then table.insert(lines, text:sub(pos)) break end
		table.insert(lines, text:sub(pos, b-1))
		pos = e + 1
	end
	return lines
end

-- Radare api functions

function Radare.cmd(cmd)
	return radare_cmd_str(cmd)
end

function Radare.system(command)
	r.cmd("!!"..command)
	-- todo handle errors here
	return 0
end

function Radare.open(filename)
	r.cmd("o "..filename)
	-- todo handle errors here
	return 0
end

function Radare.undo_seek()
	r.cmd("u")
	-- todo handle errors here
	return 0
end

function Radare.redo_seek()
	r.cmd("uu")
	-- todo handle errors here
	return 0
end

function Radare.resize(newsize)
	r.cmd("r "..newsize)
	-- todo handle errors here
	return 0
end

function Radare.fortune()
	return r.cmd("fortune")
end

function Radare.seek(offset)
	r.cmd("s "..offset)
	return 0
end

function Radare.interpret(file)
	-- control block size
	r.cmd(". "..file)
	return 0
end

function Radare.copy(size,address)
	-- control block size
	if address == nil then
		r.cmd("y "..size)
	else
		r.cmd("y "..size.." @ "..address)
	end
	return 0
end

function Radare.paste(address)
	-- control block size
	if address == nil then
		r.cmd("yy ")
	else
		r.cmd("yy @ "..address)
	end
	r.cmd("y "..offset)
	return 0
end

function Radare.endian(big)
	r.cmd("eval cfg.endian = "..big)
	return 0
end

function Radare.flag(name, address) -- rename to r.set() ?
	if address == nil then
		r.cmd("f "..name)
	else
		r.cmd("f "..name.." @ "..address)
	end
	return 0
end

function Radare.flag_get(name) -- rename to r.get() ?
	local foo = str.split(r.cmd("? "..name), " ")
	return foo[1]
end

function Radare.flag_remove(name) -- rename to r.remove() ?
	r.cmd("f -"..name)
	return 0
end

function Radare.flag_rename(oldname, newname)
	r.cmd("fr "..oldname.." "..newname)
	return 0
end

function Radare.flag_list(filter)
	local list = split(r.cmd("f"))
	local ret = {}
	local i = 1
	while list[i] ~= nil do
		local foo = split(list[i], " ")
		ret[i] = foo[4]
		i = i + 1
	end
	return ret
end

function Radare.eval(key, value)
	if value == nil then
		return r.cmd("eval "..key)
	end
	return r.cmd("eval "..key.." = "..value)
end

function Radare.cmp(value, address)
	if address == nil then
		r.cmd("c "..value)
	else
		r.cmd("c "..value.." @ "..address)
	end
	-- parse output and get ret value
	return 0
end

function Radare.cmp_file(file, address)
	if address == nil then
		r.cmd("cf "..file)
	else
		r.cmd("cf "..file.." @ "..address)
	end
	-- parse output and get ret value
	return 0
end

function Radare.quit()
	r.cmd("q");
	return 0
end


-- Radare.Debugger API

function Radare.Debugger.step(times)
	r.cmd("!step "..times);
	return Radare.Debugger
end

function Radare.Debugger.attach(pid)
	r.cmd("!attach "..pid);
	return Radare.Debugger
end

function Radare.Debugger.detach(pid)
	r.cmd("!detach")
	return Radare.Debugger
end

function Radare.Debugger.jmp(address)
	r.cmd("!jmp "..address)
	return Radare.Debugger
end

function Radare.Debugger.set(register, value)
	r.cmd("!set "..register.." "..value)
	return Radare.Debugger
end

function Radare.Debugger.call(address)
	r.cmd("!call "..address)
	return Radare.Debugger
end

function Radare.Debugger.dump(name)
	r.cmd("!dump "..name)
	return Radare.Debugger
end

function Radare.Debugger.restore(name)
	r.cmd("!restore "..name)
	return Radare.Debugger
end

function Radare.Debugger.bp(address)
	r.cmd("!bp "..address)
	return Radare.Debugger
end

-- print stuff

function Radare.Print.hex(size, address)
	if size == nil then size = "" end
	if address == nil then
		return r.cmd(":pX "..size)
	else
		return r.cmd(":pX "..size.." @ "..address)
	end
end

function Radare.Print.disasm(size, address)
	if size == nil then size = "" end
	if address == nil then
		return r.cmd("pD "..size)
	else
		return r.cmd("pD "..size.." @ "..address)
	end
end

function Radare.Print.bin(size, address) -- size has no sense here
	if size == nil then size = "" end
	if address == nil then
		return r.cmd(":pb "..size)
	else
		return r.cmd(":pb "..size.." @ "..address)
	end
end

function Radare.Print.string(address) -- size has no sense here
	if address == nil then
		return r.cmd("pz ")
	else
		return r.cmd("pz @ "..address)
	end
end

function Radare.Print.oct(size,address) -- size has no sense here
	if size == nil then size = "" end
	if address == nil then
		return r.cmd(":po "..size)
	end
	return r.cmd(":po "..size.."@ "..address)
end

-- search stuff

function Radare.Search.string(string)
	r.cmd("/ "..string)
end

function Radare.Search.hex(string)
	r.cmd("/x "..string)
end

-- config stuff

-- eval like
function Radare.Config.set(key, val)
	r.cmd("eval "..key.."="..val)
	return val
end

function Radare.Config.color(value)
	r.cmd("eval scr.color ="..value)
	return value
end

function Radare.Config.get(key)
	return r.cmd("eval "..key)
end

function Radare.Config.limit(sizs)
	return r.cmd("eval cfg.limit = "..size)
end

-- crypto stuff

function Radare.Hash.md5(size, address)
	if size == nil then size = "" end
	if address == nil then return r.cmd("#md5 "..size) end
	return r.cmd("#md5 "..size.."@"..address)
end

function Radare.Hash.crc32(size, address)
	if size == nil then size = "" end
	if address == nil then return r.cmd("#crc32 "..size) end
	return r.cmd("#crc32 "..size.."@"..address)
end

function Radare.Hash.md4(size, address)
	if size == nil then size = "" end
	if address == nil then return r.cmd("#md4 "..size) end
	return r.cmd("#md4 "..size.."@"..address)
end

function Radare.Hash.sha1(size, address)
	if size == nil then size = "" end
	if address == nil then return r.cmd("#sha1 "..size) end
	return r.cmd("#sha1 "..size.."@"..address)
end

function Radare.Hash.sha256(size, address)
	if size == nil then size = "" end
	if address == nil then return r.cmd("#sha256 "..size) end
	return r.cmd("#sha256 "..size.."@"..address)
end

function Radare.Hash.sha384(size, address)
	if size == nil then size = "" end
	if address == nil then return r.cmd("#sha384 "..size) end
	return r.cmd("#sha384 "..size.."@"..address)
end

function Radare.Hash.sha512(size, address)
	if size == nil then size = "" end
	if address == nil then return r.cmd("#sha512 "..size) end
	return r.cmd("#sha512 "..size.."@"..address)
end

function Radare.Hash.hash(algo, size, address)
	if size == nil then size = "" end
	eval("#"..algo.." "..size)
end

function Radare.Hash.sha512(size, address)
	return hash("sha512", size, address)
	if size == nil then size = "" end
	if address == nil then return r.cmd("#sha512 "..size) end
	return r.cmd("#sha512 "..size.."@"..address)
end

-- change a signal handler of the child process
function Radare.Debugger.signal(signum, sighandler)
	r.cmd("!signal "..signum.." "..sighandler)
	return Radare.Debugger
end

function Radare.Debugger.bp_remove(address)
	r.cmd("!bp -"..address);
	return Radare.Debugger
end

function Radare.Debugger.continue(address)
	if address == nil then
		r.cmd("!cont"); 
	else
		r.cmd("!cont "..address);
	end
	return Radare.Debugger
end

print "=> Type 'help()' or 'quit' to return to radare shell."
