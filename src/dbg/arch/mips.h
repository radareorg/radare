/* todo */
/* Cross-architecture macros */
// XXX: register access fixed here
#if 0
#define CPU_ARG0(x) R_EAX(x)
#define CPU_ARG1(x) R_EBX(x)
#define CPU_ARG2(x) R_ECX(x)
#define CPU_ARG3(x) R_EDX(x)
#define CPU_SP(x) R_ESP(x)
#define CPU_PC(x) R_EIP(x)
#define CPU_RET(x) R_EAX(x) /* return value */
#endif
