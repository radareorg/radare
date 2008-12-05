
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

#define R_CS(x) x.xcs
#define R_DS(x) x.xds
#define R_FS(x) x.xfs
#define R_SS(x) x.xss
#define R_GS(x) x.xgs
#define R_ES(x) x.xes

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
