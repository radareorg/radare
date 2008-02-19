#include "../main.h"

#ifndef _INCLUDE_LIBPS2FD_H_
#define _INCLUDE_LIBPS2FD_H_

#undef __UNIX__
#undef __WINDOWS__
#if __WIN32__ || __CYGWIN__ || MINGW32
#define __WINDOWS__ 1
#else
#define __UNIX__ 1
#endif

#if __WINDOWS__

#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4

#include <windows.h>
#include <winbase.h>
typedef struct {
	ULONG ContextFlags;

	ULONG   Dr0;
	ULONG   Dr1;
	ULONG   Dr2;
	ULONG   Dr3;
	ULONG   Dr6;
	ULONG   Dr7;

	FLOATING_SAVE_AREA FloatSave;

	ULONG   SegGs;
	ULONG   SegFs;
	ULONG   SegEs;
	ULONG   SegDs;

	ULONG   Edi;
	ULONG   Esi;
	ULONG   Ebx;
	ULONG   Edx;
	ULONG   Ecx;
	ULONG   Eax;

	ULONG   Ebp;
	ULONG   Eip;
	ULONG   SegCs;              // MUST BE SANITIZED
	ULONG   EFlags;             // MUST BE SANITIZED
	ULONG   Esp;
	ULONG   SegSs;

	UCHAR   ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];
} CONTEXTO;

#define regs_t CONTEXTO

#else

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ptrace.h>

#endif

#if __linux__
#include <asm/ptrace.h>
#endif
#include <unistd.h>
#include <signal.h>


#include "thread.h"
#include "wp.h"
#include "debug.h"
#include "../utils.h"
#include "../list.h"

#include "arch/cpu.h"
#if __i386__ || __x86_64__
#include "arch/i386.h"
#else
#include "arch/arm.h"
#endif
// XXX?
//#include "os.h"

#ifndef OFF_FMT
#define OFF_FMT "0x%08x"
#define OFF_FMTx "%x"
#define OFF_FMTd "%d"
#endif

#if __i386__
#if __linux__
 #if __x86_64__
  /* linux 64 bits */
  #include <sys/user.h>
  #define regs_t struct user_regs_struct
 #else
  /* linux 32 bits */
  #include <sys/user.h>
  #define regs_t struct user_regs_struct
 #endif

#elif __WINDOWS__
  #define regs_t CONTEXTO
#elif __APPLE__
#include <mach/i386/_structs.h>
#define regs_t _STRUCT_X86_THREAD_STATE32
#else
  /* bsd 32 bits */
  #include <machine/reg.h>
  #define regs_t struct reg
#endif
  /* ARM */
#endif

  /* portable ptrace */
#if __linux__
#define PTRACE_PC 0
#else
#define PTRACE_PC arch_pc()
#endif

// BSD compatibility
#if __Solaris__
#define PTRACE_PEEKTEXT PTRACE_READTEXT
#define PTRACE_POKETEXT PTRACE_WRITETEXT
#if 0
        PTRACE_ATTACH                    /* 10, attach to an existing process */
        PTRACE_DETACH                    /* 11, detach from a process */
        PTRACE_GETREGS                   /* 12, get all registers */
        PTRACE_SETREGS                   /* 13, set all registers */
        PTRACE_GETFPREGS                 /* 14, get all floating point regs */
        PTRACE_SETFPREGS                 /* 15, set all floating point regs */
        PTRACE_READDATA                  /* 16, read data segment */
        PTRACE_WRITEDATA                 /* 17, write data segment */
        PTRACE_READTEXT                  /* 18, read text segment */
        PTRACE_WRITETEXT                 /* 19, write text segment */
        PTRACE_GETFPAREGS                /* 20, get all fpa regs */
        PTRACE_SETFPAREGS                /* 21, set all fpa regs */
        PTRACE_GETWINDOW                 /* 22, get register window n */
        PTRACE_SETWINDOW                 /* 23, set register window n */
        PTRACE_22                        /* 22, filler */
        PTRACE_23                        /* 23, filler */
        PTRACE_SYSCALL                   /* 24, trap next sys call */

        PTRACE_DUMPCORE                  /* 25, dump process core */
        PTRACE_SETWRBKPT                 /* 26, set write breakpoint */
        PTRACE_SETACBKPT                 /* 27, set access breakpoint */
        PTRACE_CLRDR7                    /* 28, clear debug register 7 */
        PTRACE_26                        /* 26, filler */
        PTRACE_27                        /* 27, filler */
        PTRACE_28                        /* 28, filler */
        PTRACE_GETUCODE                  /* 29, get u.u_code */ 
     // ^- this is the trap reason
#endif
#endif

#if __FreeBSD__ || __NetBSD__ || __OpenBSD__ || __APPLE__
#define PTRACE_KILL PT_KILL
#define PTRACE_DETACH PT_DETACH
#define PTRACE_TRACEME PT_TRACE_ME
#define PTRACE_ATTACH PT_ATTACH
#define PTRACE_SYSCALL PT_SYSCALL
#define PTRACE_CONT PT_CONTINUE
#define PTRACE_PEEKTEXT PT_READ_D
#define PTRACE_POKEDATA PT_WRITE_D
#define PTRACE_GETREGS PT_GETREGS
#define PTRACE_SETREGS PT_SETREGS
#define PTRACE_GETFPREGS PT_GETFPREGS
#define PTRACE_SETFPREGS PT_SETFPREGS
#define PTRACE_SINGLESTEP PT_STEP
#endif

#define MAX_BPS	15

#define WS(w) (ps.ws.w)
#define WS_SI(f) (ps.ws.si.f)

//#ifndef regs_t
#if __arm__
  #define regs_t elf_gregset_t
//#else
//  #define regs_t struct user_regs_struct
#endif
//#endif
#if __WINDOWS__
#define siginfo_t int
#endif

struct wait_state {
	int event;
	struct bp_t *bp;
	regs_t regs;
	siginfo_t si;
};

struct debug_t {
	int fd;         /* related metadata */
	int verbose;
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

void *mmap_tagged_page(char *file, u64 addr, u64 size);


int       (*__open)   (const char *pathname, int flags, mode_t mode);
int       (*__open64) (const char *pathname, int flags, mode_t mode);
int       (*__close)  (int fd);
ssize_t   (*__read)   (int fd, void *buf, size_t count);
ssize_t   (*__read64) (int fd, void *buf, size_t count);
ssize_t   (*__write)  (int fd, const void *buf, size_t count);
ssize_t   (*__write64)(int fd, const void *buf, size_t count);
u64     (*__lseek)  (int fildes, u64 offset, int whence);
u64     (*__lseek64)(int fildes, u64 offset, int whence);
u64     (*___llseek)(int fildes, u64 offset, int whence);
int *     (*__system) (const char *command);
extern struct debug_t ps;

#endif
