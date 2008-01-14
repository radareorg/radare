#ifndef _INCLUDE_CPU_H_
#error Do not include arm.h directly!
#endif

#define REG_PC pc
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
};
