/* DISARM
 * Simple-minded ARM disassembler
 * (C) Copyright 1999 Kevin F. Quinn, all rights reserved
 * 
 * Updates for radare integration and disassembly fixups
 *
 *     pancake <youterm.com>
 *
 * // XXX this code is full of GCC extensions!!! WARNING!!! //
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.*
 *
 * The GNU General Public License is also available on the World Wide Web
 * at http://www.gnu.org/copyleft/gpl.html
 *
 * To contact the author, email kevq@banana.demon.co.uk
 *
 * The ARM instruction set is very easy to decode, thanks to its simplicity
 * and tidiness.  All ARM instructions are 32 bits wide, which removes any
 * of the difficulties that arise due to variable-length instructions seen
 * on other processors.
 *
 * The ARM family also has a "THUMB" instruction set that operates in much
 * the same vein but consists of 16-bit instructions.
 *
 * I previously wrote an ARM disassembler in C whilst learning C - and I
 * made rather gratuitous use of the ? operator to make a largely
 * incomprehensible mess :)  This version is supposed to be more easily
 * understood.
 *
 * Unfortunately, once I'd written it like this I discovered that the C
 * method for laying out structures is strictly non-portable (dumb language!)
 * I've become used to being able to express layouts accurately in a
 * platform-independent fashion in Ada.  Anyway, the upshot is that this
 * only works on little-endian (so-called "Intel bytesex") architectures.
 */


#include <stdio.h>
#include <string.h>
#include "disarm.h"

static int current_offset = 0;
int color;

#define TRUE (0==0)
#define FALSE (0==1)

#define DISARM_COMMENTPOS 32
#define DISARM_RLCOMMENTPOS 47

/* String constants */
static const char *registers[16] = {
	"r0", "r1", "r2", "r3",
	"r4", "r5", "r6", "r7",
	"r8", "r9", "r10","fp",
	"ip", "sp", "lr", "pc"
};

static const char *cregister[16] = {
	"cr0",  "cr1",  "cr2",  "cr3",
	"cr4",  "cr5",  "cr6",  "cr7",
	"cr8",  "cr9",  "cr10", "cr11",
	"cr12", "cr13", "cr14", "cr15"
};

static const char *stacktext[4][2] = {
	{ "da", "Empty, descending" },
	{ "db", "Full,  descending" },
	{ "ia", "Empty, ascending" }, // is this db?? 0xe92dd800 is stmdb
	{ "ib", "Empty, ascending" }
};

static const char *opcodes[16] = {
	"and", "eor", "sub", "rsb",
	"add", "adc", "sbc", "rsc",
	"tst", "teq", "cmp", "cmn",
	"orr", "mov", "bic", "mvn"
};

static const char *shifts[4] = {
	"lsl", "lsr", "asr", "ror"
};

/* not used ? */
static const char *condtext[16][2] = {
	{ "eq", "Equal (Z)" },
	{ "ne", "Not equal (!Z)" },
	{ "cs", "Unsigned higher or same (C)" },
	{ "cc", "Unsigned lower (!C)" },
	{ "mi", "Negative (N)" },
	{ "pl", "Positive or zero (!N)" },
	{ "vs", "Overflow (V)" },
	{ "nv", "No overflow (!V)" },
	{ "hi", "Unsigned higher (C&!Z)" },
	{ "ls", "Unsigned lower or same (!C|Z)" },
	{ "ge", "Greater or equal ((N&V)|(!N&!V))" },
	{ "lt", "Less than ((N&!V)|(!N&V)" },
	{ "gt", "Greater than(!Z&((N&V)|(!N&!V)))" },
	{ "le", "Less than or equal (Z|(N&!V)|(!N&V))" },
	{ "", "Always" },
	{ "nv", "Never - Use MOV R0,R0 for nop" }
};

/* Type definitions for instructions */
typedef unsigned long int QByte; /* 32 bits */
typedef int CondPosT;

