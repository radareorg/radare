#include <mach/i386/_structs.h>
#include <mach/i386/thread_status.h>

#define regs_t _STRUCT_X86_THREAD_STATE32
#define regs_sizeof sizeof(regs_t)/4
//#define regs_t i386_thread_state_t

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
