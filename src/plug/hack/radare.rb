=begin

Ruby API for radare scripting plugin

=end

class Radare
 def seek(addr)
  r.cmd("s %s"%addr)
 end
end
