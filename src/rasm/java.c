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

int rasm_java(off_t offset, char *str, unsigned char *data)
{
	char op[128];
	char *arg;

	strncpy(op, str, 120);
 	arg = strchr(op, ' ');
	if (arg) {
		arg[0] = '\0';
		arg = arg + 1;
	}
	if (!strcmp(op, "jsr")) {
		unsigned short so = htons((unsigned int)get_math(str));
		unsigned char *o=&so;
		data[0]=0xb8;
		data[1]=o[0];
		data[2]=o[1];
		return 3;
	} else
	if (!strcmp(op, "jmp")) {
		unsigned short so = htons((unsigned int)get_math(str));
		unsigned char *o=&so;
		data[0]=0xa7;
		data[1]=o[0];
		data[2]=o[1];
		return 3;
	} else
	if (!strcmp(op, "jz")) {
		unsigned short so = htons((unsigned int)get_math(str));
		unsigned char *o=&so;
		data[0]=0x99;
		data[1]=o[0];
		data[2]=o[1];
		return 3;
	} else
	if (!strcmp(op, "jnz")) {
		unsigned short so = htons((unsigned int)get_math(str));
		unsigned char *o=&so;
		data[0]=0x9a;
		data[1]=o[0];
		data[2]=o[1];
		return 3;
	} else
	if (!strcmp(op, "ret")) {
		data[0]=0xa9; // ret ?
		//data[0]=0xb1; // return ?
		return 1;
	} else
	if (!strcmp(op, "trap")) {
		data[0] = 0xca;
		return 1;
	} else
	if (!strcmp(op, "nop")) {
		data[0]=0x00;
		return 1;
	}

	return 0;
}
