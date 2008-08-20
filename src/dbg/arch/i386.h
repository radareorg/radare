#ifndef _INCLUDE_I386_H_
#define _INCLUDE_I386_H_

#ifndef _INCLUDE_CPU_H_
#error Do not include i386.h directly!
#endif

#include <limits.h>

/* ensure */
#undef __UNIX__
#undef __WINDOWS__
#if __WIN32__ || __CYGWIN__ || MINGW32
#define __WINDOWS__ 1
#else
#define __UNIX__ 1
#endif

#undef offsetof
#define offsetof(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)

void dr_list();
void dr_init();

#if __linux__

#if __i386__
#include <sys/user.h>
#define regs_t struct user_regs_struct
#define R_EIP(x) x.eip
#define R_EFLAGS(x) x.eflags
#define R_EBP(x) x.ebp
#define R_ESP(x) x.esp
#define R_EAX(x) x.eax
#define R_OEAX(x) x.orig_eax
#define R_EBX(x) x.ebx
#define R_ECX(x) x.ecx
#define R_EDX(x) x.edx
#define R_ESI(x) x.esi
#define R_EDI(x) x.edi
/* registers offset */

#define R_EIP_OFF offsetof(struct user_regs_struct, eip)
#define R_EFLAGS_OFF offsetof(struct user_regs_struct, eflags)
#define R_EBP_OFF offsetof(struct user_regs_struct, ebp)
#define R_ESP_OFF offsetof(struct user_regs_struct, esp)
#define R_EAX_OFF offsetof(struct user_regs_struct, eax)
#define R_OEAX_OFF offsetof(struct user_regs_struct, orig_eax)
#define R_EBX_OFF offsetof(struct user_regs_struct, ebx)
#define R_ECX_OFF offsetof(struct user_regs_struct, ecx)
#define R_EDX_OFF offsetof(struct user_regs_struct, edx)
#define R_ESI_OFF offsetof(struct user_regs_struct, esi)
#define R_EDI_OFF offsetof(struct user_regs_struct, edi)
#endif

#if __x86_64__
#include <sys/user.h>
#define regs_t struct user_regs_struct
#define R_RIP(x) x.rip
#define R_RFLAGS(x) x.eflags
#define R_RBP(x) x.rbp
#define R_RSP(x) x.rsp
#define R_RAX(x) x.rax
#define R_ORAX(x) x.orig_rax
#define R_RBX(x) x.rbx
#define R_RCX(x) x.rcx
#define R_RDX(x) x.rdx
#define R_RSI(x) x.rsi
#define R_RDI(x) x.rdi

#define R_R8(x) x.r8
#define R_R9(x) x.r9
#define R_R10(x) x.r10
#define R_R11(x) x.r11
#define R_R12(x) x.r12
#define R_R13(x) x.r13
#define R_R14(x) x.r14
#define R_R15(x) x.r15
/* registers offset */

#define R_RIP_OFF offsetof(struct user_regs_struct, rip)
#define R_RFLAGS_OFF offsetof(struct user_regs_struct, eflags)
#define R_RBP_OFF offsetof(struct user_regs_struct, rbp)
#define R_RSP_OFF offsetof(struct user_regs_struct, rsp)
#define R_RAX_OFF offsetof(struct user_regs_struct, rax)
#define R_ORAX_OFF offsetof(struct user_regs_struct, orig_rax)
#define R_RBX_OFF offsetof(struct user_regs_struct, rbx)
#define R_RCX_OFF offsetof(struct user_regs_struct, rcx)
#define R_RDX_OFF offsetof(struct user_regs_struct, rdx)
#define R_RSI_OFF offsetof(struct user_regs_struct, rsi)
#define R_RDI_OFF offsetof(struct user_regs_struct, rdi)

#define R_R8_OFF offsetof(struct reg, r8)
#define R_R9_OFF offsetof(struct reg, r9)
#define R_R10_OFF offsetof(struct reg, r10)
#define R_R11_OFF offsetof(struct reg, r11)
#define R_R12_OFF offsetof(struct reg, r12)
#define R_R13_OFF offsetof(struct reg, r13)
#define R_R14_OFF offsetof(struct reg, r14)
#define R_R15_OFF offsetof(struct reg, r15)

