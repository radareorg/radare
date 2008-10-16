=begin

Ruby API for radare scripting plugin

=end

class Radare

 # helpers
 def str2hash(str)
   t = {}
   list = str.split("\n")
   list.each do |item|
     w = item.split("=")
     if (w.len>1) do
     end
   end
   return t
 end

 def hex2bin(str)
   return 
 end

 # core
 def seek(addr)
  r.cmd("s %s"%addr)
 end

 def seek(addr)
  r.cmd("s %s"%addr)
 end

 # debugger
 def seek(addr)
  r.cmd("s %s"%addr)
 end

end
