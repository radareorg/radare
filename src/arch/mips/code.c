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

// http://www.mrc.uidaho.edu/mrc/people/jff/digital/MIPSir.html

#include "../../main.h"
#include "../../code.h"
#include <stdio.h>
#include <string.h>

extern int mips_mode;

// NOTE: bytes should be at least 16 bytes?
int arch_mips_aop(u64 addr, const unsigned char *bytes, struct aop_t *aop)
{
	unsigned long op;
	//endian_memcpy(&op, bytes, 4, ! config_get("cfg.endian"));
	memset(aop, '\0', sizeof(struct aop_t));
	//of &=0x3f;

	aop->type = AOP_TYPE_UNK;

	switch(op & 0x3f) {
	// J-Type
	case 2: // j
	case 3: // jal
		break;
	// R-Type
	case 1: // bltz
	case 4: // beq
	case 5: // bne
	case 6: // blez
	case 7: // bgtz
	case 8: // jr
	case 9: // jalr
		aop->type = AOP_TYPE_CJMP;
		aop->jump = -1;
		// calculate jump
		break;
	case 12:
		aop->type = AOP_TYPE_SWI;
		break;
	case 13:
		aop->type = AOP_TYPE_TRAP;
		break;
	default:
		switch(op) {
		case 0x03e00008:
		case 0x0800e003: // jr ra
			aop->type = AOP_TYPE_RET;
			break;
		case 0x0000000d:
		case 0x0d000000: // break
			aop->type = AOP_TYPE_TRAP; 
			break;
		case 0:
			aop->type = AOP_TYPE_NOP;
			break;
		default:
			switch(bytes[3]) { // TODO handle endian ?
			case 0xc:
				aop->type = AOP_TYPE_SWI;
				break;
			case 0x9:
			case 0x8:
				aop->type = AOP_TYPE_UJMP;
				break;
			case 0x21:
				aop->type = AOP_TYPE_PUSH; // XXX move 
				break;
			}
		}
	} 
	aop->length = (mips_mode==16)?2:4;
	return aop->length;
}
