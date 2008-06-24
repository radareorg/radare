/* Alternative system calls */
/* pancake <youterm.com> */

.global main
.data
msg:

.string "Hello World\n"
.equ len, .-msg

.text
main:
	movl $0x10, %esi
	movl %gs:(%esi), %edi
	movl $4, %eax
	movl $1, %ebx
	leal msg, %ecx
	movl $len, %edx

	call *%edi
	ret
