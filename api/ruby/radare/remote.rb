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
	# constructor
	def initialize()
		# TODO
	end

	def handle_open(file, flags)
		print "FILE #{file}\n"
		print "FLAGS #{flags}\n"
		return 0 
	end

	def handle_system(cmd)
		print "SYSTEM #{cmd}\n"
		return ""
	end

	def handle_close(fd)
		print "Connection closed\n"
		return ""
	end

	def handle_cmd(cmd)
		print "CMD #{cmd}\n"
		return ""
	end

	def handle_read(length)
		print "READ #{length}\n"
		return "patata"
	end

	def handle_write(buffer)
		print "WRITE #{buffer.length}\n"
		return "patata"
	end

	def handle_lseek(offset, type)
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
			flags = c.read(1)[0].to_i
			length = c.read(1)[0].to_i
			file = c.read(length)
			ret = handle_open(file, flags)
			buf = [RAP_OPEN|RAP_REPLY, ret].pack("CN")
			c.write(buf)
		when RAP_READ
			length = c.read(4).unpack("N")[0]
			ret = handle_read(length)
			buf = [RAP_READ|RAP_REPLY, ret.length].pack("CN").concat(ret)
			buf.slice(0, length)
			c.write(buf)
		when RAP_WRITE
			length = c.read(4).unpack("N")[0].to_i
			buf = c.read(length)
			ret = handle_write(buf)
			buf = [RAP_WRITE|RAP_REPLY, ret].pack("CN")
			c.write(buf)
		when RAP_SEEK
			type = c.read(1).unpack("C")[0].to_i
			offset = c.read(8).unpack("Q")[0]
			seek = handle_lseek(offset, type)
			# seek = seek.to_s.reverse.to_i
			# TODO: swap seek value (64 bit big endian!)
			sbuf = [seek].pack("Q").reverse
			#print "SBUF #{sbuf} (#{seek})\n"
			buf = [RAP_SEEK|RAP_REPLY].pack("C").concat(sbuf)
			c.write(buf)
		when RAP_SYSTEM
			length = c.read(4).unpack("N")[0].to_i
			buf = c.read(length)
			str = handle_system(buf)
			buf = [RAP_SYSTEM|RAP_REPLY, str.length].pack("CN").concat(str)
			c.write(buf)
		when RAP_CMD
			length = c.read(4).unpack("N")[0].to_i
			buf = c.read(length)
			str = handle_cmd(buf)
			buf = [RAP_CMD|RAP_REPLY, str.length].pack("CN").concat(str)
			c.write(buf)
		when RAP_CLOSE
			fd = c.read(4).unpack("N")[0].to_i
			handle_close(fd);
			buf = [RAP_SEEK|RAP_REPLY, 0].pack("CC")
			c.write(buf)
		end
	end

	def handle_client(client)
		while ((cmd=client.read(1)) != nil)
			handle_packet(client, cmd)
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
	fd = -1 # huh?

	def initialize()
		# TODO
	end

	def open(file, flags)
		print "Opening #{file}\n"
		buf = [RAP_OPEN, flags, file.length].pack("CCC")
		buf.concat(file)
		@fd.write(buf)
		# parse reply
		buf = @fd.read(1)
		fh = @fd.read(4).unpack("N")[0]
		print "FH=#{fh}\n"
		fh
	end

	def connect_tcp(host, port)
		print "==> Connecting to #{host}:#{port}\n"
		@fd = TCPSocket.new(host, port)
	end

	def disconnect()
		@fd.close
	end

	def lseek(addr, type)
		print "lseek #{addr}\n"
		buf = [RAP_SEEK, type].pack("CC")
		sbuf = [addr].pack("Q").reverse # big endian u64
		buf.concat(sbuf)
		@fd.write(buf)
		# read reply
		buf = @fd.read(1)
		ret = @fd.read(8).reverse.unpack("Q")[0]
		print "lseek #{ret}\n"
		ret
	end

	def write(buffer)
		# TODO
		print "WRITE #{buffer.length}\n"
		buf = [RAP_WRITE,buffer.length].pack("CN").concat(buffer)
		@fd.write(buf)
		# read reply
		@fd.read(1) # must be RAP_WRITE|RAP_REPLY
		ret = @fd.read(4).unpack("N")[0]
		print "WRITE RET IS %d\n"%ret
		ret
	end

	def read(len)
		buf = [RAP_READ,len].pack("CN")
		#print @fd.methods.join("\n")
		@fd.write(buf)
		# read reply
		@fd.read(1) # must be RAP_READ|RAP_REPLY
		len = @fd.read(4).unpack("N")[0]
		buf = @fd.read(len[0].to_i)
		return buf
	end
	
	def close(fh)
		print "==> Close\n"
		buf = [RAP_CLOSE, fh].pack("CN")
		@fd.write(buf)
		buf = @fd.read(1)
		ret = @fd.read(4).unpack("N")[0]
		ret
	end
end
