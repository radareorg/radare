/* global.c
 *
 * Data shared by all modules
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

#include "distypes.h" 

int Lst;
//short lbl[65536];

const char mnemonic[256][20] = {
/* 0 */
	"nop",
	"ajmp %s\n",
	"ljmp %s\n",
	"rr A",
	"inc A",
	"inc %s",
	"inc @R0",
	"inc @R1",
	"inc R0",
	"inc R1",
	"inc R2",
	"inc R3",
	"inc R4",
	"inc R5",
	"inc R6",
	"inc R7",
/* 1 */
	"jbc %s, %s",
	"acall %s",
	"lcall %s",
	"rrc A",
	"dec A",
	"dec %s",
	"dec @R0",
	"dec @R1",
	"dec R0",
	"dec R1",
	"dec R2",
	"dec R3",
	"dec R4",
	"dec R5",
	"dec R6",
	"dec R7",
/* 2 */
	"jb %s, %s",
	"ajmp %s\n",
	"ret\n",
	"rl A",
	"add A, #%s",
	"add A, %s",
	"add A, @R0",
	"add A, @R1",
	"add A, R0",
	"add A, R1",
	"add A, R2",
	"add A, R3",
	"add A, R4",
	"add A, R5",
	"add A, R6",
	"add A, R7",
/* 3 */
	"jnb %s, %s",
	"acall %s",
	"reti\n",
	"rlc A",
	"addc A, #%s",
	"addc A, %s",
	"addc A, @R0",
	"addc A, @R1",
	"addc A, R0",
	"addc A, R1",
	"addc A, R2",
	"addc A, R3",
	"addc A, R4",
	"addc A, R5",
	"addc A, R6",
	"addc A, R7",
/* 4 */
	"jc %s",
	"ajmp %s\n",
	"orl %s, A",
	"orl %s, #%s",
	"orl A, #%s",
	"orl A, %s",
	"orl A, @R0",
	"orl A, @R1",
	"orl A, R0",
	"orl A, R1",
	"orl A, R2",
	"orl A, R3",
	"orl A, R4",
	"orl A, R5",
	"orl A, R6",
	"orl A, R7",
/* 5 */
	"jnc %s",
	"acall %s",
	"anl %s, A",
	"anl %s, #%s",
	"anl A, #%s",
	"anl A, %s",
	"anl A, @R0",
	"anl A, @R1",
	"anl A, R0",
	"anl A, R1",
	"anl A, R2",
	"anl A, R3",
	"anl A, R4",
	"anl A, R5",
	"anl A, R6",
	"anl A, R7",
/* 6 */
	"jz %s",
	"ajmp %s\n",
	"xrl %s, A",
	"xrl %s, #%s",
	"xrl A, #%s",
	"xrl A, %s",
	"xrl A, @R0",
	"xrl A, @R1",
	"xrl A, R0",
	"xrl A, R1",
	"xrl A, R2",
	"xrl A, R3",
	"xrl A, R4",
	"xrl A, R5",
	"xrl A, R6",
	"xrl A, R7",
/* 7 */
	"jnz %s",
	"acall %s",
	"orl C, %s",
	"jmp @A+dptr\n",
	"mov A, #%s",
	"mov %s, #%s",
	"mov @R0, #%s",
	"mov @R1, #%s",
	"mov R0, #%s",
	"mov R1, #%s",
	"mov R2, #%s",
	"mov R3, #%s",
	"mov R4, #%s",
	"mov R5, #%s",
	"mov R6, #%s",
	"mov R7, #%s",
/* 8 */
	"sjmp %s\n",
	"ajmp %s\n",
	"anl C, %s",
	"movc A, @A+PC",
	"div AB",
	"mov %s, %s",
	"mov %s, @R0",
	"mov %s, @R1",
	"mov %s, R0",
	"mov %s, R1",
	"mov %s, R2",
	"mov %s, R3",
	"mov %s, R4",
	"mov %s, R5",
	"mov %s, R6",
	"mov %s, R7",
/* 9 */
	"mov dptr, #0%X%02Xh",
	"acall %s",
	"mov %s, C",
	"movC A, @A+dptr",
	"subb A, #%s",
	"subb A, %s",
	"subb A, @R0",
	"subb A, @R1",
	"subb A, R0",
	"subb A, R1",
	"subb A, R2",
	"subb A, R3",
	"subb A, R4",
	"subb A, R5",
	"subb A, R6",
	"subb A, R7",
