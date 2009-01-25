/* radare - LGPL - Copyright 2009 nibble<.ds@gmail.com> */

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
	//u8 *buf = "\x74\x31\x74\x31\x74\x31";
	u8 *buf = "\xc7\xc0\x04\x00\x00\x00";
	u32 idx = 0, ret = 0, len = 6;

	r_asm_init(&a);
	r_asm_set_syntax(&a, R_ASM_SYN_INTEL);
	//r_asm_set_syntax(&a, R_ASM_SYN_ATT);
	//r_asm_set_syntax(&a, R_ASM_SYN_OLLY);
	r_asm_set_parser(&a, R_ASM_PAR_PSEUDO, NULL);
	//r_asm_set_parser(&a, R_ASM_PAR_NULL, NULL);
	r_asm_set_pc(&a, 0x8048000);
	while (idx < len) {
		r_asm_set_pc(&a, a.pc + ret);
		ret = r_asm_disasm(&a, buf+idx, len-idx);
		idx += ret;
		printf("DISASM %s HEX %s PAR %s\n", a.buf_asm, a.buf_hex, a.buf_par);
	}
}
