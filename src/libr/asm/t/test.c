/* radare - LGPL - Copyright 2007-2009 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <r_types.h>
#include <r_asm.h>

int main()
{
	struct r_asm_t a;
	u8 *buf = "\x90\x83\xe4\xf0\x90\x90";
	u32 idx = 0, len = 4;

	r_asm_init(&a);
	while (idx < len) {
		idx += r_asm_disasm_buf(&a, buf+idx, len-idx);
		printf("DISASM %s HEX %s\n", a.buf_asm, a.buf_hex);
	}
}
