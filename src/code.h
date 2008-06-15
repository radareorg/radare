#ifndef _INCLUDE_CODE_H_
#define _INCLUDE_CODE_H_

#include "main.h"

extern struct list_head comments;

enum {
	ARCH_X86 = 0,
	ARCH_ARM = 1,
	ARCH_ARM16 = 2,
	ARCH_PPC = 3,
	ARCH_M68K = 4,
	ARCH_JAVA = 5,
	ARCH_MIPS = 6,
	ARCH_SPARC = 7,
	ARCH_CSR = 8
};

// generic assembly opcode structure type
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

struct aop_t {
	int type; /* type of opcode */
	int length; /* length in bytes of opcode */
	int eob; // end of block (boolean)
	u64 jump; /* true jmp */
	u64 fail; /* false jmp */
	u64 ref; /* referente to memory */
	int r_dst,r_src1,r_src2; // register arguments
	u64 i_dst,i_src1,i_src2; // inmediate arguments
};

#include "list.h"

struct reflines_t {
	u64 from;
	u64 to;
	int index;
	struct list_head list;
};

extern struct list_head data;
struct data_t {
	u64 from;
	u64 to;
	int type;
	u64 size;
	struct list_head *list;
};

extern struct list_head comments;
struct comment_t {
	u64 offset;
	const char *comment;
	struct list_head list;
};

int (*arch_aop)(u64 addr, const u8 *bytes, struct aop_t *aop);
int arch_arm_aop(u64 addr, const unsigned char *bytes, struct aop_t *aop);
int arch_mips_aop(u64 addr, const unsigned char *bytes, struct aop_t *aop);
int arch_x86_aop(u64 addr, const unsigned char *bytes, struct aop_t *aop);
int arch_java_aop(u64 addr, const unsigned char *bytes, struct aop_t *aop);
int arch_ppc_aop(u64 addr, const unsigned char *bytes, struct aop_t *aop);
int arch_csr_aop(u64 addr, const unsigned char *bytes, struct aop_t *aop);

struct reflines_t *code_lines_init();
void code_lines_free(struct list_head *list);
void code_lines_print(struct reflines_t *list, u64 addr, int expand);
void metadata_comment_add(u64 offset, const char *str);

void code_lines_print2(struct reflines_t *list, u64 addr);
int metadata_xrefs_print(u64 addr, int type);
int metadata_xrefs_add(u64 addr, u64 from, int type);

int data_set_len(u64 off, u64 len);
int data_set(u64 off, int type);
void data_add(u64 off, int type);
struct data_t *data_get(u64 offset);
int data_type_range(u64 offset);
int data_type(u64 offset);
int data_end(u64 offset);
int data_count(u64 offset);
int data_list();
void udis_jump(int n);
int udis_arch_opcode(int arch, int endian, u64 seek, int bytes, int myinc);
int dislen(u8* opcode0, int limit);

/* Virtual Machine */
int vm_init();
u64 vm_get(int reg);
const char *vm_get_name(int reg);
int vm_set(int reg, u64 value);


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
extern u64 vm_arch_x86_regs[VM_X86_N_REGS];
void vm_arch_x86_init();

extern const char **vm_arch_mips_regs_str;
extern int vm_arch_mips_nregs;
extern u64 vm_arch_mips_regs[VM_MIPS_N_REGS];
void vm_arch_mips_init();

#endif
