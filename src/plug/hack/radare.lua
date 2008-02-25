Radare = {}

function Radare.open(filename)
	print ("Radare.open: " .. filename)
	return 0
end

r = Radare

-- my program example --

r.open("dbg:///bin/ls")
r.cmd("!bp entry");
r.cmd("!cont");

while 1 do
	r.cmd("!contsc")
	if r.cmp("4 @ [esp]") then
		print "I found a breakpoint here"
	end
end

r.exit()
