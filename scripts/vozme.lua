-- vozme.lua
--
--   author: pancake<youterm.com>
--
-- lua/radare script to read (using vozme web service) opcodes from current seek
--
-- nicely for blind reversing. i did it for the lulz
--

require 'posix'

lang="es"
n_opcodes = 4
url="vozme.com/text2voice.php?lang="..lang

function play(text, bg)
	if bg then bg = "&" else bg="" end
	if text == "" then return end
	os.execute("mplayer -quiet -msglevel all=0 "..text.." > /dev/null 2>&1"..bg)
	print("mplayer -quiet -msglevel all=0 "..text.." > /dev/null 2>&1"..bg)
end

function play_queue(str, text)
	return str.."`curl -s -d \"text="..chop(text).."\" "..url.." | grep 'mp3\"' | cut -d '\"' -f 2` "
end

-- opcode player

Radare.Config.verbose(0)
local lines = split(Radare.Print.dis(n_opcodes),'\n')
local str = ""
for i = 1, #lines do str = play_queue(str, lines[i]) end
play(str, true)
Radare.Config.verbose(3)
