#ifndef _INCLUDE_I386_H_
#define _INCLUDE_I386_H_

#ifndef _INCLUDE_CPU_H_
#error Do not include i386.h directly!
#endif

#include <limits.h>

/* ensure */
#undef __UNIX__
#undef __WINDOWS__
#if __WIN32__ || __CYGWIN__ || MINGW32
#define __WINDOWS__ 1
#else
#define __UNIX__ 1
#endif

#undef offsetof
#define offsetof(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)

void dr_list();
void dr_init();

#if __linux__
  #include <sys/user.h>
  #if __i386__
    #include "i386-linux.h"
  #elif __x86_64__
    #include "i386-linux64.h"
  #endif
#elif __WINDOWS__
 #include <windows.h>
 #include "i386-w32.h"
#elif __APPLE__
 #include "i386-darwin.h"
#elif __sun
 #include "i386-solaris.h"
#else

#include <machine/reg.h>
#include <sys/param.h>
#include <sys/user.h>

#if defined(__amd64)
#undef __x86_64__
#undef __x86_64
#define __x86_64__ 1
#define __x86_64 1
#endif

#if __x86_64__
#  include "i386-bsd64.h"
#else
  #include "i386-bsd.h"
#endif

#endif
#include "i386-debug.h"

#define WS_PC() R_EIP(WS(regs))
#define REG_VAL	(unsigned long)

#define SYSCALL_OPS	"\xcd\x80\xcc\x90"	/* int $0x80, int $0x3, nop */
#define SYSCALL_INS	"\xcd\x80"

//extern int instLength(unsigned char *p, int s, int mode);
extern int dislen(unsigned char* opcode0, int limit);

#undef CPUREG_PC
#define CPUREG_PC eip
#undef CPUREG_SP
#define CPUREG_SP esp

#endif
