/*
 * Copyright (C) 2008
 *       pancake <youterm.com>
 *
 * radare is part of the radare project
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "libps2fd.h"
#include "arch/arch.h"
#include "../radare.h"
#include "../config.h"
#include "../print.h"
#include "../code.h"
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include "debug.h"

int debug_lib_load(const char *file)
{
#if 0
	char tmp[1024];
	regs_t oregs, regs;
	u64 pc;
//__linux__
	/* use uselib syscall here! :D */
	// XXX control strlen(fil
	if (strlen(file) > 1023) {
		fprintf(stderr, "debug_lib_load: String too long\n");
		return 1;
	}
	printf("Using 'uselib' syscall...\n");

	/* backup current state */
	debug_getregs(ps.tid, &regs);
	pc = CPU_PC(regs);
	memcpy(&oregs, &regs, sizeof(regs_t));
	debug_read_at(ps.tid, tmp, 1024, pc);

	printf("pc = 0x%08llx\n", pc);

	/* prepare memory */
	// XXX: this is intel only
	debug_setregs(ps.tid, &regs);
	debug_write_at(ps.tid, "\xcd\x80\xCC", 4, pc);
	debug_write_at(ps.tid, file, strlen(file), pc+10);

#if __i386__
	// XXX this is broken
	/* prepare registers */
	CPU_ARG0(regs) = 86; // uselib
	CPU_ARG1(regs) = pc+10; // string
	CPU_ARG2(regs) = pc; // ??? unnecessary
#endif

	/* run */
	debug_step(1);
	debug_getregs(ps.tid, &regs);
	printf("RESULT: %d\n", CPU_RET(regs));

	/* restore */
	debug_write_at(ps.tid, tmp, 1024, pc);
	debug_setregs(ps.tid, &oregs);
#elif __BSD__
	/* use dlopen from libc */
#else
	fprintf(stderr, "Sorry. !lib is not implemented for this platform\n");
	return 1;
#endif
	return 0;
}
