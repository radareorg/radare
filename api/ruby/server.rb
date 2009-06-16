require 'radare/api'
require 'radare/remote'

#print "#{bin2hex("\x90\x12\x33")}\n"

class MyRapServer < RapServer
  def handle_open(file, args)
    print "OPEN HOOKED\n"
    return super(file, args)
  end
end

rs = MyRapServer.new()
rs.listen_tcp(9999)

