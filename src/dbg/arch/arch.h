#ifndef ARCH_H
#define ARCH_H

#include "../../list.h"
#include "../../radare.h"
#include "../../code.h"


#include "../debug.h"

/*
#if __x86_64__
typedef ut64 addr_t;
#elif __arm
typedef ut32 addr_t;
#else
typedef ut32 addr_t;
#endif
*/

struct list_head *arch_bt();
void arch_view_bt(struct list_head *);
void free_bt(struct list_head *b);
int arch_rm_bp_hw(struct bp_t *bp);
int arch_rm_bp_soft(struct bp_t *bp);
int arch_set_wp_hw_n(int dr_free, ut64 addr, int type);
int arch_restore_bp(struct bp_t *bp);
int arch_set_bp_hw(struct bp_t *bp, ut64 addr);
int arch_set_bp_soft(struct bp_t *bp, ut64 addr);
struct bp_t *arch_stopped_bp();
int arch_backtrace();
int arch_stackanal();
int arch_print_syscall();
addr_t arch_pc();
addr_t arch_get_entrypoint();
addr_t arch_get_sighandler(int signum);
int arch_print_registers(int n, const char *l);
addr_t arch_mmap(int fd, int size, addr_t addr);
int arch_set_register(const char *args, const char *value);
int arch_print_fpregisters(int rad, const char *mask);
int arch_print_syscall();
int arch_jmp(ut64 addr);
int arch_is_stepoverable(const unsigned char *cmd);
int arch_is_soft_stepoverable(const unsigned char *opcode);
int arch_ret();
int arch_call(const char *arg);
int arch_set_wp_hw_n(int dr_free, ut64 addr, int type);
addr_t arch_set_sighandler(int signum, addr_t handler);
int arch_hack(const char *cmd);
ut64 arch_syscall(int pid, int sc, ...);
int arch_dump_registers();
int arch_restore_registers();
void arch_set_pc(addr_t pc);
ut64 get_reg(const char *reg);
int arch_is_fork();
//int (*arch_aop)(ut64 addr, const u8 *bytes, struct aop_t *aop);


#endif