#endif

#elif __WINDOWS__
#include <windows.h>

#define R_EIP(x) x.Eip
#define R_EFLAGS(x) x.EFlags
#define R_EBP(x) x.Ebp
#define R_ESP(x) x.Esp
#define R_EAX(x) x.Eax
#define R_OEAX(x) x.Eax /* not supported */
#define R_EBX(x) x.Ebx
#define R_ECX(x) x.Ecx
#define R_EDX(x) x.Edx
#define R_ESI(x) x.Esi
#define R_EDI(x) x.Edi

#define ROFF_DR0	7

#if __i386__
#define R_DR0(x) x.Dr0
#define R_DR1(x) x.Dr1
#define R_DR2(x) x.Dr2
#define R_DR3(x) x.Dr3
#define R_DR6(x) x.Dr6
#define R_DR7(x) x.Dr7
#endif
/* registers offset */

#define R_EIP_OFF offsetof(CONTEXT, Eip)
#define R_EFLAGS_OFF offsetof(CONTEXT, EFlags)
#define R_EBP_OFF offsetof(CONTEXT, Ebp)
#define R_ESP_OFF offsetof(CONTEXT, Esp)
#define R_EAX_OFF offsetof(CONTEXT, Eax)
#define R_OEAX_OFF offsetof(CONTEXT, Eax)
#define R_EBX_OFF offsetof(CONTEXT, Ebx)
#define R_ECX_OFF offsetof(CONTEXT, Ecx)
#define R_EDX_OFF offsetof(CONTEXT, Edx)
#define R_ESI_OFF offsetof(CONTEXT, Esi)
#define R_EDI_OFF offsetof(CONTEXT, Edi)
#define R_DR0_OFF offsetof(CONTEXT, Dr0)
#define R_DR1_OFF offsetof(CONTEXT, Dr1)
#define R_DR2_OFF offsetof(CONTEXT, Dr2)
#define R_DR3_OFF offsetof(CONTEXT, Dr3)
#define R_DR6_OFF offsetof(CONTEXT, Dr6)
#define R_DR7_OFF offsetof(CONTEXT, Dr7)

#elif __APPLE__
#include <mach/i386/_structs.h>
#define regs_t _STRUCT_X86_THREAD_STATE32
#define R_EIP(x) x.__eip
#define R_EFLAGS(x) x.__eflags
#define R_EBP(x) x.__ebp
#define R_ESP(x) x.__esp
#define R_EAX(x) x.__eax
#define R_OEAX(x) x.__eax /* not supported */
#define R_EBX(x) x.__ebx
#define R_ECX(x) x.__ecx
#define R_EDX(x) x.__edx
#define R_ESI(x) x.__esi
#define R_EDI(x) x.__edi
/* registers offset */

#define R_EIP_OFF offsetof(regs_t, __eip)
#define R_EFLAGS_OFF offsetof(regs_t, __eflags)
#define R_EBP_OFF offsetof(regs_t, __ebp)
#define R_ESP_OFF offsetof(regs_t, __esp)
#define R_EAX_OFF offsetof(regs_t, __eax)
#define R_OEAX_OFF offsetof(regs_t, __eax)
#define R_EBX_OFF offsetof(regs_t, __ebx)
#define R_ECX_OFF offsetof(regs_t, __ecx)
#define R_EDX_OFF offsetof(regs_t, __edx)
#define R_ESI_OFF offsetof(regs_t, __esi)
#define R_EDI_OFF offsetof(regs_t, __edi)

#else /* BSD? */

#include <machine/reg.h>
#include <sys/param.h>
#include <sys/user.h>

#if defined(__amd64)
#undef __x86_64
#define __x86_64 1
#endif

