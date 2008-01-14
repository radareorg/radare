/*
 * Copyright (C) 2007
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

/* code analysis functions */

#include "../../code.h"
#include <string.h>
int dislen(unsigned char* opcode0, int limit);

// NOTE: bytes should be at least 16 bytes!
// XXX addr should be off_t for 64 love
int arch_x86_aop(unsigned long addr, const unsigned char *bytes, struct aop_t *aop)
{
	unsigned long *ptr = (unsigned long *)(bytes+1);
	unsigned char *ptr2 = (unsigned char *)(bytes+1);

	memset(aop, '\0', sizeof(struct aop_t));
//printf("0x%08x %02x %02x\n", addr, (unsigned char)bytes[0], (unsigned char)bytes[1]);
	aop->type = AOP_TYPE_NOP;

	switch(bytes[0]) {
	case 0xf4: // hlt
		aop->type   = AOP_TYPE_RET;
		aop->eob = 1;
		break;
	case 0xc3: // ret
	case 0xc2: // ret + 2 bytes
	case 0xcb: // lret
	case 0xcf: // iret
		aop->type   = AOP_TYPE_RET;
	//	aop->length = 1;
		aop->eob    = 1;
		break;
	//case 0xea: // far jmp
	case 0x90:
		aop->type   = AOP_TYPE_NOP;
		aop->length = 1;
		break;
	case 0x0f: // 3 byte nop
		if (bytes[1]>=0x18 && bytes[1]<=0x1f) {
			aop->type = AOP_TYPE_NOP;
			aop->length = 3;
		} else
		if (bytes[1]>=0x80 && bytes[1]<=0x8f) {
			aop->type   = AOP_TYPE_CJMP;
			aop->jump   = (unsigned long)((bytes+2)+6);
			aop->fail   = addr+6;
			aop->length = 6;
			aop->eob    = 1;
		} 
		break;
	case 0xcc: // int3
	case 0xf1: // int1
		aop->length = 1;
		aop->type   = AOP_TYPE_SWI;
		break;
	case 0xcd:
		aop->length = 2;
		aop->type   = AOP_TYPE_SWI;
		break;
	case 0xe8: // call
		aop->type   = AOP_TYPE_CALL;
		aop->length = 5;
		aop->jump   = addr+*ptr+5; //(unsigned long)((bytes+1)+5);
		aop->fail   = addr+5;
//printf("addr: %08x\n call %08x \n ret %08x\n", addr, aop->jump, aop->fail);
		aop->eob    = 1;
		break;
	case 0xe9: // jmp
		aop->type   = AOP_TYPE_JMP;
		aop->length = 5;
		aop->jump   = (unsigned long)((bytes+1)+5);
		aop->fail   = 0L;
		aop->eob    = 1;
		break;
	case 0xeb: // short jmp 
		aop->type   = AOP_TYPE_JMP;
		aop->length = 2;
		aop->jump   = addr+(unsigned long)((bytes[1]))+2;
		aop->fail   = 0L;
		aop->eob    = 1;
		break;
	case 0xf2: // repnz
	case 0xf3: // repz
		aop->type   = AOP_TYPE_REP;
		//aop->length = dislen((unsigned char *)&bytes); //instLength(bytes, 16, 0);
		aop->jump   = 0L;
		aop->fail   = 0L;
		break;
	case 0xFF:
		if (bytes[0]>=0x50 && bytes[1]<=0x6f) {
			aop->type = AOP_TYPE_UJMP;
			aop->eob    = 1;
		} else
		if (bytes[1]>=0xd0 && bytes[1]<=0xe7) {
			aop->type = AOP_TYPE_UJMP;
		//	aop->length = 2;
			aop->eob    = 1;
		}
		break;
#if 0
	case0xF
		/* conditional jump */
		if (bytes[1]>=0x80&&bytes[1]<=0x8F) {
			aop->type   = AOP_TYPE_CJMP;
			aop->length = 6;
			aop->jump   = (unsigned long)((bytes+2)+6);
			aop->fail   = addr+6;
			aop->eob    = 1;
			return 5;
		}
		break;
#endif
	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
	case 0x76:
	case 0x77:
	case 0x78:
	case 0x79:
	case 0x7a:
	case 0x7b:
	case 0x7c:
	case 0x7d:
	case 0x7e:
	case 0x7f: {
		int bo = (int)((char) bytes[1]);
		/* conditional jump */
		//if (bytes[1]>=0x80&&bytes[1]<=0x8F) {
			aop->type   = AOP_TYPE_CJMP;
			aop->length = 2;
		//	aop->jump   = (unsigned long)((bytes+2)+6);
			aop->jump   = addr+bo+2; //(unsigned long)((bytes+1)+5);
			aop->fail   = addr+2;
			aop->eob    = 1;
			return 5;
		}
		break;
	//default:
		//aop->type = AOP_TYPE_UNK;
	}

	//if (aop->length == 0)
	aop->length = dislen((unsigned char *)bytes, 64); //instLength(bytes, 16, 0);
		//aop->length = instLength(bytes, 16, 0);

	return aop->length;
}
