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

#include <string.h>
#include "radare.h"
#include "code.h"

/* from mips-dis.c */
extern const char * mips_gpr_names_newabi[32];
extern const char * mips_gpr_names_oldabi[32];
//const char **vm_arch_mips_regs_str = &mips_gpr_names_newabi;
const char **vm_arch_mips_regs_str = (const char **)&mips_gpr_names_oldabi;

int vm_arch_mips_nregs = VM_MIPS_N_REGS;
u64 vm_arch_mips_regs[VM_MIPS_N_REGS];

void vm_arch_mips_init()
{
	memset(&vm_arch_mips_regs, '\0', sizeof(vm_arch_mips_regs));
}
