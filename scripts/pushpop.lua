function push(hex)
	-- TODO: support registers
	-- needs 
	r.cmd("!set esp esp-4")
	r.cmd("wx "..hex.." @ "..esp)
end

function pop(addr)
	-- TODO: support registers
	r.cmd("m 4 "..addr.." @ "..esp)
	r.cmd("!set esp esp+4")
end
