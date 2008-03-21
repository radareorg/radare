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


int rasm_ppc(u64 offset, const char *str, unsigned char *data)
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
		memcpy(data, "\x4e\x80\x00\x20",4);
	} else
	if (!strcmp(op, "call")) {
		off_t dst = get_math(arg);
		unsigned long addr = dst-offset; // always relative
		unsigned char *ptr = (uchar *)&addr;
		data[0] = 0x48;
		data[1] = ptr[2];
		data[2] = ptr[1];
		data[3] = ptr[0];
		data[0] = 0x48;
		data[3]|=1;
	} else
	if (!strcmp(op, "jmp")) {
		off_t dst = get_math(arg);
		unsigned long addr = dst-offset; // always relative
		unsigned char *ptr = (uchar *)&addr;
		data[0] = 0x48;
		data[1] = ptr[2];
		data[2] = ptr[1];
		data[3] = ptr[0];

		data[3]&=0xfe;
	} else
	if (!strcmp(op, "nop")) {
		memset(data, '\0', 4);
	} else
		return -1;

	return 4;
}