#if __x86_64__
#define regs_t struct reg
#define R_RIP(x) x.r_rip
#define R_RFLAGS(x) x.r_rflags
#define R_RBP(x) x.r_rbp
#define R_RSP(x) x.r_rsp
#define R_RAX(x) x.r_rax
#define R_ORAX(x) x.r_rax
#define R_RBX(x) x.r_rbx
#define R_RCX(x) x.r_rcx
#define R_RDX(x) x.r_rdx
#define R_RSI(x) x.r_rsi
#define R_RDI(x) x.r_rdi

#define R_R8(x) x.r_r8
#define R_R9(x) x.r_r9
#define R_R10(x) x.r_r10
#define R_R11(x) x.r_r11
#define R_R12(x) x.r_r12
#define R_R13(x) x.r_r13
#define R_R14(x) x.r_r14
#define R_R15(x) x.r_r15

/* registers offset */

#define R_RIP_OFF offsetof(struct reg, r_rip)
#define R_RFLAGS_OFF offsetof(struct reg, r_rflags)
#define R_RBP_OFF offsetof(struct reg, r_rbp)
#define R_RSP_OFF offsetof(struct reg, r_rsp)
#define R_RAX_OFF offsetof(struct reg, r_rax)
#define R_ORAX_OFF offsetof(struct reg, r_rax)
#define R_RBX_OFF offsetof(struct reg, r_rbx)
#define R_RCX_OFF offsetof(struct reg, r_rcx)
#define R_RDX_OFF offsetof(struct reg, r_rdx)
#define R_RSI_OFF offsetof(struct reg, r_rsi)
#define R_RDI_OFF offsetof(struct reg, r_rdi)

#define R_R8_OFF offsetof(struct reg, r_r8)
#define R_R9_OFF offsetof(struct reg, r_r9)
#define R_R10_OFF offsetof(struct reg, r_r10)
#define R_R11_OFF offsetof(struct reg, r_r11)
#define R_R12_OFF offsetof(struct reg, r_r12)
#define R_R13_OFF offsetof(struct reg, r_r13)
#define R_R14_OFF offsetof(struct reg, r_r14)
#define R_R15_OFF offsetof(struct reg, r_r15)

#else

#define regs_t struct reg
#define R_EIP(x) x.r_eip
#define R_EFLAGS(x) x.r_eflags
#define R_EBP(x) x.r_ebp
#define R_ESP(x) x.r_esp
#define R_EAX(x) x.r_eax
#define R_OEAX(x) x.r_eax /* not supported */
#define R_EBX(x) x.r_ebx
#define R_ECX(x) x.r_ecx
#define R_EDX(x) x.r_edx
#define R_ESI(x) x.r_esi
#define R_EDI(x) x.r_edi
/* registers offset */

#define R_EIP_OFF offsetof(struct reg, r_eip)
#define R_EFLAGS_OFF offsetof(struct reg, r_eflags)
#define R_EBP_OFF offsetof(struct reg, r_ebp)
#define R_ESP_OFF offsetof(struct reg, r_esp)
#define R_EAX_OFF offsetof(struct reg, r_eax)
#define R_OEAX_OFF offsetof(struct reg, r_eax)
#define R_EBX_OFF offsetof(struct reg, r_ebx)
#define R_ECX_OFF offsetof(struct reg, r_ecx)
#define R_EDX_OFF offsetof(struct reg, r_edx)
#define R_ESI_OFF offsetof(struct reg, r_esi)
#define R_EDI_OFF offsetof(struct reg, r_edi)

#endif

#endif
#include "i386-debug.h"

#define WS_PC() R_EIP(WS(regs))
#define REG_VAL	(unsigned long)

#ifdef __linux__
#define SYSCALL_OPS	"\xcd\x80\xcc\x90"	/* int $0x80, int $0x3, nop */
#define SYSCALL_INS	"\xcd\x80"
#else
#define SYSCALL_OPS	"\xcd\x80\xcc\x90"	/* int $0x80, int $0x3, nop */
#define SYSCALL_INS	""
#endif

extern int instLength(unsigned char *p, int s, int mode);
extern int dislen(unsigned char* opcode0, int limit);

#define REG_PC eip
#define REG_SP esp

#endif
