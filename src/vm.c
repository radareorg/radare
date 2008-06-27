/*
 * Copyright (C) 2008
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

const char **vm_regs_str = NULL;
static u64 *vm_regs = NULL;
int vm_nregs = 0;

/* returns register index given a name */
int vm_get_reg(const char *name)
{
	int i;
	if (vm_regs == NULL || vm_regs_str == NULL)
		return 0;
	for(i=0;i<vm_nregs;i++)
		if (!strcmp(name, vm_regs_str[i]))
			return i;
	i = atoi(name+1);
	if (i<0||i>vm_nregs)
		return i;
	return -1;
}

u64 vm_get(int reg)
{
	if (vm_regs == NULL || vm_regs_str == NULL)
		return 0;
	if (reg<0 || reg >=vm_nregs)
		return 0;
	return vm_regs[reg];
}

int vm_set(int reg, u64 value)
{
	if (vm_regs == NULL || vm_regs_str == NULL)
		return 0;
	if (reg<0 || reg >=vm_nregs)
		return 0;
	vm_regs[reg] = value;
	return 0;
}

void vm_print()
{
	int i;
	if (vm_regs == NULL || vm_regs_str == NULL)
		return;

	for(i=0;i<vm_nregs;i++) {
		cons_printf("%s = 0x%08llx\n", vm_regs_str[i], vm_regs[i]);
	}
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
		// TODO: do the same for fpregs and mmregs
		if (init)
			vm_arch_x86_init();
		break;
	case ARCH_MIPS:
		vm_nregs    = vm_arch_mips_nregs;
		vm_regs     = vm_arch_mips_regs;
		vm_regs_str = vm_arch_mips_regs_str;
		// TODO: do the same for fpregs and mmregs
		if (init)
			vm_arch_mips_init();
		break;
	default:
		vm_regs = NULL;
	}
	return 0;
}

/* emulate n opcodes */
int vm_emulate(int n)
{
	eprintf("TODO: vm_emulate\n");
	vm_init(1);
	vm_print();
	return n;
}
