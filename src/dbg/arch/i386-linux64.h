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

#if 0
#define R_CS(x) x.xcs
#define R_DS(x) x.xds
#define R_FS(x) x.xfs
#define R_SS(x) x.xss
#define R_GS(x) x.xgs
#define R_ES(x) x.xes
#endif

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

#define R_R8_OFF offsetof(struct user_regs_struct, r8)
#define R_R9_OFF offsetof(struct user_regs_struct, r9)
#define R_R10_OFF offsetof(struct user_regs_struct, r10)
#define R_R11_OFF offsetof(struct user_regs_struct, r11)
#define R_R12_OFF offsetof(struct user_regs_struct, r12)
#define R_R13_OFF offsetof(struct user_regs_struct, r13)
#define R_R14_OFF offsetof(struct user_regs_struct, r14)
#define R_R15_OFF offsetof(struct user_regs_struct, r15)
#define R_R15_OFF offsetof(struct user_regs_struct, r15)
