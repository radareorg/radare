---------------------------------------------
-- Example injecting a file and adding a hook
--

function hook_x86(from, to, file)
	r.cmd("wf "..file.." @ "..to)
	r.cmd("wa jmp "..from)
end

hook_x86(0x8048000, 0x8049000, "/tmp/file")

