/* radare - LGPL - Copyright 2007-2009 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "r_types.h"
#include "r_asm.h"

//#include "arch/x86/udis86/types.h"
//#include "arch/x86/udis86/extern.h"
//#include "arch/x86/ollyasm/disasm.h"


//static ud_t ud_obj;


int r_asm_init(r_asm_t *a)
{
	a->arch = R_ASM_ARCH_X86;
	a->mode = R_ASM_MODE_32;
	a->endianess = R_ASM_LIL_ENDIAN;
	a->syntax = R_ASM_SYN_INTEL;
	a->pc = 0;
	return 1;
}

int r_asm_set_arch(r_asm_t *a, int arch)
{
	switch (arch) {
	case R_ASM_ARCH_X86:
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
		a->arch = arch;
		return 1;
	default:
		return -1;
	}
}

int r_asm_set_mode(r_asm_t *a, int mode)
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

int r_asm_set_endianess(r_asm_t *a, int endianess)
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

int r_asm_set_syntax(r_asm_t *a, int syntax)
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

int r_asm_set_pc(r_asm_t *a, u64 pc)
{
	a->pc = pc;
	return 1;
}

#if 0 
int r_asm_disasm_string(r_asm_t *a, char *string, const u8 *buf)
{
	//unsigned char *b = config.block + bytes;
	const u8 *b = buf; //config.block + bytes;
	int ret = 0;

	//ud_idx = bytes;
	udis_mem = buf;
	udis_mem_ptr = 0;

	if (bytes>63)
		bytes=63;
//	radare_read_at(seek, b, bytes);

	//b = config.block + bytes;
	string[0]='\0';
	switch(arch) {
	case ARCH_X86:
		if (ollyasm_enable) {
			t_disasm da;
			ud_disassemble(&ud_obj);
			lowercase=1;
			ret = Disasm(b, MAXCMDSIZE, seek, &da, DISASM_FILE);
			sprintf(string, "%s", da.result);
			if (da.error)
				sprintf(string, "%i -- %s - %02x %02x %02x (error)",ret,da.dump,b[0],b[1],b[2]);
		} else {
//			udis_init();
			//ud_obj.insn_offset = seek+myinc; //+bytes;
			//ud_obj.pc = seek+myinc;
			ud_obj.pc = seek;
			//ud_idx = myinc;
			ud_disassemble(&ud_obj);
			ret = ud_insn_len(&ud_obj);
			//ud_idx+=ret;
			sprintf(string, "%s", ud_insn_asm(&ud_obj));
		}
		break;
	return ret;
}
#endif 
