/*
 * Copyright (C) 2007
 *       esteve <youterm.com>
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
#include <stdio.h>
#include <string.h>

// XXX ultra guarro
extern struct java_op {
	char *name;
	unsigned char byte;
	int size;
} java_ops[];

// NOTE: bytes should be at least 16 bytes?
int arch_java_aop(u64 addr, const unsigned char *bytes, struct aop_t *aop)
{
	unsigned int i;
	int sz = 1;

	/* get opcode size */
	for(i = 0;java_ops[i].name != NULL;i++)
		if (bytes[0] == java_ops[i].byte)
			sz = java_ops[i].size;

	if (aop == NULL)
		return sz;

	memset(aop, '\0', sizeof(struct aop_t));
	aop->length = sz;

	switch(bytes[0]) {
	case 0xa9: // ret
	case 0xb1: // return
	case 0xb0: // areturn
	case 0xaf: // dreturn
	case 0xae: // freturn
	case 0xac: // ireturn
	case 0xad: // lreturn
		aop->type = AOP_TYPE_RET;
		aop->eob  = 1;
		break;
	case 0xa7: // goto
	case 0xc8: // goto_w
		aop->type = AOP_TYPE_JMP;
		aop->jump = 0x0; // TODO
		aop->eob  = 1;
		break;
	case 0xa5: // acmpeq
	case 0xa6: // acmpne
	case 0x9f: // icmpeq
	case 0xa0: // icmpne
	case 0xa1: // icmplt
	case 0xa2: // icmpge
	case 0xa3: // icmpgt
	case 0xa4: // icmple
	case 0x99: // ifeq
	case 0x9a: // ifne
	case 0x9b: // iflt
	case 0x9c: // ifge
	case 0x9d: // ifgt
	case 0x9e: // ifle
	case 0xc7: // ifnonnull
	case 0xc6: // ifnull
		aop->type = AOP_TYPE_CJMP;
		aop->jump = 0x0; // TODO
		aop->fail = addr + sz;
		aop->eob = 1;
		break;
	case 0xa8: // jsr
	case 0xc9: // jsr_w
		aop->type = AOP_TYPE_CALL;
		aop->jump = 0x0; // TODO
		aop->fail = addr + sz;
		aop->eob = 1;
		break;
	case 0xb9: // invokeinterface
	case 0xb7: // invokespecial
	case 0xb8: // invokestatic
	case 0xb6: // invokevirtual
	case 0xbb: // new
	case 0xbc: // newarray
	case 0xc5: // multi new array
		aop->type = AOP_TYPE_SWI;
		break;
	case 0xca: // breakpoint
		aop->type = AOP_TYPE_TRAP;
		break;
	case 0xbf: // athrow
		aop->type = AOP_TYPE_TRAP;
		break;
	case 0x00: // nop
		aop->type = AOP_TYPE_NOP;
		break;
	case 0xba:
		aop->type = AOP_TYPE_ILL;
		break;
	case 0x57: // pop
	case 0x58: // pop2
		aop->type = AOP_TYPE_POP;
		break;
	case 0x10: // bipush
	case 0x11: // sipush
	case 0x59: // dup
	case 0x5a: // dup_x1
	case 0x5b: // dup_x2
	case 0x5c: // dup2
	case 0x5d: // dup2_x1
	case 0x5e: // dup2_x2
		aop->type = AOP_TYPE_PUSH;
		break;
	case 0x60: // iadd
	case 0x61: // ladd
	case 0x62: // fadd
	case 0x63: // dadd
		aop->type = AOP_TYPE_ADD;
		break;
	case 0x64: // isub
	case 0x65: // lsub
	case 0x66: // fsub
	case 0x67: // dsub
		aop->type = AOP_TYPE_SUB;
		break;
	case 0x76: // neg
		aop->type = AOP_TYPE_NOT;
		break;
	case 0x78: //ishl
	case 0x79: //lshl
		aop->type = AOP_TYPE_SHL;
		break;
	case 0x7a: //ishr
	case 0x7b: //lshr
		aop->type = AOP_TYPE_SHR;
		break;
	case 0x80: // ior
	case 0x81: // lor
		aop->type = AOP_TYPE_OR;
		break;
	case 0x82: // ixor
	case 0x83: // lxor
		aop->type = AOP_TYPE_XOR;
		break;
	case 0x7e: // iand
	case 0x7f: // land
		aop->type = AOP_TYPE_AND;
		break;
	case 0x68: // imul
	case 0x69: // lmul
	case 0x6a: // fmul
	case 0x6b: // dmul
		aop->type = AOP_TYPE_MUL;
		break;
	case 0x6c: // idiv
	case 0x6d: // ldiv
	case 0x6e: // fdiv
	case 0x6f: // ddiv
		aop->type = AOP_TYPE_DIV;
		break;
	}

	return sz;
}
