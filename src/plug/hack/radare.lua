-- radare lua api interface
--
-- 2008 pancake <youterm.com>


-- define namespaces
Radare = {}
Radare.Print = {}
Radare.Debugger = {}

-- define aliases
r = Radare
p = Radare.Print
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

function Radare.Debugger.step()
	r.cmd("!step");
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
