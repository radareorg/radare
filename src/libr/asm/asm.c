/* radare - LGPL - Copyright 2007-2009 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <r_types.h>
#include <r_asm.h>

int r_asm_init(struct r_asm_t *a)
{
	r_asm_set_arch(a, R_ASM_ARCH_X86);
	r_asm_set_mode(a, R_ASM_MODE_32);
	r_asm_set_endianess(a, R_ASM_LIL_ENDIAN);
	r_asm_set_syntax(a, R_ASM_SYN_INTEL);
	r_asm_set_pc(a, 0);
	return 1;
}

int r_asm_set_arch(struct r_asm_t *a, u32 arch)
{
	switch (arch) {
	case R_ASM_ARCH_X86:
		a->r_asm_disasm_buf = &r_asm_x86_disasm_buf;
		break;
	case R_ASM_ARCH_ARM:
	case R_ASM_ARCH_PPC:
	case R_ASM_ARCH_M68K:
	case R_ASM_ARCH_JAVA:
	case R_ASM_ARCH_MIPS:
	case R_ASM_ARCH_SPARC:
	case R_ASM_ARCH_CSR:
	case R_ASM_ARCH_MSIL:
	case R_ASM_ARCH_OBJD:
	case R_ASM_ARCH_BF:
		return -1;
	default:
		return -1;
	}
	a->arch = arch;
	return 1;
}

int r_asm_set_mode(struct r_asm_t *a, u32 mode)
{
	switch (mode) {
	case R_ASM_MODE_16:
	case R_ASM_MODE_32:
	case R_ASM_MODE_64:
		a->mode = mode;
		return 1;
	default:
		return -1;
	}
}

int r_asm_set_endianess(struct r_asm_t *a, u32 endianess)
{
	switch (endianess) {
	case R_ASM_BIG_ENDIAN:
	case R_ASM_LIL_ENDIAN:
		a->endianess = endianess;
		return 1;
	default:
		return -1;
	}
}

int r_asm_set_syntax(struct r_asm_t *a, u32 syntax)
{
	switch (syntax) {
	case R_ASM_SYN_NULL:
	case R_ASM_SYN_INTEL:
	case R_ASM_SYN_ATT:
	case R_ASM_SYN_OLLY:
	case R_ASM_SYN_PSEUDO:
		a->syntax = syntax;
		return 1;
	default:
		return -1;
	}
}

int r_asm_set_pc(struct r_asm_t *a, u64 pc)
{
	a->pc = pc;
	return 1;
}

u32 r_asm_disasm_buf(struct r_asm_t *a, u8 *buf, u32 len)
{
	if (a->r_asm_disasm_buf != NULL)
		return a->r_asm_disasm_buf(a, buf, len);
	else return -1;
}
