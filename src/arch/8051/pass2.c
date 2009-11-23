/* pass2.c
 *
 * Functions for pass 2 of the disassembler.
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
 */


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "distypes.h"
#include "global.h"

/* nextbyte: read a byte from memory
 *
 */
#define nextbyte() (mem[addr++])

/* longaddr
 *
 */
static int longaddr(uchar op1, uchar op2, char *label)
{
	int addr;
	
	/* calculate address */
	addr = ((((int)op1)<<8) | op2);
	
	/* form label string */
	sprintf(label, "0x%04x", addr);
	
	return addr; //lbl[addr];
}

/* absaddr
 *
 */
static int absaddr(uchar opcode, uchar operand, int memPtr, char *label)
{
	int addr;
	
	/* calculate address */
	addr = ((memPtr & 0xf800) | (((int)opcode & 0xe0)<<3) | operand);
	
	/* form label string */
	sprintf(label, "0x%04x", addr);
	
	return addr;//lbl[addr];
}

/* reladdr
 *
 */
static int reladdr(uchar operand, int memPtr, char *label)
{
	int addr;
	
	/* calculate address */
	addr = memPtr + (signed char)operand;
	
	/* form label string */
	sprintf(label, "0x%04x", addr);
	
	return addr; //lbl[addr];
}


/* printhex
 *
 * Pretty format a hexadecimal number in a string.
 */
static void printhex(uchar num, char *name)
{
	if ((num >= 0xa0) || ((num <= 0xf) && (num >= 0xa)))
		snprintf(name, 5, "0%Xh", num);
	else
		snprintf(name, 5, "%Xh", num);
}

/* sfrdecode
 *
 * Convert a direct memory value to a SFR name if appropriate.
 * Otherwise output the formatted number.
 */
static void sfrdecode(uchar sfr, char *name)
{
	if (sfr & 0x80)
		strncpy(name, sfrname[sfr&0x7f], 5);
	else
		printhex(sfr, name);
}

/* sfbitdecode
 *
 * Convert a bit memory value to a SF bit name if appropriate.
 * Otherwise output the formatted number.
 */
static void sfbitdecode(uchar sfbit, char *name)
{
	if (sfbit & 0x80)
		strncpy(name, sfbitname[sfbit&0x7f], 6);
	else
		printhex(sfbit, name);
}

/* _listhex, listhex
 *
 * Output list-format address and data for the -l command line switch.
 *
 * Contributed by Peter Peres.
 */
#define listhex(len,addr,ofile) { if(Lst) _listhex(ofile,mem,addr,len); }
static void _listhex(char *file, const unsigned char *mem, int addr, int len)
{
	sprintf( file, "  %04X %02X", addr-len, mem[addr-len] );
	switch(len) {
	  case 3:
	    sprintf( file, "%02X", mem[addr-2] );
	  case 2:
	  	sprintf( file, "%02X", mem[addr-1] );
		if(len == 2)
			sprintf( file, "  ");
		break;
	  default:
	    sprintf( file, "    ");
	}
}

/* dis_inst2: Disassemble one instruction to ofile
 * 
 * Returns address immediately after instruction.
 */
