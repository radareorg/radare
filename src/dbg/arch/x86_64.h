//#include "../arch.h"
//#include "../mem.h"
//x86_64.c
#define ull unsigned long long

int arch_jmp(u64 ptr);
addr_t arch_mmap(int fd, int size, addr_t addr);
//long long arch_syscall(int pid, int sc, ...);
int debug_dr(char *cmd);
int arch_is_jump(unsigned char *buf);
addr_t arch_get_entrypoint();
int arch_dump_registers();
int arch_opcode_size();
int arch_restore_registers();
int arch_inject(unsigned char *data, int size);
addr_t arch_pc();
int arch_backtrace();
void dump_eflags(const int eflags);
int arch_ret();
int arch_call(char *arg);
int arch_print_fpregisters(int rad, const char *mask);
int arch_print_syscall();
int arch_print_registers(int rad, const char *mask);
long long get_value(char *str);
int arch_set_register(char *reg, char *value);
int arch_continue();
//void *arch_get_sighandler(int signum);
//int arch_mprotect(char *addr, unsigned int size, int perms);
//void *arch_set_sighandler(int signum, off_t handler);
int arch_is_jmp(const unsigned char *cmd, ull *addr);
int arch_is_call(const char *cmd);
int arch_is_soft_stepoverable(const unsigned char *opcode);

//x86_64-bp.c
unsigned long dr_get (int reg);
int dr_set (int reg, unsigned long val);
inline void dr_set_control (unsigned long control);
inline unsigned dr_get_control ();
inline void dr_set_addr (int regnum, unsigned long addr);
inline void dr_reset_addr (int regnum);
inline unsigned long dr_get_status (void);
int arch_bp_hw_state(unsigned long addr, int enable);
void dr_init();
void dr_list();
int arch_set_wp_hw_n(int dr_free, unsigned long addr, int type);
int arch_set_wp_hw(unsigned long addr, int type);
int arch_set_bp_hw(struct bp_t *bp, unsigned long addr);
int arch_rm_bp_hw(struct bp_t *bp);
int get_len_ins(char *buf, int len);
int arch_set_bp_soft(struct bp_t *bp, unsigned long addr);
int arch_rm_bp_soft(struct bp_t *bp);

//debug.c
//int debug_getregs(pid_t tid, regs_t *regs);
//int debug_setregs(pid_t tid, regs_t *regs);

addr_t arch_alloc_page(unsigned long size, unsigned long *rsize);