typedef struct dbytes {
	QByte dbyte0 : 16; QByte dbyte1 : 16;
} DBytes;

typedef struct bytes {
	QByte byte0 : 8; QByte byte1 : 8; QByte byte2 : 8; QByte byte3 : 8;
} Bytes;

typedef struct nybbles {
	QByte nybble0 : 4; QByte nybble1 : 4; QByte nybble2 : 4; QByte nybble3 : 4;
	QByte nybble4 : 4; QByte nybble5 : 4; QByte nybble6 : 4; QByte nybble7 : 4;
} Nybbles;

typedef struct bits {
	QByte bit00 : 1; QByte bit01 : 1; QByte bit02 : 1; QByte bit03 : 1;
	QByte bit04 : 1; QByte bit05 : 1; QByte bit06 : 1; QByte bit07 : 1;
	QByte bit08 : 1; QByte bit09 : 1; QByte bit10 : 1; QByte bit11 : 1;
	QByte bit12 : 1; QByte bit13 : 1; QByte bit14 : 1; QByte bit15 : 1;
	QByte bit16 : 1; QByte bit17 : 1; QByte bit18 : 1; QByte bit19 : 1;
	QByte bit20 : 1; QByte bit21 : 1; QByte bit22 : 1; QByte bit23 : 1;
	QByte bit24 : 1; QByte bit25 : 1; QByte bit26 : 1; QByte bit27 : 1;
	QByte bit28 : 1; QByte bit29 : 1; QByte bit30 : 1; QByte bit31 : 1;
} Bits;

typedef struct swi {
	QByte comment : 24;
	QByte mustbe1111 : 4;
	QByte condition : 4;
} SWI;

typedef struct cpregtrans {
	QByte CRm : 4;
	QByte mustbe1 : 1;
	QByte CP : 3;
	QByte CPNum : 4;
	QByte Rd : 4;
	QByte CRn : 4;
	QByte L : 1;
	QByte CPOpc : 3;
	QByte mustbe1110 : 4;
	QByte condition : 4;
} CPRegTrans;

typedef struct cpdataop {
	QByte CRm : 4;
	QByte mustbe0 : 1;
	QByte CP : 3;
	QByte CPNum : 4;
	QByte CRd : 4;
	QByte CRn : 4;
	QByte CPOpc : 4;
	QByte mustbe1110 : 4;
	QByte condition : 4;
} CPDataOp;

typedef struct cpdatatrans {
	QByte offset : 8;
	QByte CPNum : 4;
	QByte CRd : 4;
	QByte Rn : 4;
	QByte L : 1;
	QByte W : 1;
	QByte N : 1;
	QByte U : 1;
	QByte P : 1;
	QByte mustbe110 : 3;
	QByte condition : 4;
} CPDataTrans;

typedef struct branch {
	QByte offset : 23;
	QByte sign : 1;
	QByte L : 1;
	QByte mustbe101 : 3;
	QByte condition : 4;
} Branch;

typedef struct blockdatatrans {
	QByte registerlist : 16;
	QByte Rn : 4;
	QByte L : 1;
	QByte W : 1;
	QByte S : 1;
	QByte U : 1;
	QByte P : 1;
	QByte mustbe100 : 3;
	QByte condition : 4;
} BlockDataTrans;

typedef struct undefined {
	QByte dontcareA : 4;
	QByte mustbe1 : 1;
	QByte dontcareB : 20;
	QByte mustbe011 : 3;
	QByte condition : 4;
} Undefined;

typedef struct singledatatrans {
	QByte offset : 12;
	QByte Rd : 4;
	QByte Rn : 4;
	QByte L : 1;
	QByte W : 1;
	QByte B : 1;
	QByte U : 1;
	QByte P : 1;
	QByte I : 1;
	QByte mustbe01 : 2;
	QByte condition : 4;
} SingleDataTrans;