/* A */
	"orl C, /%s",
	"ajmp %s\n",
	"mov C, %s",
	"inc dptr",
	"mul AB",
	"",               /* undefined opcode */
	"mov @R0, %s",
	"mov @R1, %s",
	"mov R0, %s",
	"mov R1, %s",
	"mov R2, %s",
	"mov R3, %s",
	"mov R4, %s",
	"mov R5, %s",
	"mov R6, %s",
	"mov R7, %s",
/* B */
	"anl C, /%s",
	"acall %s",
	"cpl %s",
	"cpl C",
	"cjne A, #%s, %s",
	"cjne A, %s, %s",
	"cjne @R0, #%s, %s",
	"cjne @R1, #%s, %s",
	"cjne R0, #%s, %s",
	"cjne R1, #%s, %s",
	"cjne R2, #%s, %s",
	"cjne R3, #%s, %s",
	"cjne R4, #%s, %s",
	"cjne R5, #%s, %s",
	"cjne R6, #%s, %s",
	"cjne R7, #%s, %s",
/* C */
	"push %s",
	"ajmp %s\n",
	"clr %s",
	"clr C",
	"swap A",
	"xch A, %s",
	"xch A, @R0",
	"xch A, @R1",
	"xch A, R0",
	"xch A, R1",
	"xch A, R2",
	"xch A, R3",
	"xch A, R4",
	"xch A, R5",
	"xch A, R6",
	"xch A, R7",
/* D */
	"pop %s",
	"acall %s",
	"setb %s",
	"setb C",
	"da A",
	"djnz %s, %s",
	"xchd A, @R0",
	"xchd A, @R1",
	"djnz R0, %s",
	"djnz R1, %s",
	"djnz R2, %s",
	"djnz R3, %s",
	"djnz R4, %s",
	"djnz R5, %s",
	"djnz R6, %s",
	"djnz R7, %s",
/* E */
	"movx A, @dptr",
	"ajmp %s\n",
	"movx A, @R0",
	"movx A, @R1",
	"clr A",
	"mov A, %s",
	"mov A, @R0",
	"mov A, @R1",
	"mov A, R0",
	"mov A, R1",
	"mov A, R2",
	"mov A, R3",
	"mov A, R4",
	"mov A, R5",
	"mov A, R6",
	"mov A, R7",
/* F */
	"movx @dptr, A",
	"acall %s",
	"movx @R0, A",
	"movx @R1, A",
	"cpl A",
	"mov %s, A",
	"mov @R0, A",
	"mov @R1, A",
	"mov R0, A",
	"mov R1, A",
	"mov R2, A",
	"mov R3, A",
	"mov R4, A",
	"mov R5, A",
	"mov R6, A",
	"mov R7, A"
};


/* op_format table
 *
 *  0 = illegal opcode
 *  1 = no operands
 *  2 = one immediate operand
 *  3 = one direct operand
 *  4 = one bit-addressed operand
 *  5 = one relative address operand
 *  6 = one absolute address operand
 *  7 = two-byte immediate operand
 *  8 = two operands: direct, immediate
 *  9 = two operands: direct, direct
 * 10 = two operands: immediate, relative address
 * 11 = two operands: direct, relative address
 * 12 = two operands: bit address, relative address
 * 13 = two-byte long address operand
 */
const char op_format[256] = {
	 1, 6, 13, 1, 1,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0 */
	12, 6, 13, 1, 1,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 1 */
	12, 6,  1, 1, 2,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 2 */
	12, 6,  1, 1, 2,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 3 */
	 5, 6,  3, 8, 2,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 4 */
	 5, 6,  3, 8, 2,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 5 */
	 5, 6,  3, 8, 2,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 6 */
	 5, 6,  4, 1, 2,  8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /* 7 */
	 5, 6,  4, 1, 1,  9, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 8 */
	 7, 6,  4, 1, 2,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 9 */
	 4, 6,  4, 1, 1,  0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* A */
	 4, 6,  4, 1, 10, 11, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, /* B */
	 3, 6,  4, 1, 1,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* C */
	 3, 6,  4, 1, 1, 11, 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, /* D */
	 1, 6,  1, 1, 1,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* E */
	 1, 6,  1, 1, 1,  3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1  /* F */
};

