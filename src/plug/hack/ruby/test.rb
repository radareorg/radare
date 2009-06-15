require 'radare/api'
require 'radare/remote'

print "#{bin2hex("\x90\x12\x33")}\n"

rs = RapServer.new()
rs.listen_tcp(9999)


rc = RapClient.new()
rc.connect_tcp("127.0.0.1", 5555)
fd = rc.open("/bin/ls", 0)
rc.close(fd)
