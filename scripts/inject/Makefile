all:
	as shellcode.s
	ld shellcode.o -o shellcode
	radare -i dump-text.lua shellcode #> shellcode.text
