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
	ut64 value;
	int type;
	char *get;
	char *set;
	struct list_head list;
};

struct vm_op_t {
	char opcode[32];
	char code[1024];
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
	ut64 from;
	ut64 to;
	u8 *data;
	struct list_head list;
};

#if LIBR
struct r_vm_t {
	struct list_head vm_regs;
	struct vm_cpu_t vm_cpu;
	ut64 vm_stack_base = 0;
	u8 *vm_stack = NULL;
	struct list_head vm_mmu_cache;
	int realio = 0;
}
#endif

ut64 vm_reg_get(const char *name);
void vm_stack_push(ut64 _val);
void trace_reset();
void trace_init();
int trace_sort();
struct trace_t *trace_get(ut64 addr, int tag);
int trace_tag_get();
int trace_tag_set(int id);
void flag_rebase(const char *text);
int program_save(const char *file);
void undo_write_clear();
void flag_grep_help();
int data_list_ranges();
int vm_cmd_op(const char *op);
void print_mem_add(char *name, char *fmt);
void trace_show(int, int);
int trace_set_times(ut64,int);
void graph_viz(struct program_t *, int);
int radare_macro_break(const char *v);
int data_var_cmd(const char *);
void analyze_preludes(char *);
// void grava_program_graph(struct program_t , struct mygrava_window *);
int var_cmd(const char *);
void ugraph_print_dot(int);
int radare_write_op(const char *, char op);
int radare_write_mask_str(const char*);
void grava_program_graph(struct program_t *, struct mygrava_window *);
void stdout_close();

#endif
