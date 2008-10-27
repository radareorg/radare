=begin

Ruby API for radare scripting plugin

author: pancake <nopcode.org>

=end

# This class is instantiated as $r = Radare.new()
class Radare
 # helpers
 def str2hash(str)
   t = {}
   list = str.split("\n")
   list.each do |item|
     w = item.split("=")
     if w.size > 1 then
       t[w[0]]=w[1]
     end
   end
   return t
 end

 def hex2bin(str)
   return str
 end

 # core
 def seek(addr)
  $r.cmd("s %s"%addr)
 end

 def seek(addr)
  $r.cmd("s %s"%addr)
 end

 # code
 def comment_add(addr, str)
  $r.cmd("CC #{str} @ 0x%08llx"%addr)
 end

 def comment_del(str)
  $r.cmd("CC -#{str}");
 end

 def analyze_opcode(addr)
  begin
   return str2hash($r.cmd("ao @ 0x%08x"%addr))
  rescue
   return str2hash($r.cmd("ao @ #{addr}"))
  end
 end

 # debugger
 def step(addr)
  $r.cmd("!step")
 end

 def continue()
  $r.cmd("!cont")
 end

 def until(addr)
  $r.cmd("!cont #{addr}")
 end

end

puts "Load done"
