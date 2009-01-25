/* radare - LGPL - Copyright 2009 nibble<.ds@gmail.com> */

#include <stdio.h>
#include <stdlib.h>

#include <r_types.h>
#include <r_util.h>
#include <r_asm.h>

int r_asm_init(struct r_asm_t *a)
{
	memset(a, '\0', sizeof(struct r_asm_t));
	r_asm_set_arch(a, R_ASM_ARCH_X86);
	r_asm_set_bits(a, 32);
	r_asm_set_big_endian(a, 0);
	r_asm_set_syntax(a, R_ASM_SYN_INTEL);
	r_asm_set_parser(a, R_ASM_PAR_NULL, NULL);
	r_asm_set_pc(a, 0);
	return 1;
}

struct r_asm_t *r_asm_new()
{
	struct r_asm_t *a = MALLOC_STRUCT(struct r_asm_t);
	r_asm_init(a);
	return a;
}

void r_asm_free(struct r_asm_t *a)
{
	free(a);
}

int r_asm_set_arch(struct r_asm_t *a, u32 arch)
{
	switch (arch) {
	case R_ASM_ARCH_X86:
		a->r_asm_disasm = &r_asm_x86_disasm;
		a->r_asm_asm = &r_asm_x86_asm;
		break;
	case R_ASM_ARCH_ARM:
		a->r_asm_disasm = &r_asm_arm_disasm;
		a->r_asm_asm = NULL;
		break;
	default:
		a->r_asm_disasm = NULL;
		a->r_asm_asm = NULL;
		return -1;
	}
	a->arch = arch;
	return 1;
}

int r_asm_set_bits(struct r_asm_t *a, u32 bits)
{
	switch (bits) {
	case 16:
	case 32:
	case 64:
		a->bits = bits;
		return 1;
	default:
		return -1;
	}
}

int r_asm_set_big_endian(struct r_asm_t *a, u32 boolean)
{
	a->big_endian = boolean;
	return 1;
}

int r_asm_set_syntax(struct r_asm_t *a, u32 syntax)
{
	switch (syntax) {
	case R_ASM_SYN_NULL:
	case R_ASM_SYN_INTEL:
	case R_ASM_SYN_ATT:
	case R_ASM_SYN_OLLY:
		a->syntax = syntax;
		return 1;
	default:
		return -1;
	}
}

int r_asm_set_parser(struct r_asm_t *a, u32 parser, void *aux)
{
	switch (parser) {
	case R_ASM_PAR_PSEUDO:
		if (a->arch == R_ASM_ARCH_X86 && a->syntax == R_ASM_SYN_INTEL) {
			a->r_asm_parser = &r_asm_x86_pseudo;
		} else {
			a->r_asm_parser = NULL;
		}
		break;
	default:
		a->r_asm_parser = NULL;
		return -1;
	}
	a->parser = parser;
	a->aux = aux;
	return 1;
}

int r_asm_set_pc(struct r_asm_t *a, u64 pc)
{
	a->pc = pc;
	return 1;
}

u32 r_asm_disasm(struct r_asm_t *a, u8 *buf, u32 len)
{
	u32 ret = 0;

	if (a->r_asm_disasm != NULL)
		ret = a->r_asm_disasm(a, buf, len);
	else return 0;

	if (a->r_asm_parser != NULL)
		a->r_asm_parser(a, a->aux, a->buf_asm);
	else strncpy(a->buf_par, a->buf_asm, 256);

	return ret;
}

u32 r_asm_asm(struct r_asm_t *a, char *buf)
{
	if (a->r_asm_asm != NULL)
		return a->r_asm_asm(a, buf);
	else return 0;
}
