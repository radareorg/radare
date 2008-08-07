/* Change stack perms of own process*/

#if 1
 #define _SYS_MMAN_H
 #include <bits/mman.h>
#else
 #define PROT_READ  1
 #define PROT_WRITE 2
 #define PROT_EXEC  4
#endif

#define SYS_mprotect 125
#define STACKPERMS PROT_READ|PROT_WRITE
#define FUN_IN pusha
#define FUN_OUT popa ; ret

.global change_stack_perms
change_stack_perms:
	FUN_IN
	movl %esp, %eax
	andl $0xffff0000, %eax
	movl $0xffff, %ebx
	movl STACKPERMS, %ecx
	movl SYS_mprotect, %eax
	pushl %ebp
	int $0x80
	popl %ebp
	FUN_OUT
