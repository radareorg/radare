print ""
print ""

print "Welcome to the debugging in LUA"
print ""

-- show lua help
--help(Radare)


-- search lib and process results
cmd("s 0")
local res = split(cmd_str("/ lib"), "\n")

-- iterate search results
for i=1, #res-1 do
	-- split string
	local line = split(res[i], " ")
	print (line[3].." - "..line[1])
	print (" => "..cmd_str("pz @ "..line[3]))
end

-- print("result: ("..result..")")
cmd("eval scr.width=40")

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
