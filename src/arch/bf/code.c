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

#include "../../code.h"
#include <stdio.h>
#include <string.h>

ut64 last_loop = 0;

// NOTE: bytes should be at least 16 bytes?
int arch_bf_aop(ut64 addr, const u8 *buf, struct aop_t *aop)
{
	int len = 256; /* XXX fix limit bytes here */
	int i;
	memset(aop, '\0', sizeof(struct aop_t));
	aop->type = AOP_TYPE_UNK;

	/* opcode type */
	switch(buf[0]) {
	case '[':
		aop->type = AOP_TYPE_TRAP;
		last_loop = addr;
		break;
	case ']':
		aop->type = AOP_TYPE_CJMP;
		aop->jump = last_loop;
		aop->fail = addr+1;
		aop->eob = 1;
		break;
	case '>':
		//aop->type = AOP_TYPE_ADD;
		break;
	case '<':
		//aop->type = AOP_TYPE_SUB;
		break;
	case '+':
		//aop->type = AOP_TYPE_ADD;
		break;
	case '-':
		//aop->type = AOP_TYPE_SUB;
		break;
	case '.':
		aop->type = AOP_TYPE_TRAP;
		break;
	case ',':
		aop->type = AOP_TYPE_TRAP;
		break;
	case '\x00':
		aop->type = AOP_TYPE_TRAP;
		break;
	default:
		aop->type = AOP_TYPE_NOP;
		break;
	}

	/* calculate opcode length */
	for(i=0;buf[0] == buf[1] && i<len; buf=buf+1,i++);
	if (i<1) i=1; else i++;
	aop->length = i;

	return aop->length;
}

int arch_bf_dis(const u8* buf, ut64 addr, int len)
{
	int i;
	const u8 *b = buf;

	for(i=0;b[0] == b[1] && i<len; b=b+1,i++);

	switch(buf[0]) {
	case '[':
		cons_printf("[ loop {");
		break;
	case ']':
		cons_printf("] }"); // TODO: detect clause and put label name
		break;
	case '>':
		if (i>1) cons_printf("> add [ptr]");
		else cons_printf("> inc [ptr]");
		break;
	case '<':
		if (i>1) cons_printf("< sub [ptr]");
		else cons_printf("< dec [ptr]");
		break;
	case '+':
		if (i>1) cons_printf("+ add [ptr]");
		else cons_printf("+ inc [ptr]");
		break;
	case '-':
		if (i>1) cons_printf("- sub [ptr]");
		else cons_printf("- dec [ptr]");
		break;
	case ',':
		cons_printf(", [ptr] = getch()");
		break;
	case '.':
		cons_printf(". print( [ptr] )");
		break;
	case '\x00':
		cons_printf("  trap");
		break;
	default:
		cons_printf("  nop");
		break;
	}

	if (i>0) cons_printf(", %d", i+1);
	if (i<1) i=1; else i++;

	return i;
}
