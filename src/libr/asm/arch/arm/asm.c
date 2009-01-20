/* radare - LGPL - Copyright 2009 nibble<.ds@gmail.com> */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <r_types.h>
#include <r_asm.h>

#include "gnu/dis-asm.h"

int arm_mode;
static unsigned long Offset = 0;
static unsigned char bytes[4];

static int arm_buffer_read_memory (bfd_vma memaddr, bfd_byte *myaddr, unsigned int length, struct disassemble_info *info)
{
	memcpy (myaddr, bytes, length);
	return 0;
}

static int symbol_at_address(bfd_vma addr, struct disassemble_info * info)
{
	return 0;
}

static int memory_error_func(int a, int b, int c)
{
	return 0;
}

static char *buf_global = NULL;
static void print_address(bfd_vma address, struct disassemble_info *info)
{
	char tmp[32];
	if (buf_global == NULL)
		return;
	sprintf(tmp, "0x%08llx", (u64)address);
	strcat(buf_global, tmp);
}

static void buf_fprintf(FILE *stream, const char *format, ...)
{
	va_list ap;
	char *tmp;
	if (buf_global == NULL)
		return;
	va_start(ap, format);
 	tmp = alloca(strlen(format)+strlen(buf_global)+2);
	sprintf(tmp, "%s%s", buf_global, format);
	vsprintf(buf_global, tmp, ap);
	va_end(ap);
}


u32 r_asm_arm_disasm(struct r_asm_t *a, u8 *buf, u32 len)
{
	struct disassemble_info disasm_obj;

	buf_global = a->buf_asm;
	Offset = a->pc;
	memcpy(bytes, buf, 4); // TODO handle thumb
	snprintf(a->buf_hex, 255, "%x %x %x %x", buf[0], buf[1], buf[2], buf[3]);

	/* prepare disassembler */
	memset(&disasm_obj,'\0', sizeof(struct disassemble_info));
	arm_mode = a->bits;
	//info.arch = ARM_EXT_V1|ARM_EXT_V4T|ARM_EXT_V5;
	disasm_obj.buffer = bytes;
	disasm_obj.read_memory_func = &arm_buffer_read_memory;
	disasm_obj.symbol_at_address_func = &symbol_at_address;
	disasm_obj.memory_error_func = &memory_error_func;
	disasm_obj.print_address_func = &print_address;
	disasm_obj.endian = !a->big_endian;
	disasm_obj.fprintf_func = &buf_fprintf;
	disasm_obj.stream = stdout;

	if (print_insn_arm((unsigned long)0, &disasm_obj) == -1) {
		strcpy(a->buf_asm, " (data)");
		return -1;
	}

	return 4;
#if 0 
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
#endif 
}
