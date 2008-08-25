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