typedef struct singledataswap {
	QByte Rm : 4;
	QByte mustbe00001001 : 8;
	QByte Rd : 4;
	QByte Rn : 4;
	QByte mustbe00 : 2;
	QByte B : 1;
	QByte mustbe00010 : 5;
	QByte condition : 4;
} SingleDataSwap;

typedef struct multiply {
	QByte Rm : 4;
	QByte mustbe1001 : 4;
	QByte Rs : 4;
	QByte Rn : 4;
	QByte Rd : 4;
	QByte S : 1;
	QByte A : 1;
	QByte mustbe000000 : 6;
	QByte condition : 4;
} Multiply;

typedef struct msrregtopsr {
	QByte Rm : 4;
	QByte mustbe00000000 : 8;
	QByte mustbe1010011111 : 10;
	QByte Pd : 1;
	QByte mustbe00010 : 5;
	QByte condition : 4;
} MSRRegToPSR;

typedef struct msrtopsrflags {
	QByte operand : 12;
	QByte mustbe1010001111 : 10;
	QByte Pd : 1;
	QByte mustbe10 : 2;
	QByte I : 1;
	QByte mustbe00 : 2;
	QByte condition : 4;
} MSRToPSRFlags;

typedef struct mrspsrtoreg {
	QByte mustbe000000000000 : 12;
	QByte Rd : 4;
	QByte mustbe001111 : 6;
	QByte Ps : 1;
	QByte mustbe00010 : 5;
	QByte condition : 4;
} MRSPSRToReg;

typedef struct dataproc {
	QByte operand2 : 12;
	QByte Rd : 4;
	QByte Rn : 4;
	QByte S : 1;
	QByte opcode : 4;
	QByte I : 1;
	QByte mustbe00 : 2;
	QByte condition : 4;
} DataProc;

typedef union access32 {
	QByte qbyte;
	DBytes dbytes;
	Bytes bytes;
	Nybbles nybbles;
	Bits bits;
	DataProc dataproc;
	MRSPSRToReg mrspsrtoreg;
	MSRToPSRFlags msrtopsrflags;
	MSRRegToPSR msrregtopsr;
	Multiply multiply;
	SingleDataSwap singledataswap;
	SingleDataTrans singledatatrans;
	Undefined undefined;
	BlockDataTrans blockdatatrans;
	Branch branch;
	CPDataTrans cpdatatrans;
	CPDataOp cpdataop;
	CPRegTrans cpregtrans;
	SWI swi;
} Access32;

typedef struct shiftreg {
	QByte Rm : 4;
	QByte mustbe1 : 1;
	QByte shifttype : 2;
	QByte mustbe0 : 1;
	QByte Rs : 4;
} ShiftReg;

typedef struct shiftimmed {
	QByte Rm : 4;
	QByte mustbe0 : 1;
	QByte shifttype : 2;
	QByte shiftconst : 5;
} ShiftImmed;

/* not used ? */
typedef union shiftrm {
	QByte operand : 12;
	ShiftReg shiftreg;
	ShiftImmed shiftimmed;
} ShiftRm;


/* Working areas */
static char workstr[DISARM_MAXSTRINGSIZE];
static char shiftstr[DISARM_MAXSTRINGSIZE];
static char disasmstr[DISARM_MAXSTRINGSIZE];


/* Functions to decode operands */

static char *shiftrm (ShiftRm shiftdata)
{

	if (0 == shiftdata.shiftimmed.mustbe0)
	{
		/* Immediate */
		if (0 == shiftdata.shiftimmed.shiftconst)
		{
			switch (shiftdata.shiftimmed.shifttype)
			{
				case 0: /* LSL */
					sprintf(shiftstr, "%s, lsl #0",
							registers[shiftdata.shiftimmed.Rm]);
					break;
				case 1: /* LSR */
					sprintf(shiftstr, "%s, lsr #32",
							registers[shiftdata.shiftimmed.Rm]);
					break;
				case 2: /* ASR */
					sprintf(shiftstr, "%s, asr #32",
							registers[shiftdata.shiftimmed.Rm]);
					break;
				case 3: /* ROR */
					sprintf(shiftstr, "%s, ror",
							registers[shiftdata.shiftimmed.Rm]);
					break;
			}
		}
		else
		{
			sprintf(shiftstr, "%s,%s #%d",
					registers[shiftdata.shiftimmed.Rm],
					shifts[shiftdata.shiftimmed.shifttype],
					shiftdata.shiftimmed.shiftconst);
		}
	}
	else
	{
		/* Register */
		sprintf(shiftstr, "%s,%s %s",
				registers[shiftdata.shiftreg.Rm],
				shifts[shiftdata.shiftreg.shifttype],
				registers[shiftdata.shiftreg.Rs]);
	}
	return (shiftstr);
}

