# move to rapy api
import r

class Program():
	def update(self):
		self.arch = r.cmd("e asm.arch")
		self.bits = r.cmd("e asm.bits")
		self.os = r.cmd("e asm.os")
		self.type = r.cmd("e file.type")
		self.size = r.cmd("i~size[1]#1")

	def __init__(self):
		self.update()

class Function():
	def __init__(self):
		self.name = 0
		self.addr = 0
		self.size = 0

class Functions():
	def update(self):
		self.list = []
		items = r.cmd("CF").split('\n')
		for a in items:
			words = a.split(' ')
			fun = Function()
			fun.size = words[1]
			fun.addr = words[3]
			fun.name = words[5]
			self.list.append(fun)

	def __init__(self):
		self.update()

def get_root():
	return Program()
