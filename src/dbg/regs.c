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

extern struct regs_off roff[];
int regio_enabled = 0;
ut64 regio_addr = 0;

/* reg io mode */
int debug_reg_read_at(int pid, u8 *data, int length, ut64 addr)
{
	regs_t regs;
	int i, regsz = sizeof(regs_t);
	u8 *ptr = (u8*) &regs;
	debug_getregs(pid, &regs);
	if (addr<regio_addr)
		return -1;
	if (addr>regsz+regio_addr)
		return -1;
	for(i=0;i<length;i++) {
		if (addr+i>regsz+regio_addr)
			data[i] = 0xff;
		else data[i] = ptr[i];
	}
	return length;
}

int debug_reg_write_at(int pid, const u8 *data, int length, ut64 addr)
{
	regs_t regs;
	int i, regsz = sizeof(regs_t);
	u8 *ptr = (u8*)&regs;
	debug_getregs(pid, &regs);
	if (addr>regsz+regio_addr)
		return -1;
	for(i=0;i<length;i++) {
		if (addr+i<regsz+regio_addr)
			ptr[addr-regio_addr+i] = data[i];
	}
	debug_setregs(pid, &regs);
	return length;
}
/* ----------- */

ut64 debug_get_regoff(regs_t *regs, int off)
{
	char *c = (char *)regs;
	debug_getregs(ps.tid, regs);
	return *(unsigned long *)(c + off);	
}

void debug_set_regoff(regs_t *regs, int off, unsigned long val)
{
	char *c = (char *)regs;
	*(unsigned long *)(c + off) = val;	

#if __WINDOWS__
	regs->ContextFlags = CONTEXT_FULL;
#endif
	debug_setregs(ps.tid, regs);
}

ut64 debug_get_register(const char *input)
{
	const char *reg = input;
	int off;
	ut64 ret;

	// TODO: user streclean
	if(*input == ' ')
		reg = input + 1;

	if((off = get_reg(reg)) == -1)
		return -1;

	/* TODO: debug_get_regoff must return a value addr_t */
	ret = (ut64)debug_get_regoff(&WS(regs), roff[off].off); 

	printf("0x%08llx\n", ret);

	return ret;
}

int debug_set_register(const char *args)
{
	char *value;
	char *arg;

	if (args == NULL) {
		eprintf("Usage: !set [reg] [value]\n");
		return 1;
	}

	arg = alloca(strlen(args)+1);
	strcpy(arg, args);

	if (!ps.opened) {
		eprintf(":regs No program loaded.\n");
		return 1;
	}
	value = strchr(arg, '=');
	if (!value)
		value = strchr(arg, ' ');
	if (!value) {
		eprintf("Usage: !set [reg] [value]\n");
		eprintf("  > !set eflags PZTI\n");
		eprintf("  > !set r0 0x33\n");
		return 1;
	}
	value[0]='\0';
	arg = strclean(arg);
	value = strclean(value+1);
	printf("%s=%s\n", arg, value);

	return arch_set_register(arg, value);
}

int debug_fpregisters(int rad)
{
	if (!ps.opened) {
		eprintf(":fpregs No program loaded.\n");
		return 1;
	}

	if (rad)
		cons_printf("fs fpregs\n");
	return arch_print_fpregisters(rad, "");
}

int debug_dregisters(int rad)
{
	if (!ps.opened) {
		eprintf(":regs No program loaded.\n");
		return 1;
	}
	return arch_print_registers(rad, "line");
}

int debug_oregisters(int rad)
{
	if (!ps.opened) {
		eprintf(":regs No program loaded.\n");
		return 1;
	}
	return arch_print_registers(rad, "orig");
}

int debug_registers(int rad)
{
	int fs, ret = 1;

	if (!ps.opened) {
		eprintf(":regs No program loaded.\n");
	} else {
		fs = flag_space_idx;
		flag_space_set("regs");
		ret = arch_print_registers(rad, "");
		flag_space_idx = fs;
	}
	return ret;
}
