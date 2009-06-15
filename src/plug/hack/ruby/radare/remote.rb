require 'socket'

RAP_OPEN = 1
RAP_READ = 2
RAP_WRITE = 3
RAP_SEEK = 4
RAP_CLOSE = 5
RAP_SYSTEM = 6
RAP_CMD = 7
RAP_REPLY = 0x80

class RapServer
	def initialize()
		# TODO
	end

	def handle_open(file, flags)
		print "FILE #{file}\n"
		print "FLAGS #{flags}\n"
		return 0 
	end

	def handle_read(length)
		print "READ #{length}\n"
		return "patata"
	end

	def handle_seek(offset, type)
		case type
		when 0
			return offset
		when 1
			return seek+offset
		when 2
			return 6
		end
		return offset
	end

	def handle_packet(c, packet)
		case packet[0]
		when RAP_OPEN
			flags = c.read(1)
			length = c.read(1)
			file = c.read(length[0].to_i)
			ret = handle_open(file, flags[0].to_i)
			buf = [RAP_OPEN|RAP_REPLY, ret].pack("CN")
			c.write(buf)
		when RAP_READ
			length = c.read(4).unpack("N")[0]
			ret = handle_read(length)
			buf = [RAP_READ|RAP_REPLY, ret.length].pack("CN")
			c.write(buf)
			c.write(ret)
		when RAP_SEEK
			type = c.read(1)[0].to_i
			offset = c.read(8).unpack("Q")[0]
			seek = handle_seek(offset, type)
			# seek = seek.to_s.reverse.to_i
			# TODO: swap seek value (64 bit big endian!)
			buf = [RAP_SEEK|RAP_REPLY, seek].pack("CQ")
			c.write(buf)
		when RAP_SYSTEM
			length = c.read(4).unpack("N").to_i
			buf = c.read(length)
			str = handle_system(buf)
			buf = [RAP_SEEK|RAP_REPLY, seek].pack("CQ")
			c.write(buf)
			c.write(str)
		when RAP_CLOSE
			fd = c.read(4).unpack("N")
			handle_close(fd);
			buf = [RAP_SEEK|RAP_REPLY, 0].pack("CC")
			c.write(buf)
		end
	end

	def handle_client(client)
		while true
			handle_packet(client, client.read(1))
		end
	end

	def listen_tcp(port)
		server = TCPServer.new('0.0.0.0', port)
		while(client=server.accept)
			handle_client(client)
		end
	end
end

class RapClient
	def initialize()
		# TODO
	end
	def open(file, flags)
		print "Opening #{file}\n"
		fd = -1
		fd
	end
	def connect_tcp(host, port)
		print "==> Connecting to #{host}\n"
	end
	
	def close(fd)
		print "==> Close\n"
	end
end
