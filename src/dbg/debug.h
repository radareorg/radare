#ifndef _INCLUDE_DEBUG_H_
#define _INCLUDE_DEBUG_H_

#include "libps2fd.h"
#include "regs.h"
#include "arch/arch.h"
#include "parser.h"

int debug_reg(const char *arg);
void debug_msg();
int debug_dumpall(const char *arg);
void debug_msg_set(const char *format, ...);
// bp.c
int debug_bp(const char *addr);
int debug_bp_set(struct bp_t *bp, u64 addr, int type);
int debug_bp_rm(u64 addr, int type);
int debug_bp_rm_num(int num);
int debug_tt(const char *arg);
int debug_bp_rm_addr(u64 addr);
int debug_bp_restore(int pos);
struct bp_t *debug_bp_get(addr_t addr);
struct bp_t *debug_bp_get_num(int num);

// debug.c
//
int debug_init();
int debug_ktrace();
void debug_exit();
int debug_breakpoint(unsigned long addr);
int debug_load();
int debug_read(pid_t pid, void *addr, int length);
int debug_write(pid_t pid, void *data, int length);
int debug_run();
int debug_wp(const char *expr);
int debug_mp(char *expr);
int debug_wtrace();
int debug_detach();
int debug_syms();
int debug_system(const char *command);
int debug_cont(const char *input);
int debug_ie();
int debug_contu();
int debug_contuh();
int debug_contsc();
int debug_contscp();
int debug_contfork();
int debug_registers(int rad);
int debug_oregisters(int rad);
int debug_dregisters(int rad);
int debug_fpregisters(int rad);
int debug_step(int times);
int debug_stepbp(int times);
int debug_stepo();
void debug_dumpcore();
int debug_stepu();
int debug_stepret();
int debug_skip();
int debug_trace();
int debug_init_maps(int rest);
int debug_print_maps();
int debug_open(const char *pathname, int flags, mode_t mode);
int debug_close(int fd);
u64 debug_lseek(int fildes, u64 offset, int whence);
int debug_status();
int debug_pids();
int debug_dr();
int debug_fd(char *cmd);
int debug_th(char *cmd);
int debug_bt();
int debug_jmp(const char *str);
int debug_call(const char *str);
int debug_info();
int debug_signal(const char *);
//int debug_contfork();
struct bp_t *debug_get_bp(addr_t addr);
int debug_pstree();
int debug_attach();
int debug_unload();
int debug_inject();
int debug_set_register(char *arg);
int debug_loop(char *addr_str);
u64 debug_alloc(char *arg);
int debug_mmap(char *arg);
int debug_free(char *arg);
int debug_imap(char *arg);
int debug_waitpid(int pid, int *status);
void debug_reload_bps();
int debug_write_at(pid_t pid, void *data, int length, u64 addr);
int debug_read_at(pid_t pid, void *addr, int length, u64 at);
int inline debug_contp(int pid);
int inline debug_steps();
u64 debug_get_regoff(regs_t *reg, int off);
int debug_set_register(char *args);
void debug_set_regoff(regs_t *regs, int off, unsigned long val);
addr_t debug_get_register(char *reg);


//#include "libps2fd.h"
//inline int debug_getregs(pid_t pid, regs_t *reg);
//inline int debug_setregs(pid_t pid, regs_t *reg);
int dispatch_wait();
int debug_os_init();
int debug_os_kill(int pid, int sig);
int debug_cont_until(const char *input);
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
	INT_EVENT,	/* interrupt, CTRL-C */
	FATAL_EVENT,	/* fatal exception (access violation) */
	EXIT_EVENT,
	CLONE_EVENT
};

enum {
	BP_NONE,
	BP_HARD,
	BP_SOFT
};


struct event_t {
	char *name;
	int id;
	int ignored;
};

struct bp_t
{
	int hw;
	u64 addr;
	int len;
	unsigned char data[512];
};

struct debug_t {
	int fd;         /* related metadata */
	int verbose;
	char *msg;
	pid_t opid;
	pid_t pid;
	pid_t tid;
	char *filename;
	struct list_head th_list;	/* thread list */
	struct list_head map_mem;	/* alloc regions */
	struct list_head map_reg;	/* mapped regions */
	TH_INFO	*th_active;		/* active thread */
	WP	wps[4];			/* watchpoints */
	int	wps_n;			/* number of watchpoints */
	unsigned int mem_sz;
	unsigned int map_regs_sz;
	int bps_n;      /*/ breakpoints count */
	u64 offset;
	u64 ldentry;
	u64 entrypoint;
	u64 pc;       /*/ program counter */
	int isbpaddr;
	int opened;
	int steps;
	int is_file;
	struct wait_state	ws;
	struct bp_t bps[MAX_BPS];
	char *bin_usrcode;
	char *args;
	char *argv[256];

#if __WINDOWS__
	PROCESS_INFORMATION pi;
	#define WIN32_PI(f)	ps.pi.f
#endif
};

extern struct event_t events[];
int is_code(addr_t pc);

#endif
