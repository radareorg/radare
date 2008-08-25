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
