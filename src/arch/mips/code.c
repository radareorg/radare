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

#if 0

          0x00401F3C            8f998268        lw      t9,-32152(gp)
          0x00401F40,           00000000        nop
          0x00401F44            0320f809        jalr    t9


Instruction	Opcode/Function	Syntax	Operation
add 	100000	ArithLog 	$d = $s + $t
addu 	100001	ArithLog 	$d = $s + $t
addi 	001000	ArithLogI	$t = $s + SE(i)
addiu 	001001	ArithLogI	$t = $s + SE(i)
and 	100100	ArithLog 	$d = $s & $t
andi 	001100	ArithLogI	$t = $s & ZE(i)
div 	011010	DivMult 	lo = $s / $t; hi = $s % $t
divu 	011011	DivMult 	lo = $s / $t; hi = $s % $t
mult 	011000	DivMult 	hi:lo = $s * $t
multu 	011001	DivMult 	hi:lo = $s * $t
nor 	100111	ArithLog 	$d = ~($s | $t)
or 	100101	ArithLog 	$d = $s | $t
ori 	001101	ArithLogI	$t = $s | ZE(i)
sll 	000000	Shift 	$d = $t << a
sllv 	000100	ShiftV 	$d = $t << $s
sra 	000011	Shift 	$d = $t >> a
srav 	000111	ShiftV 	$d = $t >> $s
srl 	000010	Shift 	$d = $t >>> a
srlv 	000110	ShiftV 	$d = $t >>> $s
sub 	100010	ArithLog 	$d = $s - $t
subu 	100011	ArithLog 	$d = $s - $t
xor 	100110	ArithLog 	$d = $s ^ $t
xori 	001110	ArithLogI	$d = $s ^ ZE(i)
Constant-Manipulating Instructions
Instruction	Opcode/Function	Syntax	Operation
lhi 	011001	LoadI	HH ($t) = i
llo 	011000	LoadI	LH ($t) = i
Comparison Instructions
Instruction	Opcode/Function	Syntax	Operation
slt 	101010	ArithLog 	$d = ($s < $t)
sltu 	101001	ArithLog 	$d = ($s < $t)
slti 	001010	ArithLogI	$t = ($s < SE(i))
sltiu 	001001	ArithLogI	$t = ($s < SE(i))
Branch Instructions
Instruction	Opcode/Function	Syntax	Operation
beq 	000100	Branch 	if ($s == $t) pc += i << 2
bgtz 	000111	BranchZ 	if ($s > 0) pc += i << 2
blez 	000110	BranchZ 	if ($s <= 0) pc += i << 2
bne 	000101	Branch 	if ($s != $t) pc += i << 2
Jump Instructions
Instruction	Opcode/Function	Syntax	Operation
j 	000010	Jump 	pc += i << 2
jal 	000011	Jump 	$31 = pc; pc += i << 2
jalr 	001001	JumpR 	$31 = pc; pc = $s
jr 	001000	JumpR 	pc = $s
Load Instructions
Instruction	Opcode/Function	Syntax	Operation
lb 	100000	LoadStore	$t = SE (MEM [$s + i]:1)
lbu 	100100	LoadStore	$t = ZE (MEM [$s + i]:1)
lh 	100001	LoadStore	$t = SE (MEM [$s + i]:2)
lhu 	100101	LoadStore	$t = ZE (MEM [$s + i]:2)
lw 	100011	LoadStore	$t = MEM [$s + i]:4
Store Instructions
Instruction	Opcode/Function	Syntax	Operation
sb 	101000	LoadStore	MEM [$s + i]:1 = LB ($t)
sh 	101001	LoadStore	MEM [$s + i]:2 = LH ($t)
sw 	101011	LoadStore	MEM [$s + i]:4 = $t
Data Movement Instructions
Instruction	Opcode/Function	Syntax	Operation
mfhi 	010000	MoveFrom 	$d = hi
mflo 	010010	MoveFrom 	$d = lo
mthi 	010001	MoveTo 	hi = $s
mtlo 	010011	MoveTo 	lo = $s
Exception and Interrupt Instructions
Instruction	Opcode/Function	Syntax	Operation
trap 	011010	Trap
#endif

// NOTE: bytes should be at least 16 bytes?
int arch_mips_aop(u64 addr, const unsigned char *bytes, struct aop_t *aop)
{
	unsigned long op;
	int reg; 
	short r;
	char buf[10];

	if (aop == NULL)
		return (mips_mode==16)?2:4;

	endian_memcpy_e(&op, bytes, 4, config_get("cfg.bigendian"));
	memset(aop, '\0', sizeof(struct aop_t));
	//of &=0x3f;

	//memcpy(&op, bytes, 4);
	endian_memcpy_e(&op, bytes, 4, config.endian);
	aop->type = AOP_TYPE_UNK;

	//eprintf("\n--%08llx : %d\n", addr, op&0x3f);
	//switch((op>>24) & 0x3f) {
	switch(op & 0x3f) {
	// J-Type
	case 2: // j
		break;
		// branch to register
		//XXX TODO
		//eprintf("UJUMP\n");
		//aop->type = AOP_TYPE_UJMP;
		break;
		aop->type = AOP_TYPE_CJMP;
		break;
	// R-Type
	case 1: // bltz
		// 04100001        bltzal        zero,0x2aaa8cb4
	case 4: // beq // bal
	case 5: // bne
	case 6: // blez
	case 7: // bgtz
	case 16: //beqz
	case 20: //bnel
		aop->type = AOP_TYPE_CJMP;
		reg = (((op&0x00ff0000)>>16) + ((op&0xff000000)>>24));
		aop->jump = addr+(reg<<2) + 4;
		aop->fail = addr+8;
		// calculate jump
		break;
	case 3: // jalr
	//case 9: // jalr
		reg = op>>24;
		if (reg< 10) {
			aop->type = AOP_TYPE_CALL;
			sprintf(buf, "t%d", reg); // XXX must be rN...!regs* should be synced here
			aop->jump = flag_get_addr(buf);
			aop->fail = addr+8;
		}
		break;
	case 8: // jr
		aop->type = AOP_TYPE_RET;
		break;
	case 12:
		aop->type = AOP_TYPE_SWI;
		break;
	case 13:
		aop->type = AOP_TYPE_TRAP;
		break;
	default:
		switch(op) {
		case 32: // add
		case 33: // addu
			aop->type = AOP_TYPE_ADD;
			break;
		case 34: // sub
		case 35: // subu
			aop->type = AOP_TYPE_SUB;
			break;
		case 0x03e00008:
		case 0x0800e003: // jr ra
			aop->type = AOP_TYPE_RET;
			break;
		case 0x0000000d: // case 26:
		case 0x0d000000: // break
			aop->type = AOP_TYPE_TRAP; 
			break;
		case 0:
			aop->type = AOP_TYPE_NOP;
			break;
		default:
			//switch((op<<24)&0xff) { //bytes[3]) { // TODO handle endian ?
			switch((bytes[3])) {
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