static char *cprelative (QByte Rn, QByte offset, QByte U, QByte P)
{
	if (0 == P) {
		sprintf(workstr, "[%s,#%s%2.2lX]",
				cregister[Rn],
				(0 == U)?"-":"",
				offset);
	} else {
		sprintf(workstr, "[%s],#%s%2.2lX",
				cregister[Rn],
				(0 == U)?"-":"",
				offset);
	}
	return (workstr);
}

static char *reglist (QByte reglst)
{
	int mask = 1;
	int reg;
	int pos = 0;
	int inrun = FALSE;

	for (reg=0; reg<16; reg++) {
		if (0 != (reglst & mask)) {
			if (0 != pos)
				workstr[pos++]=(inrun)?'-':',';
			sprintf(workstr+pos, registers[reg]);
			if (inrun == TRUE) pos--;
			else pos = pos + strlen(registers[reg]);

			inrun = TRUE;
		} else
			inrun = FALSE;
		mask = mask << 1;
	}
	return (workstr);
}

static char *dataoperand (QByte operand, QByte I)
{
	QByte data, rotate;
	if (0 == I) {
#if ee
		sprintf(workstr, "%s",
				shiftrm((ShiftRm)operand));
#endif
	} else {
		rotate = (operand >> 8) * 2;
		data = operand & 0xFF;
		sprintf(workstr, " 0x%lX",
				(data >> rotate)|(data<<(32-rotate)));
	}
	return (workstr);
}

static char *sdtsource (QByte Rn, QByte offset, QByte W, QByte U, QByte P, QByte I)
{
	if (0 == I) {
		if (0 == P) {
#if ee
			sprintf(workstr, "[%s],%s%s",
					registers[Rn], (0 == U)?"-":"",
					shiftrm((ShiftRm)offset));
#endif
		} else {
#if ee
			sprintf(workstr, "[%s,%s%s]%s",
					registers[Rn], (0 == U)?"-":"",
					shiftrm((ShiftRm)offset),
					(0 == W)?"":"!");
#endif
		}
	} else {
		if (0 == P) {
			sprintf(workstr, "[%s],#%s0x%7.7lX",
					registers[Rn], (0 == U)?"-":"",
					offset);
		} else {
			sprintf(workstr, "[%s,#%s0x%7.7lX]%s",
					registers[Rn], (0 == U)?"-":"",
					offset, (0 == W)?"":"!");
		}
	}
	return (workstr);
}


/* Decode functions for each instruction type */
static CondPosT decode_swi (SWI swi)
{
	sprintf(disasmstr, "%sswi  0x%08x%s", 
		(color)?"\e[31m":"",
		swi.comment,
		(color)?"\e[0m":"");
	return (3);
}

static CondPosT decode_data_transfer2 (Bytes bytes)
{
	sprintf(disasmstr, "ldr %s, [%s], 0x%x", 
			registers[bytes.byte1>>4],
			registers[bytes.byte2&0xf],
			(int)(bytes.byte0));
	if ((bytes.byte2>>4) == 2)
		memcpy(disasmstr, "str", 3);
	return (3);
}

