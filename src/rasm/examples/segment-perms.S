.intel_syntax
.text
.global main
main:
	push offset header
	call puts
	pop %ecx
	xor %ecx,%ecx
loop:
	xor %eax,%eax
	push %ecx
	verr %cx
	sete %al
	push %eax
	xor %al,%al
	verw %cx
	sete %al
	push %eax
	xor %al,%al
	push %ecx
	push offset format
	add %eax,%ss:[%esp+0xC]
	test %eax,%eax
	je short skip
	call printf
skip:
	add %esp,0x10
	pop %ecx
	inc %cx
	jnz short loop
exit:
	ret
header:
.string "segm w r"
format:
.string "%4x:%1i:%1i\n"
