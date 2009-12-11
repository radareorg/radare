/* pass1.c
 *
 * Functions for pass 1 of the disassembler.
 *
 * Copyright 2001 - 2003 by David Sullins
 * 
 * This file is part of Dis51.
 * 
 * Dis51 is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, version 2 of the License.
 * 
 * Dis51 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * Dis51; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 * You may contact the author at davesullins@earthlink.net.
 *
 * Integration with radare in 2009 by pancake <@nopcode.org>
 */

#include <stdio.h>
#include "distypes.h"
#include "global.h"

/* newlbl is used by longaddr, absaddr, and reladdr 
 * Keep track of next available label
 */
//static unsigned short newlbl = 0;

/* nextbyte: read a byte from memory
 *
 */
#define nextbyte() (nextbyte_f(mem, memPtr++))
static uchar nextbyte_f(unsigned char *mem, int memPtr)
{
	//hf->flag[memPtr] |= CODE;
	return mem[memPtr];
}

/* longaddr
 *
 */
static int longaddr(uchar op1, uchar op2)
{
	int addr;
	
	/* calculate address */
	addr = ((((int)op1)<<8) | op2);
	
	/* check if this address has already been labelled */
#if 0
	if (lbl[addr] == 0)
		lbl[addr] = ++newlbl;
#endif
	
	return addr;
}

/* absaddr
 *
 */
static int absaddr(uchar opcode, uchar operand, int memPtr)
{
	int addr;
	
	/* calculate address */
	addr = ((memPtr & 0xf800) | (((int)opcode & 0xe0)<<3) | operand);
	
	/* check if this address has already been labelled */
#if 0
	if (lbl[addr] == 0)
		lbl[addr] = ++newlbl;
#endif
	
	return addr;
}

/* reladdr
 *
 */
static int reladdr(uchar operand, int memPtr)
{
	int addr;
	
	/* calculate address */
	addr = memPtr + (signed char)operand;
	
#if 0
	/* check if this address has already been labelled */
	if (lbl[addr] == 0)
		lbl[addr] = ++newlbl;
#endif
	
	return addr;
}

/* dis_inst: Disassemble one instruction (pass 1)
 * 
 * Also increments memPtr to point to the next instruction address.
 *
 * Return -1 on error.
 * Otherwise, return opcode byte.
 *
 * CAVEAT: Indirect jumps not handled (JMP @A+DPTR)
 */
int dis51_inst1(unsigned char *mem, int memPtr, int *type)
{
	uchar opcode;
	uchar op1, op2;
	int newaddr = -1;
	
	*type = ' ';
	opcode = nextbyte();
	
	switch(op_format[opcode]) {
		case 0:
			/* A5 is an illegal opcode */
			fprintf(stderr,
			   "Illegal opcode A5 at address %04X\n", memPtr-1);
			newaddr = -1;
			break;
		case 1:
			/* no operands */
			newaddr = memPtr;
			/* if this is a return, stop disassembly */
			if ((opcode & 0xef) == 0x22)
				newaddr = -1;
			/* we don't handle indirect jumps */
			else if (opcode == 0x73)
				newaddr = -1;
			break;
		case 2:
		case 3:
		case 4:
			/* one operand */
			nextbyte();
			newaddr = memPtr;
			break;
		case 5:
			/* one operand, relative address */
			op1 = nextbyte();
			/* relative addr calculation */
			newaddr = reladdr(op1, memPtr);
			/* if this is a branch, continue disassembly */
			if (opcode != 0x80) {
				*type = 'b';
				//printf("--> branch\n");
			}
			break;
		case 6:
			/* one operand, absolute address */
			op1 = nextbyte();
			/* absolute addr calculation */
			newaddr = absaddr(opcode, op1, memPtr);
			/* if this is a call, continue disassembly */
			if (opcode & 0x10) {
				*type = 'c';
				//printf("--> call\n");
			}
			break;
		case 7:
		case 8:
		case 9:
			/* two operands */
			nextbyte();
			nextbyte();
			newaddr = memPtr;
			break;
		case 10:
		case 11:
		case 12:
			/* two operands, relative address */
			nextbyte();
			op2 = nextbyte();
			/* relative addr calculation */
			newaddr = reladdr(op2, memPtr);
			/* this is always a branch instruction */
			*type = 'j';
			//printf (" --> jmp\n");
			break;
		case 13:
			/* long address */
			op1 = nextbyte();
			op2 = nextbyte();
			/* long addr calculation */
			newaddr = longaddr(op1, op2);
			/* if this is a call, continue disassembly */
			if (opcode == 0x12) {
				*type = 'c';
			//	printf (" --> call\n");
			}
			break;
		default:
			/* error in op_format table */
			fprintf(stderr, 
			     "Invalid opcode format, error in format table\n");
			newaddr = -1;
			break;
	}
	
	return newaddr;
}

#if 0
/* pass1: Disassemble instructions starting at given entry point (pass 1)
 *
 */
void pass1(struct hexfile *hf, int addr)
{
	while ((addr != -1) && (hf->flag[addr] == 0))
		/* no error, we haven't been here before, and non-empty */
		/* disassemble next instruction */
		addr = dis_inst1(hf, addr);
}
#endif
