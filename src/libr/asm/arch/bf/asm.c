/* radare - LGPL - Copyright 2009 pancake <youterm.com> - nibble<.ds@gmail.com> */

#include <stdio.h>
#include <string.h>

#include <r_types.h>
#include <r_asm.h>


u32 r_asm_bf_disasm(struct r_asm_t *a, u8 *buf, u32 len)
{
	int i;
	char *buf_cp, *b;

	buf_cp = strdup((char*)buf);
	b = buf_cp;

	for(i=0;b[0] == b[1] && i<len; b=b+1,i++); b[1] = '\0';

	strncpy(a->buf_hex, buf_cp, 256);

	switch(buf[0]) {
	case '[':
		strcpy(a->buf_asm, "[ loop {");
		break;
	case ']':
		strcpy(a->buf_asm, "] }"); // TODO: detect clause and put label name
		break;
	case '>':
		if (i>1) strcpy(a->buf_asm, "> add [ptr]");
		else strcpy(a->buf_asm, "> inc [ptr]");
		break;
	case '<':
		if (i>1) strcpy(a->buf_asm, "< sub [ptr]");
		else strcpy(a->buf_asm, "< dec [ptr]");
		break;
	case '+':
		if (i>1) strcpy(a->buf_asm, "+ add [ptr]");
		else strcpy(a->buf_asm, "+ inc [ptr]");
		break;
	case '-':
		if (i>1) strcpy(a->buf_asm, "- sub [ptr]");
		else strcpy(a->buf_asm, "- dec [ptr]");
		break;
	case ',':
		strcpy(a->buf_asm, ", [ptr] = getch()");
		break;
	case '.':
		strcpy(a->buf_asm, ". print( [ptr] )");
		break;
	case '\x00':
		strcpy(a->buf_asm, "  trap");
		break;
	default:
		strcpy(a->buf_asm, "  nop");
		break;
	}

	if (i>0) sprintf(a->buf_asm, "%s, %d", a->buf_asm, i+1);
	if (i<1) i=1; else i++;

	free(buf_cp);

	return i;
}