int dis51_inst2(char *ofile, unsigned char *mem, int addr)
{
	uchar opcode;
	uchar op1, op2;
	char label[16];
	char name[6];
	char name2[5];
	int bytes = 1;

	/* Fetch opcode */
	opcode = nextbyte();
	
	/* Fetch second and third byte, if appropriate */
	if (op_format[opcode] > 1) {
		op1 = nextbyte();
		bytes = 2;
	}
	if (op_format[opcode] > 6) {
		op2 = nextbyte();
		bytes = 3;
	}
	
	/* Output decoded instruction */
#if 0
	if(!Lst)
		sprintf(ofile, "");
	else
		listhex(bytes, addr, ofile);
#endif
	switch(op_format[opcode]) {
		case 0:
				/* A5 is an illegal opcode */
			sprintf(ofile, "DB 85h  ; illegal opcode");
		case 1:
				/* no operands */
			sprintf(ofile, mnemonic[opcode]);
			break;
		case 2:
				/* one immediate operand */
			printhex(op1, name);
			sprintf(ofile, mnemonic[opcode], name);
			break;
		case 3:
				/* one direct operand */
			sfrdecode(op1, name);
			sprintf(ofile, mnemonic[opcode], name);
			break;
		case 4:
				/* one bit-addressed operand */
			sfbitdecode(op1, name);
			sprintf(ofile, mnemonic[opcode], name);
			break;
		case 5:
				/* one operand, relative address */
			/* relative addr calculation */
			reladdr(op1, addr, label);
			sprintf(ofile, mnemonic[opcode], label);
			break;
		case 6:
				/* one operand, absolute address */
			/* absolute addr calculation */
			absaddr(opcode, op1, addr, label);
			sprintf(ofile, mnemonic[opcode], label);
			break;
		case 7:
			/* two-byte immediate operand */
			/* MOV DPTR, #immediate16 */
			sprintf(ofile, mnemonic[opcode], op1, op2);
			break;
		case 8:
			/* two operands: direct, immediate */
			sfrdecode(op1, name);
			printhex(op2, name2);
			sprintf(ofile, mnemonic[opcode], name, name2);
			break;
		case 9:
				/* two operands: direct, direct */
			/* (operands in reverse order) */
			sfrdecode(op1, name);
			sfrdecode(op2, name2);
			sprintf(ofile, mnemonic[opcode], name2, name);
			break;
		case 10:
				/* two operands: immediate, relative */
			printhex(op1, name);
			reladdr(op2, addr, label);
			sprintf(ofile, mnemonic[opcode], name, label);
			break;
		case 11:
				/* two operands: direct, relative */
			sfrdecode(op1, name);
			reladdr(op2, addr, label);
			sprintf(ofile, mnemonic[opcode], name, label);
			break;
		case 12:
				/* two operands: bit, relative */
			sfbitdecode(op1, name);
			reladdr(op2, addr, label);
			sprintf(ofile, mnemonic[opcode], name, label);
			break;
		case 13:
				/* long address */
			/* long addr calculation */
			longaddr(op1, op2, label);
			sprintf(ofile, mnemonic[opcode], label);
			break;
		default:
				/* error in op_format table */
			sprintf(ofile, "DB 0%Xh  ; error in op_format table",
			        opcode);
	}
	
	return addr;
}

/* pass2: Disassemble memory to given output file (pass 2)
 *
 */
void pass2(char *ofile, unsigned char *mem)
{
	int addr = 0;
	uchar empty = 1; /* 1 for no code/data, 0 for code or data */
	
	while (addr < 65536)
	{
#if 0
		/* Step 1: check if memory is empty */
		if ((hf->flag[addr] != EMPTY) && (empty))
			/* We've changed from empty to non-empty,
			 * so start a new segment.
			 */
			sprintf(ofile, "CSEG AT %04Xh\n", addr);
		else if (hf->flag[addr] == EMPTY) {
			/* no code or data here */
			++addr;
			empty = 1;
			continue;
		}
#endif
		empty = 0;
		
		/* Step 2: Output a label if one exists */
#if 0
		if (lbl[addr])
			sprintf(ofile, "L%04d:\n", lbl[addr]);
#endif
		
		/* Step 3: Output code or data as appropriate */
//		if (hf->flag[addr]&CODE)
			/* code here, so disassemble next instruction */
			addr = dis51_inst2(ofile, mem, addr);
#if 0
		else {
			/* data here */
			if(!Lst)
				sprintf(ofile, "\tDB 0%Xh\n", hf->mem[addr]);
			else {
				listhex(1, addr+1, ofile);
				if(isprint(hf->mem[addr]))
					sprintf(ofile, "DB 0%02Xh ; '%c'\n", hf->mem[addr], hf->mem[addr]);
				else
					sprintf(ofile, "DB 0%02Xh \n", hf->mem[addr]);
			}
			++addr;
		}
#endif
	}
	
//	sprintf(ofile, "END\n");
}
