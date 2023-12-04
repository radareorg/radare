#ifndef _INCLUDE_CODE_H_
#define _INCLUDE_CODE_H_

#include "main.h"

int dis51_udis (char *str, const ut8 *bytes, int len, ut64 seek);

int trace_count(ut64 addr);
int trace_times(ut64 addr);
ut64 data_seek_to(ut64 offset, int type, int idx);
int trace_add(ut64 addr);
void trace_init();


struct trace_t {
	ut64 addr;
	ut64 tags;
	int opsize;
	int times;
	int count;
	struct timeval tm;
	struct list_head list;
};

enum {
	DATA_HEX    = FMT_HEXB,  /* hex byte pairs */
	DATA_STR    = FMT_ASC0,  /* ascii string */
	DATA_CODE   = FMT_UDIS,  /* plain assembly code */
	DATA_FUN    = FMT_DISAS, /* plain assembly code */
	DATA_STRUCT = FMT_MEMORY,/* memory */
	DATA_FOLD_O = 0x100,     /* open folder */
	DATA_FOLD_C = 0x101,     /* closed folder */
	DATA_EXPAND = 0x200 
};

enum {
	ARCH_NULL  = 0,
	ARCH_X86   = 1,
	ARCH_ARM   = 2,
	ARCH_ARM16 = 3,
	ARCH_PPC   = 4,
	ARCH_M68K  = 5,
	ARCH_JAVA  = 6,
	ARCH_MIPS  = 7,
	ARCH_SPARC = 8,
	ARCH_CSR   = 9,
	ARCH_MSIL  = 10,
	ARCH_OBJD  = 11,
	ARCH_BF    = 12,
	ARCH_8051  = 13,
	ARCH_Z80   = 14,
	ARCH_AOP   = 0x10000
};

enum {
	BLK_TYPE_HEAD,  // first block
	BLK_TYPE_BODY,  // conditional jump
	BLK_TYPE_LAST,  // ret
	BLK_TYPE_FOOT   // unknown jump
};

enum {
	AOP_TYPE_JMP,  // mandatory jump
	AOP_TYPE_UJMP, // unknown jump (register or so)
	AOP_TYPE_CJMP, // conditional jump
	AOP_TYPE_CALL, // call to subroutine (branch+link)
	AOP_TYPE_RCALL,// call to register
	AOP_TYPE_REP,  // repeats next instruction N times
	AOP_TYPE_RET,  // returns from subrutine
	AOP_TYPE_ILL,  // illegal instruction // trap
	AOP_TYPE_UNK,  // unknown opcode type
	AOP_TYPE_NOP,  // does nothing
	AOP_TYPE_MOV,  // register move
	AOP_TYPE_TRAP,  // it's a trap!
	AOP_TYPE_SWI,  // syscall, software interrupt
	AOP_TYPE_UPUSH, // unknown push of data into stack
	AOP_TYPE_PUSH,  // push value into stack
	AOP_TYPE_POP,   // pop value from stack to register
	AOP_TYPE_CMP,   // copmpare something
	AOP_TYPE_ADD,
	AOP_TYPE_SUB,
	AOP_TYPE_MUL,
	AOP_TYPE_DIV,
	AOP_TYPE_SHR,
	AOP_TYPE_SHL,
	AOP_TYPE_OR,
	AOP_TYPE_AND,
	AOP_TYPE_XOR,
	AOP_TYPE_NOT,
	AOP_TYPE_STORE, /* store from register to memory */
	AOP_TYPE_LOAD, /* load from memory to register */
};

enum {
	AOP_STACK_NOP = 0, /* sub $0xc, %esp */
	AOP_STACK_INCSTACK, /* sub $0xc, %esp */
	AOP_STACK_LOCAL_GET,
	AOP_STACK_LOCAL_SET,
	AOP_STACK_ARG_GET,
	AOP_STACK_ARG_SET,
};

struct aop_t {
	int type; /* type of opcode */
	int stackop; /* operation on stack? */
	int length; /* length in bytes of opcode */
	int eob; // end of block (boolean)
	ut64 jump; /* true jmp */
	ut64 fail; /* false jmp */
	ut64 ref; /* referente to memory */
	ut64 value; /* referente to value */
	int r_dst,r_src1,r_src2; // register arguments
	ut64 i_dst,i_src1,i_src2; // inmediate arguments
};

extern int (*arch_aop)(ut64 addr, const u8 *bytes, struct aop_t *aop);
int arch_aop_aop(ut64 addr, const u8 *bytes, struct aop_t *aop);
int arch_8051_aop(ut64 addr, const u8 *bytes, struct aop_t *aop);
int arch_z80_aop(ut64 addr, const u8 *bytes, struct aop_t *aop);
int arch_bf_aop(ut64 addr, const u8 *buf, struct aop_t *aop);
int arch_sparc_aop(ut64 addr, const unsigned char *bytes, struct aop_t *aop);
int arch_arm_aop(ut64 addr, const unsigned char *bytes, struct aop_t *aop);
int arch_mips_aop(ut64 addr, const unsigned char *bytes, struct aop_t *aop);
int arch_x86_aop(ut64 addr, const unsigned char *bytes, struct aop_t *aop);
int arch_java_aop(ut64 addr, const unsigned char *bytes, struct aop_t *aop);
int arch_ppc_aop(ut64 addr, const unsigned char *bytes, struct aop_t *aop);
int arch_csr_aop(ut64 addr, const unsigned char *bytes, struct aop_t *aop);
int arch_m68k_aop(ut64 addr, const unsigned char *bytes, struct aop_t *aop);
int arch_msil_aop(ut64 addr, const unsigned char *bytes, struct aop_t *aop);

