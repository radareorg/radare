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

#define CPU_ARG0(x) R_R0(x)
#define CPU_ARG1(x) R_R1(x)
#define CPU_ARG2(x) R_R2(x)
#define CPU_ARG3(x) R_R3(x)
#define CPU_SP(x) R_SP(x)
#define CPU_PC(x) R_PC(x)
#define CPU_RET(x) R_R0(x) /* return value */

#endif
