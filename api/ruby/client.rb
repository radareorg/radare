require 'radare/api'
require 'radare/remote'

rc = RapClient.new()
rc.connect_tcp("127.0.0.1", 9999)
fd = rc.open("/bin/ls", 0)
buf = rc.read(10)
print Radare.bin2hex(buf)
rc.close(fd)
