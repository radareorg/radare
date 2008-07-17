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
	 " call [addr]  - call to address\n"
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
	 " mov eax, 33  - asign a value to a register\n"
	 "Directives:\n"
	 " .zero 23     - fill 23 zeroes\n"
	 " .org 0x8000  - offset\n");
	return 0;
}

int rasm_file(const char *arch, u64 offset, const char *file, const char *ofile)
{
	char buf[1024];
	unsigned char data[1024];
	char *dofile;
	int line = 1;
	int n;
	FILE * fd = fopen(file, "rb");
	FILE * fo;
	
	if (ofile == NULL) {
		dofile = (char *)malloc(strlen(file)+3);
		strcpy(dofile, file);
		strcat(dofile, ".o");
		fo = fopen(dofile, "wb");
	} else
		fo = fopen(ofile, "wb");

	if (fd && fo) {
		// TODO : truncate+lseek+filluntil offset with 0 padding
		while(!feof(fd)) {
			buf[0]='\0';
			fgets(buf, 1024, fd);
			if (!buf[0])
				break;
			buf[strlen(buf)-1]='\0';
			eprintf(" %03d : %08llx  %s\n", line++, offset, buf);
			n = rasm_asm(arch, &offset, buf, data);
			if (n == -1)
				break;
			if (n!=0)
				fwrite(data, n, 1, fo);
			offset+=n;
		}
		fclose(fd);
		fclose(fo);
		return 0;
	}
		
	if (fd == NULL)
		eprintf("Cannot open input file.\n", ofile);
	else	fclose(fd);
	if (fo == NULL)
		eprintf("Cannot open output file.\n", ofile);
	else	fclose(fo);

	return 1;
}

int rasm_directive(const char *arch, u64 *offset, const char **str, u8 *data)
{
	char op[128];
	char *arg;
	u64 n;

	strncpy(op, str, 120);

	/* comments */
 	arg = strchr(op, ';');
	if (arg)
		arg[0] = '\0';

 	arg = strchr(op, ' ');
	if (arg) {
		arg[0] = '\0';
		arg = arg + 1;
	}

	if (!strcmp(op, ".equ")) {
		n = get_offset(arg);
printf("TODO: (%s)\n", op);
		memset(data, '\0', n);
		return n;
	}
	if (!strcmp(op, ".arch")) {
		//*arch = strdup(op+6);
printf("DEFINE CPU: (%s)\n", op+6);
		return 0;
	}

	if (!strcmp(op, ".global")) {
printf("TODO: (%s)\n", op);
		return 0; //*offset;
	}

	if (!strcmp(op, ".org")) {
		*offset = get_offset(arg);
		return 0; //*offset;
	}

	if (!strcmp(op, ".zero")) {
		n = get_offset(arg);
		memset(data, '\0', n);
		return (int)n;
	}

	if (op[strlen(op)-1]==':') {
		printf("0x%llx %s\n", *offset, op);
		// TODO: Store label offset information
		return 0;
	}

	return -1;
}

/* assemble */
int rasm_asm(const char *arch, u64 *offset, const char *str, unsigned char *data)
{
	int ret = -1;
	while(str[0]==' '||str[0]=='\t') str = str + 1;

	if (arch == NULL||str==NULL)
		return -1;

	ret = rasm_directive(arch, offset, str, data);
	if (ret != -1)
		return ret;

	if (str[0]=='\0'||str[1]=='\0' || strchr(str,'?')!=NULL)
		return 0;

	if ((!strcmp(arch, "x86")) ||(!strcmp(arch, "intel")))
		ret = rasm_x86(offset, str, data);

	if (!strcmp(arch, "arm"))
		ret = rasm_arm(offset, str, data);

	if (!strcmp(arch, "java"))
		ret = rasm_java(offset, str, data);

	if (!strcmp(arch, "ppc"))
		ret = rasm_ppc(offset, str, data);

	if (ret == -1)
		fprintf(stderr, "error in '%s'\n", str);

	return ret;
}

int rasm_disasm(const char *arch, u64 *offset, const char *str, unsigned char *data)
{
	char buf[4096];
	/* TODO: Use internal disasembler */
	/* TODO: parse hexpairs and disassemble */
	snprintf(buf, 4095, "rsc dasm '%s'", str);
	system(buf);
	return 0;
}
