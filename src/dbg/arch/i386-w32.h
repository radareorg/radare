
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