static CondPosT decode_data_transfer (Bytes bytes)
{
	sprintf(disasmstr, "ldr %s, [%s, 0x%x]",
			registers[bytes.byte1>>4],
			registers[bytes.byte2&0xf],
			(int)(bytes.byte0));
	if ((bytes.byte2>>4) == 2)
		memcpy(disasmstr, "str", 3);
	return (3);
}

static CondPosT decode_cpregtrans (CPRegTrans cpregtrans)
{
	sprintf(disasmstr, "m%s  p%d,%d,%s,%s,%s,%d",
			(0 == cpregtrans.L)?"cr":"rc",
			cpregtrans.CPNum,
			cpregtrans.CPOpc,
			cregister[cpregtrans.Rd],
			cregister[cpregtrans.CRn],
			cregister[cpregtrans.CRm],
			cpregtrans.CP);
	return (3);
}


static CondPosT decode_cpdataop (CPDataOp cpdataop)
{
	sprintf(disasmstr, "cdp  p%d,%d,%s,%s,%s,%d",
			cpdataop.CPNum,
			cpdataop.CPOpc,
			cregister[cpdataop.CRd],
			cregister[cpdataop.CRn],
			cregister[cpdataop.CRm],
			cpdataop.CP);
	return (3);
}


static CondPosT decode_cpdatatrans (CPDataTrans cpdatatrans)
{
	sprintf(disasmstr, "%sc  p%d,N%d%s,%s%s",
			(0 == cpdatatrans.L)?"st":"ld",
			cpdatatrans.CPNum,
			cpdatatrans.N,
			cregister[cpdatatrans.CRd],
			cprelative(cpdatatrans.Rn,
				cpdatatrans.offset,
				cpdatatrans.U,
				cpdatatrans.P),
			(0==cpdatatrans.W)?"":"!");

	if (0!=cpdatatrans.L) {
		if (0x0E != cpdatatrans.condition) 
			disasmstr[5]='l';
		else
			disasmstr[3]='l';
	}
	return (3);
}


static CondPosT decode_branch (Branch branch)
{
	QByte cp;
	char label[512];
	int addr = 0;
	unsigned char *ptr = (unsigned char *)&addr;
	Bytes* bytes = (Bytes *)&branch;

	ptr[2] = bytes->byte2;
	ptr[1] = bytes->byte1;
	ptr[0] = bytes->byte0;
	if (branch.sign)
		ptr[3]=0xff;
	addr*=4;
	addr+=8;

	label[0]='\0';
	string_flag_offset(label, current_offset+addr);
	sprintf(disasmstr, "%sb%s  0x%x ; %s%s",
			(color)?"\e[32m":"",
			condtext[(bytes->byte3&0xf0)>>4][0],
			(0==branch.sign)?(current_offset+addr):
			(current_offset+(addr)), 
			label,
			(color)?"\e[0m":"");
	if (0 == branch.L) {
		cp = 1;
	} else {
		disasmstr[(color)?6:1]='l';
		cp = 2;
	}
	return (cp);
}


static CondPosT decode_blockdatatrans (BlockDataTrans blockdatatrans)
{ 
#if 0
	CondPosT cp;
#endif
	QByte storetype;

	storetype = blockdatatrans.P * 2 +
		blockdatatrans.U;
	sprintf(disasmstr, "%sm   %s%s,{%s}%s",
			(0 == blockdatatrans.L)?"st":"ld",
			registers[blockdatatrans.Rn],
			(0 == blockdatatrans.W)?"":"!", /* writeback */
			reglist (blockdatatrans.registerlist),
			(0 == blockdatatrans.S)?"":"^"); /* S-bit, load CPSR */
	strncpy(disasmstr+3+((0x0E != blockdatatrans.condition)?2:0),
			stacktext[storetype][0], 2);
#if 0
	for (cp = strlen(disasmstr); cp < DISARM_RLCOMMENTPOS; cp++)
		disasmstr[cp] = ' ';
	disasmstr[DISARM_RLCOMMENTPOS] = ';';
	disasmstr[DISARM_RLCOMMENTPOS+1] = ' ';
	strcpy(disasmstr+DISARM_RLCOMMENTPOS+2, stacktext[storetype][1]);;
	disasmstr[DISARM_RLCOMMENTPOS+2 + strlen(stacktext[storetype][1])] = '\0';
#endif
	return (3);
}


