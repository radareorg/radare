#ifndef _INCLUDE_LIBPS2FD_H_
#define _INCLUDE_LIBPS2FD_H_

#undef __UNIX__
#undef __WINDOWS__
#if __WIN32__ || __CYGWIN__ || MINGW32
#define __UNIX__ 0
#define __WINDOWS__ 1
#else
#define __WINDOWS__ 0
#define __UNIX__ 1
#endif

#include "../main.h"
#include "regs.h"
#include "bp.h"

#if __WINDOWS__
#else

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

/* no ptrace for sun solaris ? */
#ifndef __sun
#include <sys/ptrace.h>
#endif

#endif

#if __linux__
//#include <asm/ptrace.h>
#endif
#include <unistd.h>
#include "signals.h"
#include "debug.h"
#include "thread.h"
#include "wp.h"
#include "../utils.h"
#include "../list.h"

#include "arch/cpu.h"
#if __i386__ || __x86_64__
#include "arch/i386.h"
#else
#if __mips__
#include "arch/mips.h"
#elif __arm__
#include "arch/arm.h"
#elif __powerpc__ || __POWERPC__
#include "arch/powerpc.h"
#else
#error Unknown cpu?
#endif
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
