/*
 * Copyright (C) 2009
 *       pancake <nopcode.org>
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

/* code analysis functions */

#include "../../code.h"
#include <stdio.h>
#include <string.h>

int arch_8051_aop(ut64 addr, const ut8 *bytes, struct aop_t *aop)
{
	int ptr = 0;
	int ilen, i;
	int type;
	char str[128];

	if (aop == NULL)
		return -1;

	memset (aop, '\0', sizeof (struct aop_t));
	aop->type = AOP_TYPE_UNK;

	ptr = dis51_inst1 (bytes, 0, &type);
	if (ptr == -1) {
		printf ("Invalid instruction %02x %02x\n",
			bytes[0], bytes[1]);
		return -1;
	}
	ilen = dis51_inst2 (str, bytes, 0);

	switch (type) {
	case 'j':
		aop->type = AOP_TYPE_JMP;
		aop->jump = ptr;
		aop->eob = 1;
		break;
	case 'c':
		aop->type = AOP_TYPE_CALL;
		aop->jump = ptr;
		break;
	case 'b':
		aop->type = AOP_TYPE_CJMP;
		aop->jump = ptr;
		aop->fail = ilen + addr;
		aop->eob = 1;
		break;
	}
	aop->length = ilen;
	aop->ref = 0;

	return ilen;
}

int dis51_udis (char *str, ut8 *bytes, int len, ut64 seek) {
	int ptr, type;

	// TODO: add += seek somewhere :)
	ptr = dis51_inst1 (bytes, 0, &type);
	if (ptr == -1) {
		sprintf (str, "(invalid instruction)");
		return -1;
	}
	return dis51_inst2 (str, bytes, 0);
}
