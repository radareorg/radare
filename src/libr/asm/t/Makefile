all: disasm_x86 disasm_arm asm_x86

disasm_x86:
	${CC} -g -I ../../include disasm_x86.c ../libr_asm.a -o disasm_x86

disasm_arm:
	${CC} -g -I ../../include disasm_arm.c ../libr_asm.a -o disasm_arm

asm_x86:
	${CC} -g -I ../../include asm_x86.c ../libr_asm.a -o asm_x86

clean:
	rm -f disasm_x86 disasm_arm asm_x86
