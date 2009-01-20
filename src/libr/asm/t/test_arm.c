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
	u8 *buf = "\x7c\x20\xa0\xe3";
	u32 idx = 0, ret = 0, len = 4;

	r_asm_init(&a);
	r_asm_set_arch(&a, R_ASM_ARCH_ARM);
	r_asm_set_pc(&a, 0x000089d8);
	while (idx < len) {
		r_asm_set_pc(&a, a.pc + ret);
		ret = r_asm_disasm(&a, buf+idx, len-idx);
		idx += ret;
		printf("DISASM %s HEX %s\n", a.buf_asm, a.buf_hex);
	}
}
