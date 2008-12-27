require 'test/unit'
require 'src/plug/hack/radare.rb'

class TestRadare < Test::Unit::TestCase
	$r = Radare::new

#	def test_slurp
#		assert_equal("\032", $r.slurp("/etc/issue"))
#	end

	def test_hex2bin
		assert_equal("111101001101001", $r.hex2bin("7A69"))
	end
	
	def test_bin2hex
		assert_equal("7A69", $r.bin2hex("111101001101001"))
	end
=begin
	def test_str2hash
		assert_equal({"radare"="rules", "pancake"="too"}, $r.str2hash("radare=rules\npancake=too"))
	end
=end
end
