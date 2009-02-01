#ifndef _INCLUDE_R_DEBUG_H_
#define _INCLUDE_R_DEBUG_H_

#include <r_types.h>
#include <r_util.h>
#include <r_reg.h>
#include <r_asm.h>
#include <r_syscall.h>

struct r_debug_t {
	int pid;    /* selected process id */
	int tid;    /* selected thread id */
	int swstep; /* steps with software traps */
	int steps;  /* counter of steps done */
	struct r_reg_t reg;  /* counter of steps done */
	int (*attach)(int pid);
	int (*detach)(int pid);
	int (*step)(int pid);
	int (*cont)(int pid);
	int (*contsc)(int pid, int sc);
	/* io */
	int (*read)(int pid, u64 addr, u8 *buf, int len);
	int (*write)(int pid, u64 addr, u8 *buf, int len);
};

int r_debug_continue(struct r_debug_t *dbg);

#if 0
Missing callbacks
=================
 - alloc
 - dealloc
 - list maps
 - change memory protections
 - touchtrace
 - filedescriptor set/get/mod..
 - get/set signals
 - get regs, set regs

#endif

#endif
