require 'radare/api'
require 'radare/remote'

print "#{bin2hex("\x90\x12\x33")}\n"


def hook_system(cmd)
	print "==> SYSTEM HANDLED\n"
	0
end
rs = RapServer.new()
#rs.handle_system = hook_system
rs.listen_tcp(9999)


rc = RapClient.new()
rc.connect_tcp("127.0.0.1", 5555)
fd = rc.open("/bin/ls", 0)
rc.close(fd)
