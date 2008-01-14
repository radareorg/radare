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

int rasm_x86(off_t offset, char *str, unsigned char *data)
{
	char op[128];
	char *arg;

	strncpy(op, str, 120);
 	arg = strchr(op, ' ');
	if (arg) {
		arg[0] = '\0';
		arg = arg + 1;
	}
	if (!strcmp(op, "ret")) {
		data[0]='\xc3';
		return 1;
	} else
	if (!strcmp(op, "trap")) {
		data[0]='\xcc';
		return 1;
	} else
	if (!strcmp(op, "int")) {
		int a = (int)get_offset(arg);
		data[0]='\xcd';
		data[1]=(unsigned char)a;
		return 2;
	} else
	if (!strcmp(op, "hang")) {
		data[0]='\xeb';
		data[1]='\xfe';
		return 2;
	} else
	if (!strcmp(op, "call")) {
		off_t dst = get_math(arg);
		unsigned long addr = dst-5;
		unsigned char *ptr = (uchar *)&addr;

		data[0] = '\xe8';
		data[1] = ptr[0];
		data[2] = ptr[1];
		data[3] = ptr[2];
		data[4] = ptr[3];
		return 5;
	} else
	if (!strcmp(op, "push")) {
		off_t dst = get_math(arg);
		unsigned long addr = dst;
		unsigned char *ptr = (uchar *)&addr;

		if (dst == 0) {
			if (!strcmp(arg, "eax"))
				data[0]='\x50';
			else
			if (!strcmp(arg, "ebx"))
				data[0]='\x53';
			else
			if (!strcmp(arg, "ecx"))
				data[0]='\x51';
			else
			if (!strcmp(arg, "edx"))
				data[0]='\x52';
			else
			if (!strcmp(arg, "esi"))
				data[0]='\x56';
			else
			if (!strcmp(arg, "edi"))
				data[0]='\x57';
			else
			if (!strcmp(arg, "ebp"))
				data[0]='\x55';
			else
			if (!strcmp(arg, "esp"))
				data[0]='\x54';
			else
				return 0; // invalid register name to push
			
			return 1;
		}

		data[0] = '\x68';
		data[1] = ptr[0];
		data[2] = ptr[1];
		data[3] = ptr[2];
		data[4] = ptr[3];
		return 5;
	} else
	if (!strcmp(op, "mov")) {
		off_t dst;
		unsigned long addr;
		unsigned char *ptr = (uchar *)&addr;
		char *arg2 = strchr(arg, ',');
		if (arg2== NULL) {
			eprintf("Invalid syntax\n");
			return 0;
		}
		arg2[0]='\0';
		dst = get_math(arg2+1);
		addr = dst;

		data[0]='\xb8';
		if (!strcmp(arg, "eax"))
			data[0] = 0xb8;
		else
		if (!strcmp(arg, "ebx"))
			data[0] = 0xbb;
		else
		if (!strcmp(arg, "ebx"))
			data[0] = 0xbb;
		else
		if (!strcmp(arg, "ecx"))
			data[0] = 0xb9;
		else
		if (!strcmp(arg, "edx"))
			data[0] = 0xba;
		else
		if (!strcmp(arg, "esp"))
			data[0] = 0xbc;
		else
		if (!strcmp(arg, "ebp"))
			data[0] = 0xbd;
		else
		if (!strcmp(arg, "esi"))
			data[0] = 0xbe;
		else
		if (!strcmp(arg, "edi"))
			data[0] = 0xbf;

		data[1] =ptr[0];
		data[2] =ptr[1];
		data[3] =ptr[2];
		data[4] =ptr[3];
		return 5;
	} else
	if (!strcmp(op, "jmp")) {
		off_t dst = get_math(arg);
		unsigned long addr = dst;
		unsigned char *ptr = (uchar *)&addr;

		dst-=offset;

		if (dst>-0x80 && dst<0x7f) {
			/* relative address */
			addr-=2;
			addr-=offset;
			data[0]='\xeb';
			data[1]=(char)dst;
			return 2;
		} else {
			/* absolute address */
			addr-=5;
			data[0]='\xe9';
			data[1] =ptr[0];
			data[2] =ptr[1];
			data[3] =ptr[2];
			data[4] =ptr[3];
			return 5;
		}
	} else
	if (!strcmp(op, "jz")) {
		off_t dst = get_math(arg);
		unsigned long addr = dst;
		unsigned char *ptr = &addr;

		if (dst>-0x80 && dst<0x7f) {
			dst-=2;
			data[0]='\x74';
			data[1]=(char)dst;
			return 2;
		} else {
			eprintf("Too far conditional jump\n");
		}
	} else
	if (!strcmp(op, "nop")) {
		data[0]='\x90';
		return 1;
	}

	return 0;
}
