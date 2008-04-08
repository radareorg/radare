#ifndef _INCLUDE_CPU_H_
#define _INCLUDE_CPU_H_

struct {
	char *name;
} cpu_struct;

#if ARCH_I386
#include "arch/i386.h"
#endif
#if ARCH_ARM
#include "arch/arm.h"
#endif
#if ARCH_MIPS
#include "arch/mips.h"
#endif

#endif
