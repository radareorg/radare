
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

int rasm_arm(off_t offset, char *str, unsigned char *data)
{
	char op[128];
	char *arg;

	strncpy(op, str, 120);
 	arg = strchr(op, ' ');
	if (arg) {
		arg[0] = '\0';
		arg = arg + 1;
	}

	if (!strcmp(op, "trap")) {
		memset(data, "\xe7\xff\xde\xf", 4);
		return 4;
	} else
	if (!strcmp(op, "int")) {
		unsigned int sc = get_offset(arg);
		unsigned char *ptr = (uchar *)&sc;
		data[0] = 0x5f; // 0xff ??
		data[1] = ptr[2];
		data[2] = ptr[1];
		data[3] = ptr[0];
		return 4;
	} else
	if (!strcmp(op, "hang")) {
		memset(data, "\xea\xfe\xff\xff", 4);
		return 4;
	} else
	if (!strcmp(op, "jmp")) {
		off_t dst = get_math(arg);
		unsigned long addr = (dst-8)/4;
		unsigned char *ptr = (uchar *)&addr;

		data[0] = 0xea;
		data[1] = ptr[2];
		data[2] = ptr[1];
		data[3] = ptr[0];
		return 4;
	} else
	if (!strcmp(op, "jz")) {
	} else
	if (!strcmp(op, "nop")) {
		memset(data, '\0', 4);
	} else
		return -1;

	return 4;
}
