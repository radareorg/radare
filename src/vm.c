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

#include "radare.h"
#include "code.h"

char *vm_regs_str = NULL;
static u64 *vm_regs;
int vm_nregs = 0;

u64 vm_get(int reg)
{
	if (reg<0 || reg >=vm_nregs)
		return 0;
	return vm_regs[reg];
}

int vm_set(int reg, u64 value)
{
	if (reg<0 || reg >=vm_nregs)
		return 0;
	vm_regs[reg] = value;
	return 0;
}

int vm_init(int init)
{
	/* if debuggah get from teh cpu */
	/* vm_dbg_arch_x86_nregs */
//	const char *arch = config_get("asm.arch");
	switch (config.arch) {
	case ARCH_X86:
		vm_nregs    = vm_arch_x86_nregs;
		vm_regs     = vm_arch_x86_regs;
		vm_regs_str = vm_arch_x86_regs_str;
		if (init)
			vm_arch_x86_init();
		break;
	}
	return 0;
}

