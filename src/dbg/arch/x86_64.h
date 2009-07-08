//#include "../arch.h"
//#include "../mem.h"
//x86_64.c

#if 0
#define CPU_ARG0(x) R_RAX(x)
#define CPU_ARG1(x) R_RBX(x)
#define CPU_ARG2(x) R_RCX(x)
#define CPU_ARG3(x) R_RDX(x)
#define CPU_SP(x) R_RSP(x)
#define CPU_PC(x) R_RIP(x)
#define CPU_RET(x) R_RAX(x) /* return value */
#endif

#define SYSCALL_OPS64	"\x0f\x05\xcc\x90"	/* syscall, int $0x3, nop */

int arch_jmp(ut64 ptr);
addr_t arch_mmap(int fd, int size, addr_t addr);
//long long arch_syscall(int pid, int sc, ...);
int debug_dr(const char *cmd);
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
int arch_call(const char *arg);
int arch_print_fpregisters(int rad, const char *mask);
int arch_print_syscall();
int arch_print_registers(int rad, const char *mask);
ut64 get_value(const char *str);
int arch_set_register(const char *reg, const char *value);
int arch_continue();
//void *arch_get_sighandler(int signum);
//int arch_mprotect(char *addr, unsigned int size, int perms);
//void *arch_set_sighandler(int signum, off_t handler);
int arch_is_jmp(const unsigned char *cmd, ut64 *addr);
int arch_is_call(const char *cmd);
int arch_is_soft_stepoverable(const unsigned char *opcode);

//x86_64-bp.c
ut64 dr_get (int reg);
int dr_set (int reg, ut64 val);
void dr_set_control (ut32 control);
unsigned dr_get_control ();
void dr_set_addr (int regnum, ut64 addr);
void dr_reset_addr (int regnum);
unsigned long dr_get_status (void);
int arch_bp_hw_state(ut64 addr, int enable);
void dr_init();
void dr_list();
int arch_set_wp_hw_n(int dr_free, ut64 addr, int type);
int arch_set_wp_hw(ut64 addr, int type);
int arch_set_bp_hw(struct bp_t *bp, ut64 addr);
int arch_rm_bp_hw(struct bp_t *bp);
int get_len_ins(char *buf, int len);
int arch_set_bp_soft(struct bp_t *bp, ut64 addr);
int arch_rm_bp_soft(struct bp_t *bp);

//debug.c
//int debug_getregs(pid_t tid, regs_t *regs);
//int debug_setregs(pid_t tid, regs_t *regs);

