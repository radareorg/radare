#ifndef _INCLUDE_ARCH_H_
#define _INCLUDE_ARCH_H_

#include <sys/types.h>
/* linux stuff */
#ifndef SYS_gettid
#define SYS_gettid 224
#endif
#ifndef SYS_getpid
#define SYS_getpid 20
#endif
#ifndef SYS_tkill
#define SYS_tkill 238
#endif
int arch_backtrace();
int arch_stackanal();
int arch_continue();
off_t arch_get_entrypoint();
void *arch_get_sighandler(int signum);
int arch_is_breakpoint(int pre); // 1 to restore eip
int arch_is_stepoverable(const unsigned char *cmd);
int arch_jmp(off_t ptr);
off_t arch_pc();
int arch_ret();
long long arch_syscall(int pid, int sc, ...);
int arch_call(char *arg);
int arch_jmp();
int arch_print_fpregisters(int rad, const char *mask);
int arch_print_registers(int rad, const char *mask);
int arch_reset_breakpoint(int step); // 1 to remove bp, step + restore bp
int arch_restore_breakpoint(int pre); // 1 to restore eip
struct bp_t *arch_set_breakpoint(off_t addr);
int arch_set_register(char *reg, char *value);
void *arch_set_sighandler(int signum, off_t handler);
int arch_hack(const char *cmd);
int arch_set_wp_hw_n(int dr_free, unsigned long addr, int type);

#endif
