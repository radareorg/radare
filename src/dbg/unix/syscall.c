/*
 * Copyright (C) 2008
 *       pancake <youterm.com>
 *
 * radare is part of the radare project
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include "../libps2fd.h"
#include "../../radare.h"
#include <sys/syscall.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#if __x86_64__
#define ARCH_X86_64 1
#endif

struct syscall_t {
  char *name;
  int num;
  int args;
};

/* syscall-linux */
struct syscall_t syscalls_linux_x86[] = {
  { "exit", 1, 1 },
  { "fork", 2, 0 },
  { "read", 3, 3 },
  { "write", 4, 3 },
  { "open", 5, 3 },
  { "close", 6, 1 },
  { "waitpid", 7, 3 },
  { "creat", 8, 2 },
  { "link", 9, 2 },
  { "unlink", 10, 1 },
  { "execve", 11, 3},
  { "chdir", 12, 1},
  { "getpid", 20, 0},
  { "setuid", 23, 1},
  { "getuid", 24, 0},
  { "ptrace", 26, 4},
  { "utime", 30, 2 },
  { "access", 33, 2},
  { "kill", 37,2 },
  { "dup", 41, 2},
  { "brk", 45, 1},
  { "signal", 48, 2},
  { "ioctl", 54, 3 },
  { "mmap", 90, 6},
  { "munmap", 91, 1},
  { "socketcall", 102, 2 },
  { "sigreturn", 119, 1 },
  { "clone", 120, 4 },
  { "mprotect", 125, 3},
  { "rt_sigaction", 174, 3},
  { "rt_sigprocmask", 175, 3},
  { "sysctl", 149, 1 },
  { "mmap2", 192, 6},
  { "fstat64", 197, 2},
  { "fcntl64", 221, 3},
  { "gettid", 224, 0},
  { "set_thread_area", 243, 2},
  { "get_thread_area", 244, 2},
  { "exit_group", 252, 1},
  { NULL, 0, 0 }
};

/* there are LOT of magic syscalls in freebsd...ill really have to look on it */
/* jail, jail_attach, utrace, kqueue->EV_FILTER_PROC->NOTE_FORK, nfsclnt */
/* syscall-freebsd */
struct syscall_t syscalls_freebsd_x86[] = {
  { "syscall", 0, 4 },
  { "exit", 1, 1 },
  { "fork", 2, 0 },
  { "read", 3, 3 },
  { "write", 4, 3 },
  { "open", 5, 3 },
  { "close", 6, 1 },
  { "wait4", 7, 3 },
  { "old_creat", 8, 2 },
  { "link", 9, 2 },
  { "unlink", 10, 1 },
  { "old_execve", 11, 3},
  { "chdir", 12, 1},
  { "fchdir", 13, 1},
  { "mknod", 14, 1},
  { "chmod", 15, 1},
  { "chown", 16, 1},
  { "break", 17, 1},
  { "old_fsstat", 18, 1},
  { "old_lseek", 19, 1},
  { "getpid", 20, 0},
  { "mount", 21, 0},
  { "unmount", 22, 0},
  { "setuid", 23, 1},
  { "getuid", 24, 0},
  { "ptrace", 26, 4},
  { "recvmsg", 27, 4},
  { "sendmsg", 28, 4},
  { "recvfrom", 29, 4},
  { "accept", 30, 4},
  { "getpeername", 31, 3},
  { "getsockname", 32, 3},
  { "access", 33, 2},
  { "chflags", 34, 2},
  { "fchflags", 35, 2},
  { "sync", 36, 0},
  { "kill", 37, 2},
  { "dup", 41, 2},
  { "pipe", 42, 1},
  { "getepid", 43, 0},
  { "profil", 44, 4}, /* LOLSOME! THIS IS LIKE linux oprofile! we need to hack on it! */
  { "ktrace", 45, 4},
  { "getgid", 47, 0},
  { "signal", 48, 2},
  { "getlogin", 49, 0}, /* like getuid but returns a string */
  { "setlogin", 50, 1}, /* like setuid but passing a string */
  { "acct", 51, 1}, /* given a string for file */
  { "sigaltstack", 53, 2}, /* alterate stack context for signals */
  { "ioctl", 54, 3 },
  { "reboot", 55, 1},
  { "revoke", 56, 1}, /* given a string invalidates the access to a file */
  { "symlink", 57, 2},
  { "readlink", 58, 1},
  { "execve", 59, 2},
  { "umask", 60, 1},
  { "chroot", 61, 1},
  { "mmap", 90, 6},
  { "munmap", 91, 1},
  { "socketcall", 102, 2 },
  { "sigreturn", 119, 1 },
  { "clone", 120, 4 },
  { "mprotect", 125, 3},
  { "rt_sigaction", 174, 3},
  { "rt_sigprocmask", 175, 3},
  { "sysctl", 149, 1 },
  { "mmap2", 192, 6},
  { "fstat64", 197, 2},
  { "fcntl64", 221, 3},
  { "gettid", 224, 0},
  { "set_thread_area", 243, 2},
  { "get_thread_area", 244, 2},
  { "exit_group", 252, 1},
  { NULL, 0, 0 }
};

