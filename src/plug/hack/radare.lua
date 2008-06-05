-- radare lua api interface
--
-- 2008 pancake <youterm.com>


-- define namespaces
Radare = {}
Radare.Print = {}
Radare.Search = {}
Radare.Config = {}
Radare.Code = {}
Radare.Hash = {}
Radare.Debugger = {}
Radare.Write = {}

-- define aliases
r = Radare
p = Radare.Print
cfg = Radare.Config
code = Radare.Code
hash = Radare.Hash
s = Radare.Search
d = Radare.Debugger
w = Radare.Write

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

-- join strings from a table
function join(delimiter, list)
  local len = getn(list)
  if len == 0 then 
    return "" 
  end
  local string = list[1]
  for i = 2, len do 
    string = string .. delimiter .. list[i] 
  end
  return string
end

-- split a string by a separator
function split(text, sep)
	sep = sep or "\n"
	text = chomp(text)
	local lines = {}
	local pos = 1
	while true do
		local b,e = text:find(sep, pos)
		if not b then table.insert(lines, text:sub(pos)) break end
		table.insert(lines, text:sub(pos,b-1))
		pos = e + 1
	end
	return lines
end

function chomp(text)
	if text == nil then return "" end
	return string.gsub(text, "\n$", "")
end

-- Radare api functions

function Radare.get(value)
 	-- | cut -d ' ' -f 1");
	foo = split(
		string.gsub(
		  cmd_str("? "..value),'(0x[^ ])', 
			function(x)return x end),';')
	return tonumber(foo[1])
end

function Radare.bytes(text)
	local res = split(Radare.cmd("pX @"..text), " ")
	-- TODO
	return res;
end

function Radare.cmd(cmd)
	return chomp(cmd_str(cmd))
end

function Radare.iosystem(command)
	r.cmd("!"..command)
	-- todo handle errors here
	return 0
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

function Radare.attach(pid)
	return r.cmd("o pid://"..pid)
end

function Radare.debug(filename)
	return r.cmd("o dbg://"..filename)
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

function Radare.exit()
	return r.quit()
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

function Radare.Print.dis(nops, address)
	if size == nil then size = "" end
	if address == nil then
		return r.cmd("pd "..nops)
	else
		return r.cmd("pd "..nops.." @ "..address)
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

function Radare.Search.parse(string)
	local res = split(string,"\n")
	local ret = {}
	for i = 1, #res do
		local line = split(res[i], " ")
		ret[i] = line[3]
	end
	return ret;
end

function Radare.Search.string(string)
	return Radare.Search.parse(Radare.cmd("/ "..string))
end

function Radare.Search.hex(string)
	return Radare.Search.parse(Radare.cmd("/x "..string))
end


-- write stuff

function Radare.Write.hex(string, address)
	if address == nil then
		return r.cmd("wx "..string)
	else
		return r.cmd("wx "..string.." @ "..address)
	end
end

function Radare.Write.string(string, address)
	if address == nil then
		return r.cmd("w ", string)
	else
		return r.cmd("w "..string.." @ "..address)
	end
end

function Radare.Write.wide_string(string, address)
	if address == nil then
		return r.cmd("ws "..string)
	else
		return r.cmd("ws "..string.." @ "..address)
	end
end

function Radare.Write.asm(string, address)
	if address == nil then
		return r.cmd("wa ".. string)
	else
		return r.cmd("wa "..string.." @ "..address)
	end
end

function Radare.Write.rscasm(string, address)
	if address == nil then
		return r.cmd("wA "..string)
	else
		return r.cmd("wA "..string.." @ "..address)
	end
end

function Radare.Write.from_file(filename, address)
	if address == nil then
		return r.cmd("wf "..filename)
	else
		return r.cmd("wf "..filename.." @ "..address)
	end
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
	--if size == nil then size = "" end
	--if address == nil then return r.cmd("#sha512 "..size) end
	--return r.cmd("#sha512 "..size.."@"..address)
end

-- code api
function Radare.Code.comment(offset, message)
	-- TODO: if only offset passed, return comment string
	r.cmd("CC "..message.." @ "..offset)
	return Radare.Code
end

function Radare.Code.code(offset, len)
	r.cmd("Cc "..len.." @ "..offset)
	return Radare.Code
end

function Radare.Code.data(offset, len)
	r.cmd("Cd "..len.." @ "..offset)
	return Radare.Code
end

function Radare.Code.string(offset, len)
	r.cmd("Cs "..len.." @ "..offset)
	return Radare.Code
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

function Radare.Debugger.step(num)
	r.cmd("!step "..num)
	return Radare.Debugger
end

function Radare.Debugger.step(num)
	r.cmd("!step "..num)
	return Radare.Debugger
end

function Radare.Debugger.step_over()
	r.cmd("!stepo");
	return Radare.Debugger
end

function Radare.Debugger.step_until_user_code()
	r.cmd("!stepu");
	return Radare.Debugger
end

function Radare.Debugger.add_bp(addr)
	r.cmd("!bp "..num)
	return Radare.Debugger
end

function Radare.Debugger.remove_bp(addr)
	r.cmd("!bp -"..num)
	return Radare.Debugger
end

function Radare.Debugger.alloc(size)
	return cmd_str("!alloc "..size)
end

function Radare.Debugger.free(addr) -- rename to dealloc?
	return cmd_str("!free "..addr)
end

function Radare.Debugger.dump(dirname)
	r.cmd("!dump "..dirname)
	return Radare.Debugger
end

function Radare.Debugger.restore(dirname)
	r.cmd("!restore "..dirname)
	return Radare.Debugger
end

function Radare.Debugger.jump(addr)
	r.cmd("!jmp "..addr)
	return Radare.Debugger
end

function Radare.Debugger.backtrace()
	local res = split(Radare.cmd("!bt"),"\n")
	local ret = {}
	for i = 1, #res do
		local line = split(res[i], " ")
		ret[i] = line[2]
	end
	return ret;
end

print "=> Type 'help()' or 'quit' to return to radare shell."
