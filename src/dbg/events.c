/*
 * Copyright (C) 2007
 *       pancake <youterm.com>
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
/*
   This code implements a portable layer for handling events on different
   architectures.

   Some debugger APIs supports event messages to inform the debugger about
   different actions of the target process. This is for example a fork,
   clone, execve, cleaner SIGTRAP implementation etc..
 */

#if __linux__

#include "libps2fd.h"
#include <linux/ptrace.h> // PTRACE_O_*
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include "mem.h"
#include "arch/i386.h"

int events_init()
{
	int flags;

#if !__x86_64__
	/* 2.4 */
	flags    = PTRACE_O_TRACESYSGOOD;
	/* 2.6 */
	flags   |= PTRACE_O_TRACEFORK
		|  PTRACE_O_TRACEVFORK
		|  PTRACE_O_TRACECLONE
		|  PTRACE_O_TRACEEXEC
		|  PTRACE_O_TRACEVFORKDONE
		|  PTRACE_O_TRACEEXIT;

	return ptrace(PTRACE_SETOPTIONS,ps.pid, NULL, flags);
#endif
	return 0;
}

int events_get()
{
	int ret;
	unsigned long ret2;

#if !__x86_64__
	ret = ptrace(PTRACE_GETEVENTMSG, ps.pid, NULL, &ret2);

	switch(ret) {
	case PTRACE_EVENT_EXIT:
		printf("Exit status: %d\n", ret);
		break;
	case PTRACE_EVENT_EXEC:
		printf("Execve handled %d %d\n", ret, ret2);
		break;
	case PTRACE_EVENT_FORK:
	case PTRACE_EVENT_VFORK:
	case PTRACE_EVENT_CLONE:
		printf("New pid: %d\n", ret);
		break;
	case PTRACE_EVENT_VFORK_DONE:
		printf("Vfork done: %d\n", ret);
		break;
	}
#endif
	return 0;
}

#else

int events_init()
{
	/* not supported */
}

int events_get()
{
	/* not supported */
}

#endif