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

#include "rasm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

int rasm_show_list()
{
 //" jmpa [addr]  - jump to absolute address (??)\n"
	printf(
 "Architectures:\n"
 " x86, ppc, arm, java\n"
 "Opcodes:\n"
 " call [æddr]  - call to address\n"
 " jmp [addr]   - jump to relative address\n"
 " jz  [addr]   - jump if equal\n"
 " jnz          - jump if not equal\n"
 " trap         - trap into the debugger\n"
 " nop          - no operation\n"
 " push 33      - push a value or reg in stack\n"
 " pop eax      - pop into a register\n"
 " int 0x80     - system call interrupt\n"
 " ret          - return from subroutine\n"
 " ret0         - return 0 from subroutine\n"
 " hang         - hang (infinite loop\n"
 " mov eax, 33  - asign a value to a register\n");
	return 0;
}

/* assemble */
int rasm_asm(char *arch, off_t offset, char *str, unsigned char *data)
{
	int ret = -1;

	if (arch == NULL||str==NULL)
		return -1;

	if (str[0]=='\0'||str[1]=='\0' || strchr(str,'?')!=NULL)
		return rasm_show_list();

	if ((!strcmp(arch, "x86")) ||(!strcmp(arch, "intel")))
		ret = rasm_x86(offset, str, data);

	if (!strcmp(arch, "arm"))
		ret = rasm_arm(offset, str, data);

	if (!strcmp(arch, "java"))
		ret = rasm_java(offset, str, data);

	if (!strcmp(arch, "ppc"))
		ret = rasm_ppc(offset, str, data);

	return ret;
}