const char sfbitname[128][6] = {
/* 80 */
	"P0.0", "P0.1", "P0.2", "P0.3", "P0.4", "P0.5", "P0.6", "P0.7",
/* 88 */
	"IT0", "IE0", "IT1", "IE1", "TR0", "TF0", "TR1", "TF1",
/* 90 */
	"P1.0", "P1.1", "P1.2", "P1.3", "P1.4", "P1.5", "P1.6", "P1.7",
/* 98 */
	"RI", "TI", "RB8", "TB8", "REN", "SM2", "SM1", "SM0",
/* A0 */
	"P2.0", "P2.1", "P2.2", "P2.3", "P2.4", "P2.5", "P2.6", "P2.7",
/* A8 */
	"EX0", "ET0", "EX1", "ET1", "ES", "0ADh", "0AEh", "EA",
/* B0 */
	"P3.0", "P3.1", "P3.2", "P3.3", "P3.4", "P3.5", "P3.6", "P3.7",
/* B8 */
	"PX0", "PT0", "PX1", "PT1", "PS", "0BDh", "0BEh", "0BFh",
/* C0 */
	"0C0h", "0C1h", "0C2h", "0C3h", "0C4h", "0C5h", "0C6h", "0C7h",
/* C8 */
	"0C8h", "0C9h", "0CAh", "0CBh", "0CCh", "0CDh", "0CEh", "0CFh",
/* D0 */
	"P", "0D1h", "OV", "RS0", "RS1", "F0", "AC", "CY",
/* D8 */
	"0D8h", "0D9h", "0DAh", "0DBh", "0DCh", "0DDh", "0DEh", "0DFh",
/* E0 */
	"ACC.0", "ACC.1", "ACC.2", "ACC.3", "ACC.4", "ACC.5", "ACC.6", "ACC.7",
/* E8 */
	"0E8h", "0E9h", "0EAh", "0EBh", "0ECh", "0EDh", "0EEh", "0EFh",
/* F0 */
	"B.0", "B.1", "B.2", "B.3", "B.4", "B.5", "B.6", "B.7",
/* F8 */
	"0F8h", "0F9h", "0FAh", "0FBh", "0FCh", "0FDh", "0FEh", "0FFh"
};

const char sfrname[128][5] = {
/* 80 */
	"P0", "SP", "DPL", "DPH", "84h", "85h", "86h", "PCON",
/* 88 */
	"TCON", "TMOD", "TL0", "TL1", "TH0", "TH1", "8Eh", "8Fh",
/* 90 */
	"P1", "91h", "92h", "93h", "94h", "95h", "96h", "97h",
/* 98 */
	"SCON", "SBUF", "9Ah", "9Bh", "9Ch", "9Dh", "9Eh", "9Fh",
/* A0 */
	"P2", "0A1h", "0A2h", "0A3h", "0A4h", "0A5h", "0A6h", "0A7h",
/* A8 */
	"IE", "0A9h", "0AAh", "0ABh", "0ACh", "0ADh", "0AEh", "0AFh",
/* B0 */
	"P3", "0B1h", "0B2h", "0B3h", "0B4h", "0B5h", "0B6h", "0B7h",
/* B8 */
	"IP", "0B9h", "0BAh", "0BBh", "0BCh", "0BDh", "0BEh", "0BFh",
/* C0 */
	"0C0h", "0C1h", "0C2h", "0C3h", "0C4h", "0C5h", "0C6h", "0C7h",
/* C8 */
	"0C8h", "0C9h", "0CAh", "0CBh", "0CCh", "0CDh", "0CEh", "0CFh",
/* D0 */
	"PSW", "0D1h", "0D2h", "0D3h", "0D4h", "0D5h", "0D6h", "0D7h",
/* D8 */
	"0D8h", "0D9h", "0DAh", "0DBh", "0DCh", "0DDh", "0DEh", "0DFh",
/* E0 */
	"ACC", "0E1h", "0E2h", "0E3h", "0E4h", "0E5h", "0E6h", "0E7h",
/* E8 */
	"0E8h", "0E9h", "0EAh", "0EBh", "0ECh", "0EDh", "0EEh", "0EFh",
/* F0 */
	"B", "0F1h", "0F2h", "0F3h", "0F4h", "0F5h", "0F6h", "0F7h",
/* F8 */
	"0F8h", "0F9h", "0FAh", "0FBh", "0FCh", "0FDh", "0FEh", "0FFh"
};
