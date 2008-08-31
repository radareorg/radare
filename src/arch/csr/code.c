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

#include "dis.h"
#include "../../main.h"
#include "../../code.h"
#include <stdio.h>
#include <string.h>

static int get_num(int num, int shift)
{
	int tmp;
	char x;

	x = (char) ((num >> shift) & 0xff);
	tmp = x;
	tmp <<= shift;

	return tmp;
}

static int get_operand(struct state *s, struct directive *d)
{
	int total = 0;

	total += get_num(d->d_inst.in_operand, 0);

	if (s->s_prefix)
		total += get_num(s->s_prefix_val, 8);

	if (s->s_prefix == 2)
		total += get_num(s->s_prefix_val, 16);

	return total;
}

static int label_off(struct directive *d)
{
#if 1
//	int off = get_operand(d);
	int off = d->d_operand;

	int lame = off & 0x80;

	/* XXX WTF? */
	if (!d->d_prefix) {
		off = (char) (off &0xff);
	} else if (d->d_prefix == 1) {
		off = (short) (off & 0xffff);

		if (lame)
			off -= 0x100;

	} else {
		off = (int) (off & 0xffffff);

		if (off & 0x800000)
			off |= 0xff000000;

		if (off & 0x8000)
			off -= 0x10000;

		if (lame)
			off -= 0x100;
	}
#endif
//	int off = d->d_operand;

	return d->d_off + off;
}

static uint16_t i2u16(struct instruction *in)
{
	return *((uint16_t*)in);
}
// NOTE: bytes should be at least 16 bytes?
int arch_csr_aop(u64 addr, const unsigned char *bytes, struct aop_t *aop)
{
	unsigned int i;
	int sz = 1;
	uint16_t lol;
	uint16_t ins;
	struct directive d;
	int rel = 0;
	struct instruction *in = bytes;
	struct state *s = &_state;

	if (aop == NULL)
		return 2;

	memcpy(&ins, bytes, sizeof(uint16_t));
	s->s_buf = bytes;
	s->s_off = addr;
	s->s_out = -1;
	memset(&d, '\0', sizeof(struct directive));
	memcpy(&d.d_inst, s->s_buf, sizeof(d.d_inst));
	d.d_off = (s->s_off+=2);
	csr_decode(s, &d);
	d.d_operand = get_operand(s,&d );

	memset(aop, '\0', sizeof(struct aop_t));
	aop->type = AOP_TYPE_UNK;
	memcpy(&ins, bytes, sizeof(uint16_t));
	memcpy(&lol, bytes, sizeof(uint16_t));
	aop->length = 2;

	switch(i2u16(in)) {
	case INST_NOP:
		aop->type = AOP_TYPE_NOP;
		break;
	case INST_BRK:
		aop->type = AOP_TYPE_TRAP;
		break;
	case INST_BC:
		aop->type = AOP_TYPE_TRAP;
		break;
	case INST_BRXL:
		aop->type = AOP_TYPE_TRAP;
		break;
	default:
		switch(in->in_opcode) {
		case 0:
			switch(lol&0xf) {
				case 1:
				case 2:
				case 3:
				case 0xa:
					aop->type = AOP_TYPE_PUSH;
					break;
				case 4:
				case 5:
				case 6:
				case 7:
				case 0xe:
					aop->type = AOP_TYPE_POP;
					break;
			}
		case 1:
			aop->type = AOP_TYPE_POP;
			break;
		case 2:
			aop->type = AOP_TYPE_PUSH;
			break;
		case 3:
		case 4:
		case 7:
			aop->type = AOP_TYPE_ADD;
			break;
		case 5:
		case 6:
			aop->type = AOP_TYPE_SUB;
			break;
		case 8:
			aop->type = AOP_TYPE_CMP;
			break;
		case 9:
			switch(in->in_reg) {
			case 0:
				aop->type = AOP_TYPE_MUL;
				break;
			case 1:
				aop->type = AOP_TYPE_DIV;
				break;
			case 2:
				aop->type = AOP_TYPE_CMP;
				break;
			case 3:
				// BSR
				aop->type = AOP_TYPE_CALL;
				if (in->in_mode == ADDR_MODE_RELATIVE)
					rel = 1;
				aop->jump = label_off(&d);
				rel = 0;
				if (aop->jump&1)aop->jump+=3;
				aop->fail = addr+2;
				aop->eob = 1;
				break;
				
			}
			break;
		case 0xb:
			aop->type = AOP_TYPE_OR;
			break;
		case 0xc:
			aop->type = AOP_TYPE_AND;
			break;
		case 0xd:
			aop->type = AOP_TYPE_XOR;
			break;
		case 0xe:
			if (in->in_mode == ADDR_MODE_RELATIVE)
				rel = 1;
			switch(in->in_reg) {
			case 0: // BRA
				aop->type = AOP_TYPE_JMP;
				aop->jump = label_off(&d)+4;
				if (aop->jump&1)aop->jump+=3;
				aop->eob = 1;
				break;
			case 1:
				// BLT
				aop->type = AOP_TYPE_CJMP;
				aop->jump = label_off(&d);
				if (aop->jump&1)aop->jump+=3;
				aop->fail = addr + 2;
				aop->eob = 1;
				break;
			case 2:
				// BPL
				aop->type = AOP_TYPE_CJMP;
				aop->jump = label_off(&d);
				if (aop->jump&1)aop->jump+=3;
				aop->fail = addr + 2;
				aop->eob = 1;
				break;
			case 3:
				// BMI
				aop->type = AOP_TYPE_CJMP;
				aop->jump = label_off(&d);
				if (aop->jump&1)aop->jump+=3;
				aop->fail = addr + 2;
				aop->eob = 1;
				break;
			}
			break;
		case 0xf:
			switch(in->in_reg) {
			case 0: // BNE
			case 1: // BEQ
			case 2: // BCC
			case 3: // BCS
				aop->type = AOP_TYPE_CJMP;
				rel = 0;
				aop->jump = label_off(&d);
if (aop->jump&1)aop->jump+=3;
				aop->fail = addr+2;
				break;
			}
		}
	}
	/* get opcode size */

	return aop->length;
}
