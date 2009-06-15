def hex2bin(str)
  return [str].pack('H*')
end

def bin2hex(binstr)
  return binstr.unpack('C*').collect{|x| x.to_s 16}
end
