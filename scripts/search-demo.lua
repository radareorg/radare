print ""

print "Welcome to the debugging in LUA"
print ""

-- show lua help
--help(Radare)


-- search lib and show results
Radare.seek(0)
local hits = Radare.Search.string("lib")
for i = 1, #hits do
	print(" => "..hits[i]..": "..Radare.cmd("pz @ "..hits[i]))
end

print ""

-- print("result: ("..result..")")
r.eval("scr.width", "40")

-- enter commandline loop
while true do
	print "<radare-lua-dbg>"
	line = io.read()
--	regs = cmd_str("!regs")
--	print(regs)
	print (cmd_str(line))
end

-- exit
cmd("q")
