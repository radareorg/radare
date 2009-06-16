--
-- 2008 th0rpe <nopcode.org>
--
-- P0C loop entry
--
--

entry_addr = "0x00401050"
jump_addr = "0x004010b7"

do 
	local ret;
	local buf_addr;
	local lebuf_addr;
	local param_addr;
	local addr;

	-- set breakpoint at entry address
	cmd("!bp "..entry_addr);
	-- continue execution
	cmd("!cont");
	-- remove breakpoint at entry address
	cmd("!bp -"..entry_addr);

	--alloc memory
	buf_addr = string.format("0x%x", cmd("!alloc 4096"));

	--get stack frame address
	param_addr = string.format("0x%x", cmd("!get esp") + 12);

	-- translate to little endian 
	lebuf_addr = "0x"..string.sub(buf_addr, 9, 10)..
		     string.sub(buf_addr, 7, 8)..
		     string.sub(buf_addr, 5, 6)..
		     string.sub(buf_addr, 3, 4);

	--seek at 3 arg
	cmd("s "..param_addr);

	--write buffer address(allocated)
	cmd("w "..lebuf_addr);

	-- init randomize seed
	math.randomseed(os.time());

	--seek at buffer
	cmd("s "..buf_addr);

	--write random sequence
	cmd("w "..string.format("0x%x%x",math.random(1,255), math.random(1,255)));
	--begin loop
	ret = cmd("!loop 0x"..jump_addr);
	while(ret == 0) do
		cmd("w "..string.format("0x%x%x",math.random(1,255), math.random(1,255)));

		-- call again
		ret = cmd("!loop 0x"..jump_addr);
	end

	if ret == 3 then
		print "fatal exception (possible bug)";
	else if ret == 2 then
		print "exit program";
		end
	end

end
