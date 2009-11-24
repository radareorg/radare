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

#include "../../code.h"
#include <stdio.h>
#include <string.h>

int arch_z80_aop(ut64 addr, const ut8 *bytes, struct aop_t *aop)
{
	int len;
	int type;
	if (aop == NULL)
		return -1;

	memset (aop, '\0', sizeof (struct aop_t));
	switch(type) {
	case 'r':
		aop->type = AOP_TYPE_RET;
		break;
	case 'j':
		aop->type = AOP_TYPE_JMP;
		aop->jump = 0; // XXX
		break;
	case 'b':
		aop->type = AOP_TYPE_CJMP;
		aop->jump = 0; // XXX
		break;
	case 'c':
		aop->type = AOP_TYPE_CALL;
		aop->jump = 0; // XXX
		break;
	default:
		aop->type = AOP_TYPE_UNK;
	}

	// TODO: code analysis not yet implemented on z80
	len = z80_OpcodeLen (bytes, 0, &type);

	return len;
}

int z80_udis (char *str, ut8 *bytes, int len, ut64 seek) {
	int ilen = z80_OpcodeLen (bytes, 0, NULL);
	if (ilen<1) {
		/* oops invalid instruction */
		return 1;
	}
	z80_Disassemble (bytes, 0, str);
	return ilen;
}
