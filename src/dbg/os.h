#ifndef _INCLUDE_OS_H_
#define _INCLUDE_OS_H_
/* debugger interface for radare */
#include "regs.h"

/* lifetime */
void debug_init_calls();
int debug_init_maps(int rest);
int debug_detach();
int debug_load_threads();

/* open */
int debug_attach(int pid);
int debug_fork_and_attach();

/* flow */
int debug_contp(int tid);
int debug_os_steps();
int debug_contfork(int tid);
int debug_contscp();
int debug_status();
int debug_pstree();
int debug_getregs(pid_t tid, regs_t *regs);
int debug_setregs(pid_t tid, regs_t *regs);
int debug_single_setregs(pid_t tid, regs_t *regs);

int debug_print_wait(char *act);
int debug_dispatch_wait();

void debug_print_sigh(char *signame, unsigned long handler);

/* basic io */
int debug_read_at(pid_t tid, void *buff, int len, u64 addr);
int debug_write_at(pid_t tid, void *buff, int len, u64 addr);

/* file descriptors */
u64 debug_fd_seek(int pid, int fd, u64 addr, int whence);
int debug_fd_list(int pid);
int debug_fd_dup2(int pid, int oldfd, int newfd);
int debug_fd_open(int pid, char *file, int mode);
int debug_fd_close(int pid, int fd);
#endif
