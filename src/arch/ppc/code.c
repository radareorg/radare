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

/* code analysis functions */

#include "../../main.h"
#include "../../code.h"
#include <string.h>

// NOTE: bytes should be at least 16 bytes!
// XXX addr should be off_t for 64 love
int arch_ppc_aop(u64 addr, const u8 *bytes, struct aop_t *aop)
{
	// TODO swap endian here??
	int opcode = (bytes[0] & 0xf8) >> 3; // bytes 0-5
	short baddr  = ((bytes[2]<<8) | (bytes[3]&0xfc));// 16-29
	int aa     = bytes[3]&0x2;
	int lk     = bytes[3]&0x1;
	//if (baddr>0x7fff)
	//	baddr = -baddr;

	memset(aop, '\0', sizeof(struct aop_t));
//printf("0x%08x %02x %02x\n", addr, (unsigned char)bytes[0], (unsigned char)bytes[1]);
	aop->type = AOP_TYPE_NOP;
	aop->length = 4;

//printf("OPCODE IS %08x : %02x (opcode=%d) baddr = %d\n", addr, bytes[0], opcode, baddr);

	switch(opcode) {
	case 11: // cmpi
		aop->type = AOP_TYPE_CMP;
		break;
	case 9: // pure branch
		if (bytes[0] == 0x4e) {
			// bctr
		} else {
			aop->jump = (aa)?(baddr):(addr+baddr);
			if (lk) {
				aop->fail = addr+4;
			}
		}
		aop->eob = 1;
		break;
	case 6: // bc // conditional jump
		aop->type = AOP_TYPE_JMP;
		aop->jump = (aa)?(baddr):(addr+baddr+4);
		aop->eob = 1;
		break;
	case 7: // sc/svc
		aop->type = AOP_TYPE_SWI;
		break;
#if 0
	case 15: // bl
		// OK
		aop->type = AOP_TYPE_CJMP;
		aop->jump = (aa)?(baddr):(addr+baddr);
		aop->fail = addr+4;
		aop->eob = 1;
		break;
#endif
	case 8: // bne i tal
		// OK
		aop->type = AOP_TYPE_CJMP;
		aop->jump = (aa)?(baddr):(addr+baddr+4);
		aop->fail = addr+4;
		aop->eob = 1;
		break;
	case 19: // bclr/bcr/bcctr/bcc
		aop->type = AOP_TYPE_RET; // jump to LR
		if (lk) {
			aop->jump = 0xFFFFFFFF; // LR ?!?
			aop->fail = addr+4;
		}
		aop->eob = 1;
		break;
	}

	// 10000
	// AA = absolute address
	// LK = link bit
	// BD = bits 16-19 
	//   address
	// if (AA) {
	//   address = (int32) BD << 2
	// } else {
	//   address += (int32) BD << 2
	// }
	//AA LK
	//30 31
	// 0  0  bc
	// 1  0  bca
	// 0  1  bcl
	// 1  1  bcla

	// 10011
	// BCCTR
	// LK = 31

	// bclr or bcr (Branch Conditional Link Register) Instruction
	// 10011

	// 6-29 -> LL (addr) ?
	// B  10010 -> branch
	// 30 31
	// 0  0   b
	// 1  0   ba
	// 0  1   bl
	// 1  1   bla
	// SC SYSCALL 5 first bytes 10001
	// SVC SUPERVISORCALL
	// 30 31
	// 0  0  svc
	// 0  1  svcl
	// 1  0  svca
	// 1  1  svcla

	return 4; /* all opcodes are 4 byte length */
}

#if 0
BO (6:10) 	Specifies options for the branch conditional instructions. The possible encodings for the BO field are:

BO
    Description 
0000x
    Decrement Count Register (CTR). Branch if the decremented CTR value is not equal to 0 and the condition is false. 
0001x
    Decrement CTR. Branch if the decremented CTR value is 0 and the condition is false. 
001xx
    Branch if the condition is false. 
0100x
    Decrement CTR. Branch if the decremented CTR value is not equal to 0 and the condition is true. 
0101x
    Decrement CTR. Branch if the decremented CTR value is equal to 0 and the condition is true. 
011x
    Branch if the condition is true. 
1x00x
    Decrement CTR. Branch if the decremented CTR value is not equal to 0. 
1x01x
    Decrement CTR. Branch if bits 32-63 of the CTR are 0 (PowerPC) or branch if the decremented CTR value is equal to 0 (POWER family). 
1x1xx
    Branch always. 
#endif