static CondPosT decode_undefined (Access32 instruction)
{
	sprintf(disasmstr, "dcd  0x%lx", instruction.qbyte);
	return (0);
}


static CondPosT decode_singledatatrans (SingleDataTrans singledatatrans)
{
	QByte cp;

	/* Single Data Transfer */
	sprintf(disasmstr, "%sr  %s,%s",
			(0 == singledatatrans.L)?"st":"ld",
			registers[singledatatrans.Rd],
			sdtsource(singledatatrans.Rn,
				singledatatrans.offset, /* Offset field */
				singledatatrans.W, /* Writeback */
				singledatatrans.U, /* up/down */
				singledatatrans.P, /* Pre/post indexed */
				singledatatrans.I)); /* Immediate */
	cp = 11 + ((0x0E != singledatatrans.condition)?2:0);
	/* Byte (8bit) or Word (32bit) transfer */
	if (0 != singledatatrans.B) disasmstr[cp++]='B';
	/* T is present if writeback and post-indexed */
	if ((0 == singledatatrans.P) &&
			(0 != singledatatrans.W)) disasmstr[cp++]='T';
	return (3);
}


static CondPosT decode_singledataswap (SingleDataSwap singledataswap)
{
	sprintf(disasmstr, "swp  %s,%s,[%s]",
			registers[singledataswap.Rd],
			registers[singledataswap.Rm],
			registers[singledataswap.Rn]);
	if (0 != singledataswap.B) disasmstr[3 + ((0x0E != singledataswap.condition)?2:0)]='B';
	return (3);
}


static CondPosT decode_multiply (Multiply multiply)
{
	if (0 == multiply.A) {
		sprintf(disasmstr, "mul  %s, %s, %s",
				registers[multiply.Rd],
				registers[multiply.Rm],
				registers[multiply.Rs]);
	} else {
		sprintf(disasmstr, "mla  %s, %s, %s, %s",
				registers[multiply.Rd],
				registers[multiply.Rm],
				registers[multiply.Rs],
				registers[multiply.Rn]);
	}
	//if (0 != multiply.S) disasmstr[3 + ((0x0E != multiply.condition)?2:0)]='S';
	return (3);
}


static CondPosT decode_msrregtopsr (MSRRegToPSR msrregtopsr)
{
	sprintf(disasmstr, "msr  %spsr,%s",
			(0 == msrregtopsr.Pd)?"c":"s",
			registers[msrregtopsr.Rm]);
	return (3);
}


static CondPosT decode_msrtopsrflags (MSRToPSRFlags msrtopsrflags)
{
	QByte data, rotate;

	if (0 == msrtopsrflags.I) {
		data = msrtopsrflags.operand & 0xFF;
		rotate = msrtopsrflags.operand >> 8;
		sprintf(disasmstr, "msr  %sPSR_flg,#%8.8lX",
				(0 == msrtopsrflags.Pd)?"c":"s",
				(data >> rotate) | (data << (32 - rotate)));
	} else {
		sprintf(disasmstr, "msr  %sPSR_flg,%s",
				(0 == msrtopsrflags.Pd)?"c":"s",
				registers[msrtopsrflags.operand & 0xF]);
	}
	return (3);
}


static CondPosT decode_mrspsrtoreg (MRSPSRToReg mrspsrtoreg)
{
	sprintf(disasmstr, "mrs  %s,%sPSR",
			registers[mrspsrtoreg.Rd],
			(0 == mrspsrtoreg.Ps)?"c":"s");
	return (3);
}


