/*
 * Copyright (C) 2007, 2008
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

#include "libps2fd.h"

struct event_t events[] = {
	{ "stop", 19, 0 },
	{ "trap", 5,  0 },
	{ "pipe", 13,  0 },
	{ "alarm", 14, 0 },
	{ "fpe", 8, 0},
	{ "ill", 4, 0},
	{ NULL, 0, 0 }
};

void event_ignore_list()
{
	int i;
	for(i=0;events[i].name;i++)
		cons_printf(" %d %s\n", events[i].ignored, events[i].name);
}

int event_set_ignored(char *name, int ignored)
{
	int i;
	for(i=0;events[i].name;i++) {
		if (!strcmp(events[i].name, name)) {
			events[i].ignored = ignored;
			return 1;
		}
	}
	eprintf("No event named '%s'.\n", name);
	return 0;
}

int event_is_ignored(int id)
{
	int i;
//printf("chk(%d)\n", id);
	for(i=0;events[i].name;i++) {
		if (events[i].id == id) {
			if (events[i].ignored)
				return 1;
			else	return 0;
		}
	}
	return 0;
}

#if __linux__

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


int events_init(int pid)
{
	int flags;
#if __linux__
	/* 2.4 */
	flags    = PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXIT | PTRACE_O_TRACEEXEC;
	/* 2.6 */
if (config_get_i("dbg.forks"))
	flags   |= PTRACE_O_TRACEFORK
		|  PTRACE_O_TRACEVFORK
		|  PTRACE_O_TRACEVFORKDONE;
if (config_get_i("dbg.threads"))
	flags   |= PTRACE_O_TRACECLONE;

	return ptrace(PTRACE_SETOPTIONS, pid, NULL, flags);
#endif
	return 0;
}

int events_init_all()
{
	struct list_head *pos;
	events_init(ps.pid);
	events_init(ps.tid);
	list_for_each_prev(pos, &ps.th_list) {
		TH_INFO		*th = list_entry(pos, TH_INFO, list);
		events_init(th->tid);
	}
	return 0;
}

int events_get()
{
	int ret;
	unsigned int ret2;

#if __linux__
	ret = ptrace(PTRACE_GETEVENTMSG, ps.tid, NULL, &ret2);

	switch(ret) {
	case PTRACE_EVENT_EXIT:
		cons_printf("tid = %d\n", ps.tid);
		cons_printf("event-msg = exit status: %d\n", ret);
		config.interrupted = 1;
		break;
	case PTRACE_EVENT_EXEC:
		cons_printf("tid = %d\n", ps.tid);
		cons_printf("event-msg = execve handled %d %d\n", ret, ret2);
		break;
	case PTRACE_EVENT_FORK:
		cons_printf("tid = %d\n", ps.tid);
		cons_printf("type = fork\n");
		cons_printf("event-msg = new pid: %d\n", ret);
		break;
	case PTRACE_EVENT_VFORK:
		cons_printf("tid = %d\n", ps.tid);
		cons_printf("type = vfork\n");
		cons_printf("event-msg = new pid: %d\n", ret);
		break;
	case PTRACE_EVENT_CLONE:
		cons_printf("tid = %d\n", ps.tid);
		cons_printf("type = clone\n");
		cons_printf("event-msg = new pid: %d\n", ret);
		break;
	case PTRACE_EVENT_VFORK_DONE:
		cons_printf("tid = %d\n", ps.tid);
		cons_printf("event-msg = vfork done: %d\n", ret);
		break;
	}
#endif
	return 0;
}

#else

int events_init()
{
	return 0;
	/* not supported */
}

int events_get()
{
	return 0;
	/* not supported */
}

#endif
