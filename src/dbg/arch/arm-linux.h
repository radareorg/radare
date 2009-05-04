#ifndef _INCLUDE_ARM_H_
#define _INCLUDE_ARM_H_

//#define regs_t elf_gregset_t
/* linux */
#define R_R0(x) x[0]
#define R_R1(x) x[1]
#define R_R2(x) x[2]
#define R_R3(x) x[3]
#define R_SP(x) x[14]
#define R_PC(x) x[15]

//#define SYSCALL_OPS "\xcd\x80\xcc\x90"
#define SYSCALL_OPS "\xef\x90\x00\x00"
#define SYSCALL_OPS_little "\x00\x00\x90\xef"

#define CPU_ARG0(x) R_R0(x)
#define CPU_ARG1(x) R_R1(x)
#define CPU_ARG2(x) R_R2(x)
#define CPU_ARG3(x) R_R3(x)
#define CPU_SP(x) R_SP(x)
#define CPU_PC(x) R_PC(x)
#define CPU_RET(x) R_R0(x) /* return value */

#endif
