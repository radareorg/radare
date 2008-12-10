#ifndef _INCLUDE_VM_H_
#define _INCLUDE_VM_H_

#define VMREG_BIT   1
#define VMREG_INT8  2
#define VMREG_INT16 3
#define VMREG_INT32 4
#define VMREG_INT64 5
#define VMREG_FLOAT32 6
#define VMREG_FLOAT64 7

struct vm_reg_t {
	char name[16];
	u64 value;
	int type;
	char *get;
	char *set;
	struct list_head list;
};

struct vm_reg_type {
	int type;
	char *str;
};

struct vm_cpu_t {
	const char *pc;
	const char *sp;
	const char *bp;
	const char *ctr;
	const char *a0;
	const char *a1;
	const char *a2;
	const char *a3;
	const char *ret;
	const char *zf;
};

struct vm_change_t {
	u64 from;
	u64 to;
	u8 *data;
	struct list_head list;
};

u64 vm_reg_get(const char *name);
void vm_stack_push(u64 _val);

#endif
