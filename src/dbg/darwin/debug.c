/*
 * Copyright (C) 2008
 *       th0rpe <nopcode.org>
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

#define __addr_t_defined

#include "../mem.h"
#include "../debug.h"
#include "../thread.h"
//#include "utils.h"



int debug_ktrace()
{
}

inline int debug_detach()
{
	/* TODO */
	return 0;
}

int debug_attach(int pid)
{
}

int debug_load_threads()
{
	return 0;
}

int debug_contp(int tid)
{

	return 0;
}

inline int debug_os_steps()
{

	return 0;
}

/* TODO: not work */
int debug_contfork(int tid) { eprintf("not work yet\n"); return -1; }
int debug_contscp() { eprintf("not work yet\n"); return -1; }
int debug_status() { eprintf("not work yet\n"); return -1; }

int debug_pstree(char *input)
{ 
	int tid = atoi(input);
	if (tid != 0) {
		ps.tid = tid;
		eprintf("Current selected thread id (pid): %d\n", ps.tid);
		// XXX check if exists or so
	}

	printf(" pid : %d\n", ps.pid);
	printf(" tid : %d\n", ps.tid);

	eprintf("more work to do\n"); return -1; 
}

int debug_fork_and_attach()
{
	/* TODO */
}

int debug_read_at(pid_t tid, void *buff, int len, u64 addr)
{
	int ret_len = 0;

	/* TODO */

	return ret_len;
}

int debug_write_at(pid_t tid, void *buff, int len, u64 addr)
{
	int ret_len = -1;

	/* TODO */

	return ret_len;
}


int debug_getregs(pid_t tid, regs_t *regs)
{
	/* TODO */
	return 0;
}

int debug_setregs(pid_t tid, regs_t *regs)
{
	/* TODO */
	return 0;
}

inline int debug_single_setregs(pid_t tid, regs_t *regs)
{
	return 0;
}

static int debug_enable(int enable)
{

	return 0;
}

int debug_print_wait(char *act)
{
	return 0;
}

static void debug_init_console()
{
}

int debug_os_init()
{
	return debug_enable(1);
}

static int debug_exception_event(unsigned long code)
{

	return 0;
}

int debug_dispatch_wait() 
{

	return 0;
}

static inline int CheckValidPE(unsigned char * PeHeader)
{    	    

	return 0;
}


int debug_init_maps(int rest)
{

	return 0;
}

