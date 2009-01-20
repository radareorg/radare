/* radare - LGPL - Copyright 2007-2009 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <r_types.h>
#include <r_asm.h>

#include "udis86/types.h"
#include "udis86/extern.h"
#include "ollyasm/disasm.h"


u32 r_asm_x86_disasm(struct r_asm_t *a, u8 *buf, u32 len)
{
	union {
		ud_t     ud;
		t_disasm olly;
	} disasm_obj;
	u32 ret = -1;

	switch (a->syntax) {
	case R_ASM_SYN_INTEL:
	case R_ASM_SYN_ATT:
	case R_ASM_SYN_PSEUDO:
		ud_init(&disasm_obj.ud);
		if (a->syntax == R_ASM_SYN_INTEL)
			ud_set_syntax(&disasm_obj.ud, UD_SYN_INTEL);
		else if (a->syntax == R_ASM_SYN_ATT)
			ud_set_syntax(&disasm_obj.ud, UD_SYN_ATT);
		else ud_set_syntax(&disasm_obj.ud, UD_SYN_PSEUDO);
		ud_set_mode(&disasm_obj.ud, a->bits);
		ud_set_pc(&disasm_obj.ud, a->pc);
		ud_set_input_buffer(&disasm_obj.ud, buf, len);
		ud_disassemble(&disasm_obj.ud);
		snprintf(a->buf_asm, 255, "%s", ud_insn_asm(&disasm_obj.ud));
		snprintf(a->buf_hex, 255, "%s", ud_insn_hex(&disasm_obj.ud));
		ret = ud_insn_len(&disasm_obj.ud);
		break;
	case R_ASM_SYN_OLLY:
		lowercase=1;
		ret = Disasm(buf, len, a->pc, &disasm_obj.olly, DISASM_FILE);
		snprintf(a->buf_asm, 255, "%s", disasm_obj.olly.result);
		snprintf(a->buf_hex, 255, "%s", disasm_obj.olly.dump);
		break;
	default:
		ret = -1;
	}

	return ret;
}
