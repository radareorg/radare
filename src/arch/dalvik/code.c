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

#include "../../code.h"
#include <stdio.h>
#include <string.h>

// XXX ultra guarro
extern struct dalvik_op {
	char *name;
	unsigned char byte;
	int size;
} dalvik_ops[];

// NOTE: bytes should be at least 16 bytes?
int arch_dalvik_aop(u64 addr, const u8 *bytes, struct aop_t *aop)
{
	unsigned int i;
	int sz = 1;

#if 0
	/* get opcode size */
	for(i = 0;dalvik_ops[i].name != NULL;i++)
		if (bytes[0] == dalvik_ops[i].byte)
			sz = dalvik_ops[i].size;
#endif

	if (aop == NULL)
		return sz;

	memset(aop, '\0', sizeof(struct aop_t));
	aop->type = AOP_TYPE_UNK;
	aop->length = sz;

	switch(bytes[0]) {
	case 0x00: // nop
		aop->type = AOP_TYPE_NOP;
		break;
	case 0x0E: // return-void
	case 0x0F: // return
	case 0x10: // return-wide
	case 0x11: // return-object
		aop->type = AOP_TYPE_RET;
		aop->eob  = 1;
		break;
	case 0x28: // goto
	case 0x29: // goto/16
	case 0x2A: // goto/32
		aop->type = AOP_TYPE_JMP;
		aop->jump = 0; // XXX
		break;
	case 0x2d: // cmpl-float
	case 0x2e: // cmpg-float
	case 0x2f: // cmpl-double
	case 0x30: // cmpg-double
	case 0x31: // cmp-long
		aop->type = AOP_TYPE_CMP;
		break;
	case 0x31: // if-eq
	case 0x32: // if-ne
	case 0x33: // if-lt
	case 0x34: // if-ge
	case 0x35: // if-gt
	case 0x36: // if-le
	case 0x37: // if-eqz
	case 0x38: // if-nez
	case 0x39: // if-ltz
	case 0x3a: // if-gez
	case 0x3b: // if-gtz
	case 0x3c: // if-lez
		aop->type = AOP_TYPE_CJMP;
		aop->jump = 0; // XXX
		break;
	/* TODO: complete opcode table */
	}

	return sz;
}
