#ifndef _INCLUDE_ARM_H_
#define _INCLUDE_ARM_H_

#ifndef _INCLUDE_CPU_H_
#error Do not include arm.h directly!
#endif

#define CPUREG_PC pc
#define WS_PC() ARM_pc

enum {
	ARMBP_LE,
	ARMBP_BE,
	ARMBP_ARM_LE,
	ARMBP_ARM_BE,
	ARMBP_EABI_LE,
	ARMBP_EABI_BE,
	ARMBP_THUMB_LE,
	ARMBP_THUMB_BE,
	ARMBP_ARM_THUMB_LE,
	ARMBP_ARM_THUMB_BE,
	ARMBP_LAST
} ArmBpType;

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