static CondPosT decode_dataproc (DataProc dataproc)
{
	Bytes *bytes = (Bytes *)&dataproc;
	char pfx[16];

	pfx[0]='\0';
	if (
	   bytes->byte0==0
	&& bytes->byte1 == 0
	&& bytes->byte2 == 0
	&& bytes->byte3 == 0) {
		strcpy(pfx,"\e[36m");
	}

	if ( (dataproc.opcode == 0xD)  || (dataproc.opcode == 0xF)) {
		if (dataproc.I) {
			/* MOV/MVN */
			sprintf(disasmstr, "%s%s %s,%s",
					pfx, opcodes[dataproc.opcode],
					registers[dataproc.Rd],
					dataoperand(dataproc.operand2, dataproc.I));
		} else {
			/* MOV between registers*/
			if (dataproc.Rd > 15 || dataproc.operand2 > 15)
				sprintf(disasmstr, "Invalid opcode");
			else 
			sprintf(disasmstr, "%s%s %s, %s",
					pfx, opcodes[dataproc.opcode],
					registers[dataproc.Rd],
					registers[dataproc.operand2]);
		}
	} else {
		if ((dataproc.opcode >= 0x8) && (dataproc.opcode <= 0xB))
		{
			if (bytes->byte1 == 0xff) {
				/* BLX injection */
				sprintf(disasmstr, "blx %s", registers[bytes->byte0>>4]);
				//decode_branch (*branch);
			} else {
				/* CMP/CMN/TEQ/TST */
				sprintf(disasmstr, "%s%s %s, %s",
						pfx, opcodes[dataproc.opcode],
						registers[dataproc.Rn],
						dataoperand(dataproc.operand2, dataproc.I));
			}
		} else {
			if (dataproc.I) {
				/* AND/EOR/SUB/RSB/ADC/SBC/RSC/ORR/BIC */
				sprintf(disasmstr, "%s%s  %s, %s, %s",
						pfx, opcodes[dataproc.opcode],
						registers[dataproc.Rd],
						registers[dataproc.Rn],
						dataoperand(dataproc.operand2, dataproc.I));
			} else {
				/* ADD/ADC/SBC/RSC/ORR/BIC */
				sprintf(disasmstr, "%s%s  %s, %s",
						pfx, opcodes[dataproc.opcode],
						registers[dataproc.Rd],
						registers[dataproc.Rn]);
			}
		}
	}
	//if (0 != dataproc.S) disasmstr[3 + ((0x0E != dataproc.condition)?2:0)]='S';
	return (3);
}


static CondPosT decode_unknown (Access32 instruction)
{
	sprintf(disasmstr, "; data (0x%08x)", (unsigned int)instruction.qbyte);
	return (0);
}


