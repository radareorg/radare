/*
 * Copyright (C) 2007, 2008, 2009
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
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#if __UNIX__
#include <sys/mman.h>
#endif
#include "../main.h"
#include "../utils.h"
#include "../list.h"
#include "mem.h"
#include "string.h"
#include "debug.h"

#define debug_read_raw(x,y) ptrace(PTRACE_PEEKTEXT, x, y, 0)

#define ALIGN_SIZE 4096

int debug_read_at(pid_t pid, void *data, int length, ut64 addr)
{
	if (regio_enabled) {
		int ret = debug_reg_read_at(pid, data, length, addr);
		if (ret != -1) return ret;
	}
	if (fdio_enabled)
		return debug_fd_read_at(pid, data, length, addr);
	return debug_os_read_at(ps.tid, data, length, addr);
}

int debug_read(pid_t pid, void *data, int length)
{
	if (length<0)
		return -1;
	return debug_read_at(pid, data, length, ps.offset);
}

int debug_write_at(pid_t pid, void *data, int length, ut64 addr)
{
	if (regio_enabled) {
		int ret = debug_reg_write_at(pid, data, length, addr);
		if (ret != -1) return ret;
	}
	if (fdio_enabled)
		return debug_fd_write_at(pid, data, length, addr);
	return debug_os_write_at(ps.tid, data, length, addr);
}

int debug_write(pid_t pid, void *data, int length)
{
	return debug_write_at(ps.tid, data, length, ps.offset);
}

int debug_loaduri(char *cmd)
{
	ps.filename = cmd;
	return debug_load();
}

int debug_unload()
{
	ps.opened = 0;
#if __UNIX__
	debug_os_kill(ps.tid, SIGKILL);
#endif
	ps.pid = ps.tid = 0;

	return 0; //for warning message
}

int debug_load()
{
	int ret = 0;
	char pids[128];

	if (ps.pid!=0) {
		/* TODO: check if pid is still running */
		// use signal(0) to check if its already there
		/* TODO: ask before kill */
	//	if (ps.is_file)
	//		debug_os_kill(ps.tid, SIGKILL);
	//	else return 0;
	}

	WS(event)   = UNKNOWN_EVENT;
	ps.bin_usrcode = NULL;

	ps_parse_argv();

	if (ps.argv[0]== NULL) {
		eprintf("No argv[0] available\n");
		return 1;
	}
	ps.is_file = !atoi(ps.filename);
	if (ps.is_file) 
		ret = debug_fork_and_attach();
	else
		ret = debug_attach(atoi(ps.filename));

	ps.entrypoint = arch_get_entrypoint();
	ps.th_active = 0;

	sprintf(pids, "%d", ps.tid);
	setenv("DPID", pids, 1);
	debug_init_maps(0);
	events_init(ps.pid);

	debug_getregs(ps.pid, &(WS(regs)));

	return ret;
}
