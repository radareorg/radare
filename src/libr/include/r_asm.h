#ifndef _INCLUDE_R_ASM_H_
#define _INCLUDE_R_ASM_H_

enum {
	R_ASM_ARCH_NULL  = 0,
	R_ASM_ARCH_X86   = 1,
	R_ASM_ARCH_ARM   = 2,
	R_ASM_ARCH_PPC   = 3,
	R_ASM_ARCH_M68K  = 4,
	R_ASM_ARCH_JAVA  = 5,
	R_ASM_ARCH_MIPS  = 6,
	R_ASM_ARCH_SPARC = 7,
	R_ASM_ARCH_CSR   = 8,
	R_ASM_ARCH_MSIL  = 9,
	R_ASM_ARCH_OBJD  = 10,
	R_ASM_ARCH_BF    = 11,
};

enum {
	R_ASM_SYN_NULL   = 0,
	R_ASM_SYN_INTEL  = 1,
	R_ASM_SYN_ATT    = 2,
	R_ASM_SYN_OLLY   = 3,
	R_ASM_SYN_PSEUDO = 4
};

enum {
	R_ASM_MODE_NULL  = 0,
	R_ASM_MODE_16    = 16,
	R_ASM_MODE_32    = 32,
	R_ASM_MODE_64    = 64
};

enum {
	R_ASM_BIG_ENDIAN = 0,
	R_ASM_LIL_ENDIAN = 1
};

struct r_asm_t {
	int arch;
	int mode;
	int endianess;
	int syntax;
	u64 pc;
	u32 (*r_asm_disasm_buf)(struct r_asm_t *a, u8 *string, u8 *buf, u32 len);
};

/* asm.c */
int r_asm_init(struct r_asm_t *a);
int r_asm_set_arch(struct r_asm_t *a, int arch);
int r_asm_set_mode(struct r_asm_t *a, int mode);
int r_asm_set_endianess(struct r_asm_t *a, int endianess);
int r_asm_set_syntax(struct r_asm_t *a, int syntax);
int r_asm_set_pc(struct r_asm_t *a, u64 pc);
u32 r_asm_disasm_buf(struct r_asm_t *a, u8 *string, u8 *buf, u32 len);

/* arch/x86/asm.c */
u32 r_asm_x86_disasm_buf(struct r_asm_t *a, u8 *string, u8 *buf, u32 len);
#endif