/* Disassembler entry point */
char *disarm (RawInstruction rawinstruction, int offset)
{
	CondPosT condpos;
	Access32 instruction;

	current_offset = offset;

	instruction = (Access32)rawinstruction;

	sprintf(disasmstr, "Not decoded");
	condpos = 0; /* Position in disasmstr for condition - set to 0 to inhibit insertion of condition */

	if (0xF == instruction.swi.mustbe1111) {
		/* SWI */
		condpos = decode_swi (instruction.swi);
	} else {
		if (0xE == instruction.cpregtrans.mustbe1110) {
			if (0x1 == instruction.cpregtrans.mustbe1) {
				/* Coprocessor Register Transfer */
				condpos = decode_cpregtrans (instruction.cpregtrans);
			} else {
				/* Coprocessor Data Operation */
				condpos = decode_cpdataop (instruction.cpdataop);
			}
		} else {
			if (0x6 == instruction.cpdatatrans.mustbe110) {
				/* Coprocessor Data Transfer */
				condpos = decode_cpdatatrans (instruction.cpdatatrans);
			} else {
				if (0x5 == instruction.branch.mustbe101) {
					/* Branch */
					condpos = decode_branch (instruction.branch);
				} else {
					if (0x4 == instruction.blockdatatrans.mustbe100) {
						/* Block Data Transfer */
						if (0 == instruction.blockdatatrans.registerlist) {
							/*
							   sprintf(disasmstr,
							   "dcd  0x%8.8lX  ; Illegal block data transfer - register list 0",
							   instruction.qbyte);
							   condpos = 0;
							 */
							condpos = decode_data_transfer (instruction.bytes);
						} else {
							condpos = decode_blockdatatrans (instruction.blockdatatrans);
						}
					} else {
						if ((0x3 == instruction.undefined.mustbe011) &&
								(0x1 == instruction.undefined.mustbe1)) {
							/* Undefined */
							condpos = decode_undefined (instruction);
						} else {
							if (0x2 == instruction.singledatatrans.mustbe01) {
								/* Single Data Transfer */
								condpos = decode_singledatatrans (instruction.singledatatrans);
							} else {
								if ((0x2 == instruction.singledataswap.mustbe00010) &&
										(0x0 == instruction.singledataswap.mustbe00) &&
										(0x09 == instruction.singledataswap.mustbe00001001))
								{
									/* Single Data Swap */
									if ((15 == instruction.singledataswap.Rd) ||
											(15 == instruction.singledataswap.Rn) ||
											(15 == instruction.singledataswap.Rm))
									{
										sprintf(disasmstr,
												"dcd  0x%8.8lX  ; Illegal SWP - R15 not allowed as operand",
												instruction.qbyte);
										condpos = 0;
									} else {
										condpos = decode_singledataswap (instruction.singledataswap);
									}
								} else {
									if ((0x0 == instruction.multiply.mustbe000000) &&
											(0x9 == instruction.multiply.mustbe1001))
									{
										/* Multiply */
										condpos = decode_multiply (instruction.multiply);
									} else {
										if ((0x2 == instruction.msrregtopsr.mustbe00010) &&
												(0x29F == instruction.msrregtopsr.mustbe1010011111) &&
												(0x0 == instruction.msrregtopsr.mustbe00000000))
										{
											/* MSR transfer reg to PSR */
											if ((0 == instruction.msrtopsrflags.I) &&
													(0 == (instruction.msrtopsrflags.operand & 0xFF0)))
											{
												sprintf(disasmstr,
														"dcd  0x%8.8lX  ; Illegal MSR - Immed but bits 4-11 not 0",
														instruction.qbyte);
												condpos = 0;
											} else {
												condpos = decode_msrregtopsr (instruction.msrregtopsr);
											}
										} else {
											if ((0x0 == instruction.msrtopsrflags.mustbe00) &&
													(0x2 == instruction.msrtopsrflags.mustbe10) &&
													(0x29F == instruction.msrtopsrflags.mustbe1010001111))
											{
												/* MSR transfer immed or reg to PSR Flags */
												condpos = decode_msrtopsrflags (instruction.msrtopsrflags);
											} else {
												if ((0x2 == instruction.mrspsrtoreg.mustbe00010) &&
														(0xF == instruction.mrspsrtoreg.mustbe001111) &&
														(0x0 == instruction.mrspsrtoreg.mustbe000000000000))
												{
													/* MSR transfer immed or reg to PSR Flags */
													condpos = decode_mrspsrtoreg (instruction.mrspsrtoreg);
												} else {
													if (0x0 == instruction.dataproc.mustbe00)
													{
														/* Data Processing/PSR Transfer */
														condpos = decode_dataproc (instruction.dataproc);
													} else {
														switch(instruction.bytes.byte3) {
															case 0xe5:
																condpos = decode_data_transfer(instruction.bytes);
																break;
															case 0xe4:
																condpos = decode_data_transfer2(instruction.bytes);
																break;
															default:
																/* Unkwnown */
																condpos = decode_unknown (instruction);
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return (disasmstr);
}

#if 0
main()
{
	RawInstruction foo = 0xe92dd800; //0xc8239fe5;//e59f23c8;
	//RawInstruction foo = 0xef900014; //0xe59f23c8;

	printf("%08s\n", disarm(foo));
}
#endif
