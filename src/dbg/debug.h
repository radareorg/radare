#ifndef _INCLUDE_DEBUG_H_
#define _INCLUDE_DEBUG_H_

#include "libps2fd.h"
#include "os.h"

// arch/ dir
#include "arch/arch.h"

// debug.c
int debug_init();
int debug_ktrace();
void debug_exit();
int debug_breakpoint(unsigned long addr);
int debug_load();
int debug_read(pid_t pid, void *addr, int length);
int debug_write(pid_t pid, void *data, int length);
int debug_run();
int debug_bp(const char *addr);
int debug_wp(const char *expr);
int debug_wtrace();
//int debug_detach();
int debug_syms();
int debug_system(const char *command);
int debug_cont();
int debug_contu();
int debug_contsc();
int debug_contscp();
int debug_registers(int rad);
int debug_oregisters(int rad);
int debug_fpregisters(int rad);
int debug_step(int times);
int debug_stepo();
int debug_stepu();
int debug_stepret();
int debug_skip();
int debug_trace();
int debug_init_maps(int rest);
int debug_print_maps();
int debug_open(const char *pathname, int flags, mode_t mode);
int debug_close(int fd);
off_t debug_lseek(int fildes, off_t offset, int whence);
int debug_status();
int debug_pids();
int debug_dr();
int debug_fd(char *cmd);
int debug_th(char *cmd);
int debug_bt();
int debug_info();
int debug_signal(char *);
//int debug_contfork();
int debug_pstree();
int debug_attach();
int debug_unload();
int debug_inject();
int debug_set_register(char *arg);
int debug_alloc(char *arg);
int debug_mmap(char *arg);
int debug_free(char *arg);
int debug_jmp(char *arg);
int debug_call(char *arg);
int debug_imap(char *arg);
//int arch_hack(char *arg);
int debug_write_at(pid_t pid, void *data, int length, off_t addr);
int debug_read_at(pid_t pid, void *addr, int length, off_t at);
int inline debug_contp(int pid);
int inline debug_steps();


//#include "libps2fd.h"
//inline int debug_getregs(pid_t pid, regs_t *reg);
//inline int debug_setregs(pid_t pid, regs_t *reg);
int dispatch_wait();
int debug_os_init();
void print_wps();
int rm_wp(int i);
void print_sigh(char *signame, unsigned long handler);
void debug_environment();
//inline unsigned long debug_get_regoff(regs_t *reg, int off);
inline int restore_bp();
int getv();


// events.c
int events_init();
int events_get();

enum {
	DEBUG_CONT,
	DEBUG_CONTFORK,
	DEBUG_CONTSC,
	DEBUG_STEP
};

enum {
	UNKNOWN_EVENT,
	BP_EVENT,
	INT3_EVENT,
	EXIT_EVENT
};

enum {
	BP_NONE,
	BP_HARD,
	BP_SOFT
};

struct bp_t
{
	int hw;
	unsigned long addr;
	unsigned char data[512];
	int len;
};

#endif
