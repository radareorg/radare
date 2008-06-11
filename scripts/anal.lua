-- Example of lua script using the code analysis API

foo = Radare.Analyze.opcode()

print("Key Values of table returned by a.opcode()")
for k,v in pairs(foo) do
	print (" - "..k.." = "..v)
end

print("Opcode size: "..foo["size"])
