/* todo */
/* Cross-architecture macros */
#ifndef _INCLUDE_MIPS_H_
#define _INCLUDE_MIPS_H_

/* a0, a1, a2, a3 */
#define CPU_ARG0(x) (((unsigned long long*)&x)[4]) /* a0 */
#define CPU_ARG1(x) (((unsigned long long*)&x)[5])
#define CPU_ARG2(x) (((unsigned long long*)&x)[6])
#define CPU_ARG3(x) (((unsigned long long*)&x)[7])
#define CPU_SP(x) (((unsigned long long*)&x)[29])  /* sp */
#define CPU_PC(x) (((unsigned long long*)&x)[0])   /* pc */
#define CPU_RET(x) (((unsigned long long*)&x)[2])  /* v0 */

#endif