/* syscall-netbsd */
struct syscall_t syscalls_netbsd_x86[] = {
  { "syscall", 0, 4 },
  { "exit", 1, 1 },
  { "fork", 2, 0 },
  { "read", 3, 3 },
  { "write", 4, 3 },
  { "open", 5, 3 },
  { "close", 6, 1 },
  { "wait4", 7, 3 },
  { "compat_43_ocreat", 8, 2 },
  { "link", 9, 2 },
  { "unlink", 10, 1 },
  //{ "execve", 11, 3},
  { "chdir", 12, 1},
  { "fchdir", 13, 1},
  { "mknod", 14, 1},
  { "chmod", 15, 1},
  { "chown", 16, 1},
  { "break", 17, 1},
  { "getpid", 20, 0},
  { "mount", 21, 0},
  { "unmount", 22, 0},
  { "setuid", 23, 1},
  { "getuid", 24, 0},
  { "ptrace", 26, 4},
  { "recvmsg", 27, 4},
  { "sendmsg", 28, 4},
  { "recvfrom", 29, 4},
  { "accept", 30, 4},
  { "access", 33, 2},
  { "dup", 41, 2},
  { "ktrace", 45, 1},
  { "signal", 48, 2},
  { "utime", 30, 2 },
  { "kill", 37,2 },
  { "ioctl", 54, 3 },
  { "mmap", 90, 6},
  { "munmap", 91, 1},
  { "socketcall", 102, 2 },
  { "sigreturn", 119, 1 },
  { "clone", 120, 4 },
  { "mprotect", 125, 3},
  { "rt_sigaction", 174, 3},
  { "rt_sigprocmask", 175, 3},
  { "sysctl", 149, 1 },
  { "mmap2", 192, 6},
  { "fstat64", 197, 2},
  { "fcntl64", 221, 3},
  { "gettid", 224, 0},
  { "set_thread_area", 243, 2},
  { "get_thread_area", 244, 2},
  { "exit_group", 252, 1},
  { NULL, 0, 0 }
};

#if 0
// TODO: use it!
int arch_print_syscall_mips64()
{
	unsigned int sc;
	int i,j;

	/* read 4 previous bytes to ARM_pc and get syscall number from there */
	debug_read_at(ps.tid, &sc, 4, arch_pc()-4);
	sc<<=8; // drop opcode
	for(i=0;syscalls_linux_mips64[i].num;i++) {
		if (sc == 0x900000 + syscalls_linux_mips64[i].num) {
			printf("%s ( ", syscalls_linux_mips64[i].name);
			j = syscalls_linux_mips64[i].args;
			/*
			if (j>0) printf("0x%08x ", R_EBX(regs));
			if (j>1) printf("0x%08x ", R_ECX(regs));
			if (j>2) printf("0x%08x ", R_EDX(regs));
			if (j>3) printf("0x%08x ", R_ESI(regs));
			if (j>4) printf("0x%08x ", R_EDI(regs));
			*/
			break;
		}
	}
	return sc-0x900000;
}
#endif

int arch_print_syscall()
{
#if __ARM__
	return arch_arm_print_syscall();
#elif __linux__ || __NetBSD__ || __FreeBSD__ || __OpenBSD__
	int i,j,ret;
	regs_t regs;
	struct syscall_t *sysptr;

	if (!strcmp(config_get("asm.os"), "linux"))
		sysptr = &syscalls_linux_x86;
	else
	if (!strcmp(config_get("asm.os"), "netbsd"))
		sysptr = &syscalls_netbsd_x86;
	else
	if (!strcmp(config_get("asm.os"), "openbsd"))
		sysptr = &syscalls_netbsd_x86; // XXX
	else
	if (!strcmp(config_get("asm.os"), "freebsd"))
		sysptr = &syscalls_freebsd_x86;
	else {
		eprintf("Unknown/unhandled OS in asm.os for arch_print_syscall()\n");
		return -1;
	}

	ret = debug_getregs(ps.tid, &regs);
	if (ret < 0) {
		perror("getregs");
		return -1;
	}
#if __x86_64__
	cons_printf("0x%08llx syscall(%d) ", (u64)R_RIP(regs), (int)R_ORAX(regs));
	
	for(i=0;sysptr[i].num;i++) {
		if(R_ORAX(regs) == sysptr[i].num) {
			cons_printf("%s ( ", sysptr[i].name);
			j = sysptr[i].args;
			if (j>0) cons_printf("0x%08llx ", (u64) R_RBX(regs));
			if (j>1) cons_printf("0x%08llx ", (u64) R_RCX(regs));
			if (j>2) cons_printf("0x%08llx ", (u64) R_RDX(regs));
			if (j>3) cons_printf("0x%08llx ", (u64) R_RSI(regs));
			if (j>4) cons_printf("0x%08llx ", (u64) R_RDI(regs));
			break;
		}
	}

	cons_printf(") = 0x%08llx\n", R_RAX(regs));
	return (int)R_ORAX(regs);
#elif __i386__
	cons_printf("0x%08x syscall(%d) ", R_EIP(regs), R_OEAX(regs), R_EAX(regs));

	for(i=0;sysptr[i].num;i++) {
		if (R_OEAX(regs) == sysptr[i].num) {
			cons_printf("%s ( ", sysptr[i].name);
			j = sysptr[i].args;
			if (j>0) cons_printf("0x%08x ", R_EBX(regs));
			if (j>1) cons_printf("0x%08x ", R_ECX(regs));
			if (j>2) cons_printf("0x%08x ", R_EDX(regs));
			if (j>3) cons_printf("0x%08x ", R_ESI(regs));
			if (j>4) cons_printf("0x%08x ", R_EDI(regs));
			break;
		}
	}
	cons_printf(") = 0x%08x\n", R_EAX(regs));
	return (int)R_OEAX(regs);
#else
#warning Unknown arch for arch_print_syscall
	eprintf("arch_print_syscall: not implemented for this platform\n");
#endif
#endif
	return -1;
}
