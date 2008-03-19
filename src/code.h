#ifndef _INCLUDE_CODE_H_
#define _INCLUDE_CODE_H_

#include "main.h"

// generic assembly opcode structure type
enum {
	AOP_TYPE_JMP,  // mandatory jump
	AOP_TYPE_UJMP, // unknown jump (register or so)
	AOP_TYPE_CJMP, // conditional jump
	AOP_TYPE_CALL, // call to subroutine (branch+link)
	AOP_TYPE_REP,  // repeats next instruction N times
	AOP_TYPE_RET,  // returns from subrutine
	AOP_TYPE_ILL,  // illegal instruction // trap
	AOP_TYPE_UNK,  // unknown opcode type
	AOP_TYPE_NOP,  // does nothing
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
};

struct aop_t {
	int type; /* type of opcode */
	int length; /* length in bytes of opcode */
	int eob; // end of block (boolean)
	unsigned long jump; /* true jmp */
	unsigned long fail; /* false jmp */
	u64 ref; /* referente to memory */
};

#include "list.h"

struct reflines_t {
	u64 from;
	u64 to;
	int index;
	struct list_head list;
};

int arch_arm_aop(unsigned long addr, const unsigned char *bytes, struct aop_t *aop);
int arch_mips_aop(unsigned long addr, const unsigned char *bytes, struct aop_t *aop);
int arch_x86_aop(unsigned long addr, const unsigned char *bytes, struct aop_t *aop);
int arch_java_aop(unsigned long addr, const unsigned char *bytes, struct aop_t *aop);
int arch_ppc_aop(unsigned long addr, const unsigned char *bytes, struct aop_t *aop);

#endif
