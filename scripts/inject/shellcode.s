.intel_syntax noprefix
.global _start

_start:
	pusha
	call foo
	.string "/bin/sh"
foo:
	mov ebx, [esp]
	push edx
	push ebx
	mov ecx, esp
	mov eax, 0xb
	int 0x80
	popa
