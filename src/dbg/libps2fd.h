#include "../main.h"
#include "regs.h"

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
#include "debug.h"
#include "thread.h"
#include "wp.h"
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
void ps_parse_argv();
#include "os.h"

#endif
