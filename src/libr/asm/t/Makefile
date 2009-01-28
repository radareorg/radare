all: disasm_x86 disasm_arm disasm_mips asm_x86 rasm2 realloc_x86

disasm_x86:
	${CC} -g -I ../../include disasm_x86.c ../libr_asm.a -o disasm_x86

disasm_arm:
	${CC} -g -I ../../include disasm_arm.c ../libr_asm.a -o disasm_arm

disasm_mips:
	${CC} -g -I ../../include disasm_mips.c ../libr_asm.a -o disasm_mips

asm_x86:
	${CC} -g -I ../../include asm_x86.c ../libr_asm.a -o asm_x86

rasm2:
	${CC} -g -I ../../include rasm2.c ../libr_asm.a ../../util/*.a -o rasm2

realloc_x86:
	${CC} -g -I ../../include realloc_x86.c ../libr_asm.a ../../bin/*.a ../../util/*.a -o realloc_x86

clean:
	rm -f disasm_x86 disasm_arm disasm_mips asm_x86 rasm2 realloc_x86
