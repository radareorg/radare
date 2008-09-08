#ifndef _INCLUDE_POWERPC_H_
#define _INCLUDE_POWERPC_H_

#if __APPLE__
#include "powerpc-darwin.h"
#endif

#define CPU_ARG0(x) R_R0(x)
#define CPU_ARG1(x) R_R1(x)
#define CPU_ARG2(x) R_R2(x)
#define CPU_ARG3(x) R_R3(x)
#define CPU_SP(x) R_SRR1(x)
#define CPU_PC(x) R_SRR0(x)
#define CPU_RET(x) R_R0(x) /* return value */

#endif