int gnu_dismips_str(char *str, const u8 *inst, ut64 offset);
int gnu_disarm_str(char *str, const u8 *inst, ut64 offset);
int gnu_disparc_str(char *str, const u8 *inst, ut64 offset);
int z80_udis (char *str, ut8 *bytes, int len, ut64 seek);

struct aop_t *pas_aop(int arch, ut64 seek, const u8 *bytes, int len, struct aop_t *aop, char *newstr, int pseudo);
struct reflines_t *code_lines_init(int );
void code_lines_free(struct list_head *list);
void code_lines_print(struct reflines_t *list, ut64 addr, int expand);
void metadata_comment_add(ut64 offset, const char *str);

void code_lines_print2(struct reflines_t *list, ut64 addr);
int metadata_xrefs_print(ut64 addr, int type);
int metadata_xrefs_print(ut64 addr, int type);
int metadata_xrefs_add(ut64 addr, ut64 from, int type);
void metadata_xrefs_del(ut64 addr, ut64 from, int data);
void metadata_comment_del(ut64 offset, const char *str);
void metadata_comment_add(ut64 offset, const char *str);

struct data_t *data_get_range(ut64 offset);
struct data_t *data_get_between(ut64 from, ut64 to);
int data_set_len(ut64 off, ut64 len);
int data_set(ut64 off, int type);
struct data_t *data_add_arg(ut64 off, int type, const char *arg);
struct data_t *data_add(ut64 off, int type);
struct data_t *data_get(ut64 offset);
int data_type_range(ut64 offset);
int data_type(ut64 offset);
int data_end(ut64 offset);
int data_size(ut64 offset);
// int data_list();
void udis_jump(int n);
int udis_arch_opcode(int arch, const u8 *b, int endian, ut64 seek, int bytes, int myinc);
void udis_arch(int arch, int len, int rows);
void udis_arch_buf(int arch, const u8 *block, int len, int rows);
int dislen(u8* opcode0, int limit);

int arch_bf_dis(const u8* buf, ut64 addr, int len);

/* Virtual Machine */
int vm_init(int);
int vm_import(int vm);
ut64 vm_get(int reg);
int vm_eval(const char *str);
int vm_op_eval(const char *str);
int vm_reg_alias(const char *name, const char *get, const char *set);
void udis_init();
int var_init();
void radis_update();
int vm_emulate(int n);
int print_mem_del(char *name);
void analyze_spcc(const char *name);
void graph_set_user(int b);
int vm_cmd_reg(const char*);
void vm_print(int type);
int vm_eval_file(const char *str);
int vm_cmd_op_help();
int vm_op_list();
int cons_palette_set(const char *key, const u8 *value);
ut64 undo_get_last_seek();
const char *vm_get_name(int reg);
int vm_set(int reg, ut64 value);


/* arch/x86/vm.c */
enum {
	VM_X86_EAX = 0,
	VM_X86_ECX,
	VM_X86_EDX,
	VM_X86_EBX,
	VM_X86_ESI,
	VM_X86_EDI,
	VM_X86_ESP,
	VM_X86_EBP,
	VM_X86_N_REGS
};

/* arch/mips/vm.c */
enum {
	VM_MIPS_ZERO = 0,
	VM_MIPS_AT,
	VM_MIPS_V0,
	VM_MIPS_V1,
	VM_MIPS_A0,
	VM_MIPS_A1,
	VM_MIPS_A2,
	VM_MIPS_A3,
	VM_MIPS_A4,
	VM_MIPS_A5,
	VM_MIPS_A6,
	VM_MIPS_A7,
	VM_MIPS_T0,
	VM_MIPS_T1,
	VM_MIPS_T2,
	VM_MIPS_T3,
	VM_MIPS_S0,
	VM_MIPS_S1,
	VM_MIPS_S2,
	VM_MIPS_S3,
	VM_MIPS_S4,
	VM_MIPS_S5,
	VM_MIPS_S6,
	VM_MIPS_S7,
	VM_MIPS_T8,
	VM_MIPS_T9,
	VM_MIPS_K0,
	VM_MIPS_K1,
	VM_MIPS_GP,
	VM_MIPS_SP,
	VM_MIPS_S8,
	VM_MIPS_RA,
	VM_MIPS_N_REGS
};

extern const char *vm_arch_x86_regs_str[VM_X86_N_REGS];
extern int vm_arch_x86_nregs;
extern ut64 vm_arch_x86_regs[VM_X86_N_REGS];
void vm_arch_x86_init();

extern const char **vm_arch_mips_regs_str;
extern int vm_arch_mips_nregs;
extern ut64 vm_arch_mips_regs[VM_MIPS_N_REGS];
void vm_arch_mips_init();
int analyze_function(ut64 from, int recursive, int report);
char *metadata_comment_get(ut64 offset, int lines);
int radare_analyze(ut64 seek, int size, int depth, int rad);
void radis_str_e(int arch, const u8 *block, int len, int rows);
int udis_arch_string(int arch, char *string, const u8 *buf, int endian, ut64 seek, int bytes, int myinc);
int search_from_simple_file(char *file);
int radare_search_asm(const char *str);
int do_byte_pat(int);
void search_similar_pattern(int);
void analyze_progress(int _o, int _x, int _p, int _v);

#endif
