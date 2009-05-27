/*
 * Copyright (C) 2007, 2008
 *       pancake <youterm.com>
 *       th0rpe <nopcode.org>
 *
 * libps2fd is part of the radare project
 *
 * libps2fd is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * libps2fd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libps2fd; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#if 0
/* TODO: Use this everywhere! */
#if defined(__amd64__) || defined(__x86_64__)
#  define FMT "l"
#else
#  define FMT "ll"
#endif
#endif

#if defined(__amd64__)
#undef __x86_64
#define __x86_64
#endif


#if __x86_64__

#include "../libps2fd.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include "../mem.h"
#include "i386.h"
//#include "../../arch/x86/instcount.c"
#include "../parser.h"
#include "../debug.h"
#include "../flags.h"
#include "x86_64.h"

struct regs_off roff[] = {
	{"rax", R_RAX_OFF},
	{"rbx", R_RBX_OFF},
	{"rcx", R_RCX_OFF},
	{"rdx", R_RDX_OFF},
	{"rsi", R_RSI_OFF},
	{"rdi", R_RDI_OFF},
	{"rip", R_RIP_OFF},
	{"eip", R_RIP_OFF},
	{"rsp", R_RSP_OFF},
	{"rbp", R_RBP_OFF},
	{"eflags", R_RFLAGS_OFF},
	{0, 0}
};

#if 0

     56 #define tR8	4
     57 #define tR9	5
     58 #define tR10	6
     59 #define tR11	7
     60 #define	tR12	8
     61 #define	tR13	9
     62 #define	tR14	10
     63 #define	tR15	11
#endif

u64 arch_syscall(int pid, int sc, ...)
{
	long long ret = (off_t)-1;
#if __linux__
	va_list ap;
	regs_t reg, reg_saved;
	int baksz = 128;
	int status;
	char bak[128];
	long long addr;
	char *arg;
	char *file;

	//printf("Seek pid=%d, fd=%d, addr=%08llx, whence=%d\n", pid,fd, addr, whence);

	/* save old registers */
	debug_getregs(pid, &reg_saved);
	memcpy(&reg, &reg_saved, sizeof(reg));

	/* eip is in the stack now */
	//R_RIP(reg) = R_RSP(reg) - 4;

	/* read stack values */
	debug_read_at(pid, bak, baksz, R_RIP(reg));

	/* set syscall */
	R_RAX(reg) = sc;

	arg = (char*) &sc;
	va_start(ap, arg);
	switch(sc) {
		case SYS_gettid:
			break;
		case SYS_getpid:
			break;
		case SYS_tkill:
			R_RDI(reg) = va_arg(ap, pid_t);
			R_RSI(reg) = va_arg(ap, int);
#if __BSD__
			R_RSP(reg)+=4;
			debug_write_at(pid, &(R_RBX(reg)), 4, R_RSP(reg));
			R_RSP(reg)+=4;
			debug_write_at(pid, &(R_RCX(reg)), 4, R_RSP(reg));
#endif
			break;
		case SYS_open:
			addr = R_RIP(reg)+4;
			file = va_arg(ap, char *);
			debug_write_at(pid, (unsigned char*)file, strlen(file)+4, addr);
			R_RDI(reg) = addr;
			R_RSI(reg) = va_arg(ap, int);
			R_RDX(reg) = 0755; // TODO: Support create flags
			break;
		case SYS_close:
			R_RDI(reg) = va_arg(ap, int);
			break;
		case SYS_dup2:
			R_RDI(reg) = va_arg(ap, int);
			R_RSI(reg) = va_arg(ap, int);
			break;
		case SYS_lseek:
			R_RDI(reg) = va_arg(ap, int);
			R_RSI(reg) = va_arg(ap, off_t);
			R_RDX(reg) = va_arg(ap, int);
			break;
		default:
			printf("ptrace-syscall %d not yet supported\n", sc);
			// XXX return ???
			break;
	}
		
	va_end(ap);

	/* write SYSCALL OPS */
	debug_write_at(pid, (unsigned char*)SYSCALL_OPS64, 4, R_RIP(reg));

	/* set new registers value */
	debug_setregs(pid, &reg);

	/* continue */
	debug_contp(pid);

	/* wait to stop process */
	waitpid(ps.tid, &status, 0);

	if(WIFSTOPPED(status)) {
		/* get new registers value */
		debug_getregs(ps.tid, &reg);

		/* read allocated address */
		ret = (off_t)R_RAX(reg);
		
		if (((long long)ret)<0)
			ret=0;
	}

	/* restore memory */
	//debug_write_at(ps.tid, (unsigned char*)bak, baksz, R_RSP(reg_saved) - 4);
	debug_write_at(ps.tid, (unsigned char*)bak, baksz, R_RIP(reg_saved));

	/* restore registers */
	debug_setregs(ps.tid, &reg_saved);
	
#else
	eprintf("not yet for this platform\n");
#endif

	return ret;
}

// NOTE: it is not possible to use read+write watchpoints.
// not implemented on all x86, and needs some CR4 hacking.
int debug_dr(const char *cmd)
{
	const char *ptr = strchr(cmd, ' ');
	off_t addr;
	int reg = -1;

	if (cmd[0]>='0'&&cmd[0]<='3') {
		reg = cmd[0]-'0';
		cmd = cmd+1;
	}

	switch(cmd[0]) {
	case '?':
		printf("Usage: !dr[type] [args]\n");
		printf("  dr                   - show DR registers\n");
		printf("  dr-                  - reset DR registers\n");
		printf("  drr [addr]           - set a read watchpoint\n");
		printf("  drw [addr]           - set a write watchpoint\n");
		printf("  drx [addr]           - set an execution watchpoint\n");
		printf("  dr[0-3][rwx] [addr]  - set a rwx wp at a certain DR reg\n");
		printf("Use addr=0 to undefine a DR watchpoint\n");
		break;
	case '\0':
	case ' ': //list
		dr_list();
		break;
	case '-':
		dr_init();
		break;
	case 'r':
	case 'w': // 
	case 'x': // breakpoint
		if (!ptr) {
			printf("Usage: !drb [address]\n");
			return -1;
		}
		addr = get_math(ptr+1);
		switch(cmd[0]) {
		case 'r':
			arch_set_wp_hw_n(reg, addr, DR_RW_READ);
			break;
		case 'w':
			arch_set_wp_hw_n(reg, addr, DR_RW_WRITE);
			break;
		case 'x':
			arch_set_wp_hw_n(reg, addr, DR_RW_EXECUTE);
			break;
		}
		break;
		
	}
	return 0;
}

int arch_is_jump(u8 *buf)
{
	switch(buf[0]) {
	case 0x75:
	case 0x74:
	case 0xe9:
	case 0x0f:
		return 1;
	}
	return 0;
}

u64 arch_get_entrypoint()
{
	u64 addr;
	debug_read_at(ps.tid, &addr, 4, 0x8048018);
	return (off_t)addr;
}

int arch_jmp(u64 ptr)
{
	regs_t regs;
	int ret = ptrace(PTRACE_GETREGS, ps.tid, NULL, &regs);
	if (ret < 0) return 1;
	R_RIP(regs) = ptr;
	ptrace(PTRACE_SETREGS, ps.tid, NULL, &regs);
	return 0;
}

int arch_dump_registers()
{
	regs_t regs;
	FILE *fd;

	printf("Dumping CPU to cpustate.dump...\n");
        debug_getregs(ps.tid, &regs);

	fd = fopen("cpustate.dump", "w");
	if (fd == NULL) {
		fprintf(stderr, "Cannot open\n");
		return 0;
	}
	fprintf(fd, "rax 0x%llx\n", (long long)R_RAX(regs));
	fprintf(fd, "rbx 0x%llx\n", (long long)R_RBX(regs));
	fprintf(fd, "rcx 0x%llx\n", (long long)R_RCX(regs));
	fprintf(fd, "rdx 0x%llx\n", (long long)R_RDX(regs));
	fprintf(fd, "rbp 0x%llx\n", (long long)R_RBP(regs));
	fprintf(fd, "rsi 0x%llx\n", (long long)R_RSI(regs));
	fprintf(fd, "rdi 0x%llx\n", (long long)R_RDI(regs));
	fprintf(fd, "rip 0x%llx\n", (long long)R_RIP(regs));
	fprintf(fd, "rsp 0x%llx\n", (long long)R_RSP(regs));
	fprintf(fd, "eflags 0x%llx\n", (long long)R_RFLAGS(regs));
	/* r8-r15 */
	fprintf(fd, "r8 0x%llx\n", (long)R_R8(regs));
	fprintf(fd, "r9 0x%llx\n", (long)R_R9(regs));
	fprintf(fd, "r10 0x%llx\n", (long)R_R10(regs));
	fprintf(fd, "r11 0x%llx\n", (long)R_R11(regs));
	fprintf(fd, "r12 0x%llx\n", (long)R_R12(regs));
	fprintf(fd, "r13 0x%llx\n", (long)R_R13(regs));
	fprintf(fd, "r14 0x%llx\n", (long)R_R14(regs));
	fprintf(fd, "r15 0x%llx\n", (long)R_R15(regs));
	fclose(fd);

#if 0
	// TODO: show file date
	asm ("movl %%eax, %0;" : "=r" ( eax ));
	asm ("movl %%ebx, %0;" : "=r" ( ebx ));
	asm ("movl %%ecx, %0;" : "=r" ( ecx ));
	asm ("movl %%edx, %0;" : "=r" ( edx ));
#endif
	return 1;
}

int arch_opcode_size()
{
	// XXX TODO: read from current seek, etc...
	//return instLength(unsigned char *p, int s, int mode);
	return 0; //this is for compiler warning
}

int arch_restore_registers()
{
	FILE *fd;
	char buf[1024];
	char reg[10];
	long long val;
	regs_t regs;

	printf("Restoring CPU from cpustate.dump...\n");
        debug_getregs(ps.tid, &regs);

	// TODO: show file date
	fd = fopen("cpustate.dump", "r");
	if (fd == NULL) {
		fprintf(stderr, "Cannot open cpustate.dump\n");
		return 0;
	}

	while(!feof(fd)) {
		fgets(buf, 1023, fd);
		if (feof(fd)) break;
		sscanf(buf, "%3s 0x%08x", reg, (unsigned int*) &val);
		//printf("	case %d: // %s \n", ( reg[0] + (reg[1]<<8) + (reg[2]<<16) ), reg);
		switch( reg[0] + (reg[1]<<8) + (reg[2]<<16) ) {
		case 7889253: R_RAX(regs) = val; break;
		case 7889509: R_RBX(regs) = val; break;
		case 7889765: R_RCX(regs) = val; break;
		case 7890021: R_RDX(regs) = val; break;
		case 7365221: R_RBP(regs) = val; break;
		case 6910821: R_RSI(regs) = val; break;
		case 6906981: R_RDI(regs) = val; break;
		case 7367013: R_RIP(regs) = val; break;
		case 7369573: R_RSP(regs) = val; break;
/* TODO: add R8-R15 registers */
		case 7104101: R_RFLAGS(regs) = val; break;
		}
	}
	fclose(fd);

	debug_setregs(ps.tid, &regs);

	return 1;
}

int arch_inject(unsigned char *data, int size)
{
	regs_t regs;
        int ret = debug_getregs(ps.tid, &regs);
	if (ret < 0) return 1;
	//ptrace_read_at(regs.eip = ptr;
	//ptrace(PTRACE_SETREGS, ps.tid, NULL, &regs);
	return 0;
}

u64 arch_pc()
{
	regs_t regs;
	debug_getregs(ps.tid, &regs);
	return (off_t)R_RIP(regs);
}

int arch_backtrace()
{
	/*
	%ebp points to the old ebp var
	%ebp+4 points to ret
	*/
	int ret, i;
	u64 ptr;
	u64 ebp2;
	regs_t regs;
	unsigned char buf[4];

	if (ps.opened == 0)
		return 0;

	ret = ptrace(PTRACE_GETREGS, ps.tid, NULL, &regs);
	if (ret < 0)
		return 1;

	debug_read_at(ps.tid, &buf, 4, R_RIP(regs));
	if (!memcmp(buf, "\x55\x89\xe5", 3)
	||  !memcmp(buf, "\x89\xe5\x57", 3)) { /* push %ebp ; mov %esp, %ebp */
		debug_read_at(ps.tid, &ptr, 4, R_RSP(regs));
		cons_printf("#0 0x%08llx %s", (u64)ptr, flag_name_by_offset((off_t)ptr));
		R_RBP(regs) = ptr;
	}

	for(i=0;i<10;i++) {
		debug_read_at(ps.tid, &ebp2, 4, R_RBP(regs));
		debug_read_at(ps.tid, &ptr, 4, R_RBP(regs)+4);
		if (ptr == 0x0 || R_RBP(regs) == 0x0) break;
		cons_printf("#%d 0x%08llx %s\n", i, (u64)ptr, flag_name_by_offset((off_t)ptr));
		R_RBP(regs) = ebp2;
	}
	return i;
}

void dump_eflags64(const int eflags)
{
#define EFL(x,y,z) eflags & (1 << x) ? y : z
    cons_printf("%c%c%c%c%c%c%c%c%c%c%c ",
	EFL(0,'C','c'), EFL(2,'P','p'), EFL(4,'A','a'), 
	EFL(6,'Z','z'), EFL(7,'S','s'), EFL(8,'T','t'),
	EFL(9,'I','i'), EFL(10,'D','d'), EFL(11,'O','o'),
	EFL(16,'R','r'), ((eflags >> 12) & 3) + '0');

#define EF(x,y) eflags & (1 << x) ? y : ""
    cons_printf("(%s%s%s%s%s%s%s%s%s%s)\n",
	EF(0,"C"), EF(2,"P"), EF(4,"A"), EF(6,"Z"),
	EF(7,"S"), EF(8,"T"), EF(9,"I"), EF(10,"D"),
	EF(11,"O"), EF(16,"R"));
}

int arch_ret()
{
	int ret;
	regs_t regs;

	if (ps.opened == 0)
		return 0;

	debug_getregs(ps.tid, &regs);
	if (ret < 0)
		return 1;
	debug_read_at(ps.tid, &R_RIP(regs), 4, R_RSP(regs));
	R_RSP(regs)+=4;
	debug_setregs(ps.tid, &regs);

	return 0;
}

int arch_call(const char *arg)
{
	int ret;
	regs_t regs;
	u64 addr;

	if (ps.opened == 0)
		return 0;

	addr = get_offset(arg);
	ret = debug_getregs(ps.tid, &regs);
	if (ret < 0)
		return 1;
	R_RSP(regs)-=4;
	debug_write_at(ps.tid, (unsigned char*)&R_RIP(regs), 4, R_RSP(regs));
	if (arg[0]=='+')
		R_RIP(regs) += addr;
	else
		if (arg[0]=='-')
			R_RIP(regs) -= addr;
		else
			R_RIP(regs) = addr;
	ret = debug_setregs(ps.tid, &regs);

	return 0;
}


// NO RAD FOR FPREGS (only 32&64 bit vars)
int arch_print_fpregisters(int rad, const char *mask)
{
	int i, ret;
#if __linux__
	struct user_fpregs_struct regs;
#else
	struct fpreg regs;
#endif

	if (ps.opened == 0)
		return 1;
#if __linux__
#warning THIS CODE IS BUGGY! LINUX AND GNU SUX A LOT
	ret = ptrace(PTRACE_GETFPREGS, ps.tid, NULL, &regs);

	cons_printf(" cwd = 0x%04lx  ; control   ", regs.cwd);
	cons_printf(" swd = 0x%04lx  ; status\n", regs.swd);
	cons_printf(" ftw = 0x%04lx              ", regs.ftw); /* For The Winners */
	cons_printf(" fop = 0x%04lx\n", regs.fop);
	cons_printf(" rip = 0x%016llx  ", regs.rip);
	cons_printf(" rdp = 0x%016llx\n", regs.rdp);
	cons_printf(" mxcsr = 0x%08lx        ", regs.mxcsr);
	cons_printf(" mxcr_mask = 0x%08lx\n", regs.mxcr_mask);

#define FADDR ((double*)&regs.st_space[i*4])

	for(i=0;i<8;i++) {
		u16 *a = (u16*)&regs.xmm_space;
		a = a + (i * 4);

		cons_printf(" mm%d = %04x %04x %04x %04x    ", i, (int)a[0], (int)a[1], (int)a[2], (int)a[3]);
		cons_printf(" st%d = %lg (0x%08llx)\n", i, *FADDR, *FADDR);
	}

#else
	ret = ptrace(PTRACE_GETFPREGS, ps.tid, &regs, sizeof(regs_t));
	for(i=0;i<8;i++)
#if __NetBSD__
		cons_printf("  st%d %f\n", i, regs.__data[i]); // XXX Fill this in with real info.
#else
#if __linux__
		cons_printf("  st%d %f\n", i, regs.fpr_env[i]);
#endif
#endif
#endif

	return ret;
}

#if __linux__
/* syscall-linux */
struct syscall_t {
  const char *name;
  int num;
  int args;
} syscalls_linux_x86_64[] = {
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
  { "access", 33, 2},
  { "dup", 41, 2},
  { "brk", 45, 1},
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
#endif

#if __NetBSD__
/* syscall-netbsd */
struct syscall_t {
  char *name;
  int num;
  int args;
} syscalls_netbsd_x86_64[] = {
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
#endif

#if 0
int arch_print_syscall()
{
#if __linux__
	struct syscall_t *ptr = (struct syscall_t*) &syscalls_linux_x86_64;
#endif
#if __NetBSD__
	struct syscall_t *ptr = &syscalls_netbsd_x86_64;
#endif

#if __linux__ || __NetBSD__
	int i,j,ret;
	regs_t regs;

	ret = ptrace(PTRACE_GETREGS, ps.tid, NULL, &regs);
	if (ret < 0) {
		perror("getregs");
		return -1;
	}
	
	//XXX why are there 3 parameters?
	printf("0x%08llx syscall(%d) ", R_RIP(regs),
		(unsigned int)R_RAX(regs));

	for(i=0;ptr[i].num;i++) {
		if (R_RAX(regs) == ptr[i].num) {
			cons_printf("%s ( ", ptr[i].name);
			j = ptr[i].args;
			if (j>0) cons_printf("0x%08x ", R_RBX(regs));
			if (j>1) cons_printf("0x%08x ", R_RCX(regs));
			if (j>2) cons_printf("0x%08x ", R_RDX(regs));
			if (j>3) cons_printf("0x%08x ", R_RSI(regs));
			if (j>4) cons_printf("0x%08x ", R_RDI(regs));
			break;
		}
	}
	cons_printf(") = 0x%08x\n", R_RAX(regs));
	return (int)R_RAX(regs);
#else
	return -1;
#endif
}
#endif

static regs_t oregs;
static regs_t nregs;
int arch_print_registers(int rad, const char *mask)
{
	int ret;
	regs_t regs;
	int color = config_get("scr.color")?1:0;

	if (ps.opened == 0)
		return 0;

	if (mask[0]=='o') { // orig
		memcpy(&regs, &oregs, sizeof(regs_t));
	} else {
#if __linux__
		ret = ptrace(PTRACE_GETREGS, ps.tid, NULL, &regs);
#else
		ret = ptrace(PTRACE_GETREGS, ps.tid, &regs, sizeof(regs_t));
#endif
		if (ret < 0) {
			perror("getregs");
			return 1;
		}
	}
	
	if (rad) {
		cons_printf("f rax_orig @ 0x%llx\n", (long)R_ORAX(regs));
		cons_printf("f rax @ 0x%llx\n", (long)R_RAX(regs));
		cons_printf("f rbx @ 0x%llx\n", (long)R_RBX(regs));
		cons_printf("f rcx @ 0x%llx\n", (long)R_RCX(regs));
		cons_printf("f rdx @ 0x%llx\n", (long)R_RDX(regs));
		cons_printf("f rbp @ 0x%llx\n", (long)R_RBP(regs));
		cons_printf("f rsi @ 0x%llx\n", (long)R_RSI(regs));
		cons_printf("f rdi @ 0x%llx\n", (long)R_RDI(regs));
		cons_printf("f eip @ 0x%llx\n", (long)R_RIP(regs));
		cons_printf("f rip @ 0x%llx\n", (long)R_RIP(regs));
		cons_printf("f rsp @ 0x%llx\n", (long)R_RSP(regs));
		cons_printf("f esp @ 0x%llx\n", (long)R_RSP(regs));

		/* r8-r15 */
		cons_printf("f r8 @ 0x%llx\n", (long)R_R8(regs));
		cons_printf("f r9 @ 0x%llx\n", (long)R_R9(regs));
		cons_printf("f r10 @ 0x%llx\n", (long)R_R10(regs));
		cons_printf("f r11 @ 0x%llx\n", (long)R_R11(regs));
		cons_printf("f r12 @ 0x%llx\n", (long)R_R12(regs));
		cons_printf("f r13 @ 0x%llx\n", (long)R_R13(regs));
		cons_printf("f r14 @ 0x%llx\n", (long)R_R14(regs));
		cons_printf("f r15 @ 0x%llx\n", (long)R_R15(regs));
	} else {
		if (color) {
			if (R_RAX(regs)!=R_RAX(oregs)) cons_printf("\x1b[35m");
			else cons_printf("\x1b[0m");
			cons_printf(" rax  0x%08llx\x1b[0m", (long long)R_RAX(regs));
			if (R_RSI(regs)!=R_RSI(oregs)) cons_printf("\x1b[35m");
			cons_printf("    rsi  0x%08llx\x1b[0m", (long long)R_RSI(regs));
			if (R_RIP(regs)!=R_RIP(oregs)) cons_printf("\x1b[35m");
			cons_printf("    rip    0x%08llx\x1b[0m\n",  (u64) R_RIP(regs));
			if (R_RBX(regs)!=R_RBX(oregs)) cons_printf("\x1b[35m");
			cons_printf(" rbx  0x%08llx\x1b[0m", (long long)R_RBX(regs));
			if (R_RDI(regs)!=R_RDI(oregs)) cons_printf("\x1b[35m");
			cons_printf("    rdi  0x%08llx\x1b[0m", (long long)R_RDI(regs));
			if (R_ORAX(regs)!=R_ORAX(oregs)) cons_printf("\x1b[35m");
			cons_printf("    orax   0x%08llx\x1b[0m\n",   (long long)R_ORAX(regs));
			if (R_RCX(regs)!=R_RCX(oregs)) cons_printf("\x1b[35m");
			cons_printf(" rcx  0x%08llx\x1b[0m", (long long)R_RCX(regs));
			if (R_RSP(regs)!=R_RSP(oregs)) printf("\x1b[35m");
			cons_printf("    rsp  0x%08llx\x1b[0m", (long long)R_RSP(regs));
			if (R_RFLAGS(regs)!=R_RFLAGS(oregs)) cons_printf("\x1b[35m");
			//cons_printf("    eflags 0x%04x  ", R_RFLAGS(regs));
			cons_printf("    rflags 0x%04llx  \n", (long long)R_RFLAGS(regs));
			if (R_RDX(regs)!=R_RDX(oregs)) printf("\x1b[35m");
			cons_printf(" rdx  0x%08llx\x1b[0m", (long long)R_RDX(regs));
			if (R_RBP(regs)!=R_RBP(oregs)) printf("\x1b[35m");
			cons_printf("    rbp  0x%08llx\x1b[0m    ",(long long) R_RBP(regs));
			dump_eflags64(R_RFLAGS(regs));
			if (R_R8(regs)!=R_R8(oregs)) printf("\x1b[35m");
			cons_printf("  r8 0x%08llx     ",(u64)R_R8(regs)); 
			if (R_R11(regs)!=R_R11(oregs)) printf("\x1b[35m");
			cons_printf("r11 0x%08llx    ",(u64)R_R11(regs)); 
			if (R_R14(regs)!=R_R14(oregs)) printf("\x1b[35m");
			cons_printf("r14 0x%08llx\n", (u64)R_R14(regs));
			if (R_R9(regs)!=R_R9(oregs)) printf("\x1b[35m");
			cons_printf("  r9 0x%08llx     ",(u64)R_R9(regs)); 
			if (R_R12(regs)!=R_R12(oregs)) printf("\x1b[35m");
			cons_printf("r12 0x%08llx    ",(u64)R_R12(regs)); 
			if (R_R15(regs)!=R_R15(oregs)) printf("\x1b[35m");
			cons_printf("r15 0x%08llx\n", (u64)R_R15(regs));
			if (R_R10(regs)!=R_R10(oregs)) printf("\x1b[35m");
			cons_printf(" r10 0x%08llx     ", (u64)R_R10(regs));
			if (R_R13(regs)!=R_R13(oregs)) printf("\x1b[35m");
			cons_printf("r13 0x%08llx\n", (u64)R_R13(regs));
			
			fflush(stdout);
//#if __linux__
//		printf("  cs: 0x%04x   ds: 0x%04x   fs: 0x%04x   gs: 0x%04x\n", regs.cs, regs.ds, regs.fs, regs.gs);
//#endif
		} else {
			cons_printf(" rax  0x%08llx    rsi  0x%08llx    rip    0x%08llx\n",   (u64)R_RAX(regs), (u64)R_RSI(regs), (u64)R_RIP(regs));
			cons_printf(" rbx  0x%08llx    rdi  0x%08llx    orax   0x%08llx\n",   (u64)R_RBX(regs), R_RDI(regs), (u64)R_ORAX(regs));
			cons_printf(" rcx  0x%08llx    rsp  0x%08llx    rflags 0x%04x\n",   (u64)R_RCX(regs), (u64)R_RSP(regs), (u64)R_RFLAGS(regs));
			cons_printf(" rdx  0x%08llx    rbp  0x%08llx    ", (u64)R_RDX(regs), (u64)R_RBP(regs));
			dump_eflags64(R_RFLAGS(regs));

			/* r8-r15 */
			cons_printf("  r8 0x%08llx     r11 0x%08llx    r14 0x%08llx\n", (u64)R_R8(regs), (u64)R_R11(regs), (u64)R_R14(regs));
			cons_printf("  r9 0x%08llx     r12 0x%08llx    r15 0x%08llx\n", (u64)R_R9(regs), (u64)R_R12(regs), (u64)R_R15(regs));
			cons_printf(" r10 0x%08llx     r13 0x%08llx\n", (u64)R_R10(regs), (u64)R_R13(regs));
		}

		if (memcmp(&nregs,&regs, sizeof(regs_t))) {
			memcpy(&oregs, &nregs, sizeof(regs_t));
			memcpy(&nregs, &regs, sizeof(regs_t));
		} else {
			memcpy(&nregs, &regs, sizeof(regs_t));
		}
	}

	return 0;
}

u64 get_value(const char *str)
{
	long long tmp;

	if (str[0]&&str[1]=='x')
		sscanf(str, "0x%llx",  (u64 *)&tmp);
	else	tmp = atoll(str);
	return tmp;
}

int arch_set_register(const char *reg, const char *value)
{
	int ret;
	regs_t regs;

	if (ps.opened == 0)
		return 0;

#if __linux__
	ret = ptrace(PTRACE_GETREGS, ps.tid, NULL, &regs);
#else
	ret = ptrace(PTRACE_GETREGS, ps.tid, &regs, sizeof(regs_t));
#endif
	if (ret < 0) return 1;
	
	if (!strcmp(reg, "rax"))
		R_RAX(regs) = get_value(value);
	else if (!strcmp(reg, "rbx"))
		R_RBX(regs) = get_value(value);
	else if (!strcmp(reg, "rcx"))
		R_RCX(regs) = get_value(value);
	else if (!strcmp(reg, "rdx"))
		R_RDX(regs) = get_value(value);
	else if (!strcmp(reg, "rbp"))
		R_RBP(regs) = get_value(value);
	else if (!strcmp(reg, "rsp"))
		R_RSP(regs) = get_value(value);
	else if (!strcmp(reg, "rsi"))
		R_RSI(regs) = get_value(value);
	else if (!strcmp(reg, "rdi"))
		R_RDI(regs) = get_value(value);
	else if (!strcmp(reg, "rip"))
		R_RIP(regs) = get_value(value);
#warning TODO: Add !set r8, r9, ....

#if __linux__
	else if (!strcmp(reg, "eflags")) {
		int i, foo = 0;
		for(i=0;value[i];i++) {
			switch(value[i]) {
			case '+': foo = regs.eflags; break;
			case 'c': case 'C': foo|=(1<<0); break;
			case 'p': case 'P': foo|=(1<<2); break;
			case 'a': case 'A': foo|=(1<<4); break;
			case 'z': case 'Z': foo|=(1<<6); break;
			case 's': case 'S': foo|=(1<<7); break;
			case 't': case 'T': foo|=(1<<8); break;
			case 'i': case 'I': foo|=(1<<9); break;
			case 'd': case 'D': foo|=(1<<10); break;
			case 'o': case 'O': foo|=(1<<11); break;
			case 'r': case 'R': foo|=(1<<16); break;
			}
		}
		regs.eflags= foo;
	}
#endif
	else {
		eprintf("Valid registers:\n rax, rbx, rcx, rdx, rsi, rdi, rbp, rsp, rflags\n");
		return 1;
	}

#if __linux__
	ret = ptrace(PTRACE_SETREGS, ps.tid, NULL, &regs);
#else
	ret = ptrace(PTRACE_SETREGS, ps.tid, &regs, sizeof(regs_t));
#endif
	return 0;
}


int arch_continue()
{
	regs_t regs;
#if __linux__
	ptrace(PTRACE_GETREGS, ps.tid, NULL, &regs);
#else
	ptrace(PTRACE_GETREGS, ps.tid, &regs, sizeof(regs_t));
#endif
	return ptrace(PTRACE_CONT, ps.tid, R_RIP(regs), 0);
}

//void *arch_mmap(int fd, int size, off_t addr) //int *rsize)
u64 arch_mmap(int fd, int size, u64 addr)
{
#if 0
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define STDOUT	1

void main(void) {
	char file[]="mmap.s";
	char *mappedptr;
	int fd,filelen;

	fd=fopen(file, O_RDONLY);
	filelen=lseek(fd,0,SEEK_END);
	mappedptr=mmap(NULL,filelen,PROT_READ,MAP_SHARED,fd,0);
	write(STDOUT, mappedptr, filelen);
	munmap(mappedptr, filelen);
	close(fd);
}

;-----------------
	push	%ebp
	movl	%esp,%ebp
	subl	$24,%esp

//	open($file, $O_RDONLY);

	movl	$fd,%ebx	// save fd
	movl	%eax,(%ebx)

//	lseek($fd,0,$SEEK_END);

	movl	$filelen,%ebx	// save file length
	movl	%eax,(%ebx)

	xorl	%edx,%edx

//	mmap(NULL,$filelen,PROT_READ,MAP_SHARED,$fd,0);
	movl	%edx,(%esp)
	movl	%eax,4(%esp)	// file length still in %eax
	movl	$PROT_READ,8(%esp)
	movl	$MAP_SHARED,12(%esp)
	movl	$fd,%ebx	// load file descriptor
	movl	(%ebx),%eax
	movl	%eax,16(%esp)
	movl	%edx,20(%esp)
	movl	$SYS_mmap,%eax
	movl	%esp,%ebx
	int	$0x80

	movl	$mappedptr,%ebx	// save ptr
	movl	%eax,(%ebx)
		
// 	write($stdout, $mappedptr, $filelen);
//	munmap($mappedptr, $filelen);
//	close($fd);
	
	movl	%ebp,%esp
	popl	%ebp

	ret
$
#endif
#ifdef __linux__
	regs_t  reg, reg_saved;
	int     status;
	char    bak[4];
    u64 ret = -1;

	/* save old registers */
	debug_getregs(ps.tid, &reg_saved);
	memcpy(&reg, &reg_saved, sizeof(reg));

	// XXX: FUCK MMAP GOES TO ESP!

	/* mmap call */

	//R_RAX(reg) = 90;    // SYS_mmap
	R_RAX(reg) = 9;    // SYS_mmap
	R_RDI(reg) = addr;  // mmap addr
	R_RSI(reg) = size;  // size
	R_RDX(reg) = 0x7;   // perm
	//R_RDX(reg) = 0x1;   // options
    R_RCX(reg) = 0x2;   // options
	R_R8(reg) = fd;    // fd
	R_R9(reg) = 0;     // offset

	/* write syscall interrupt code */
	//R_RIP(reg) = R_RSP(reg) - 4;

	/* read stack values */
	debug_read_at(ps.tid, bak, 4, R_RIP(reg));

	/* write SYSCALL OPS */
	debug_write_at(ps.tid, (unsigned char*)SYSCALL_OPS64, 4, R_RIP(reg));

	/* set new registers value */
	debug_setregs(ps.tid, &reg);

	/* continue */
	debug_contp(ps.tid);

	/* wait to stop process */
	waitpid(ps.tid, &status, 0);

	if(WIFSTOPPED(status)) {
		/* get new registers value */
		debug_getregs(ps.tid, &reg);

		/* read allocated address */
        ret = (addr_t)(R_RAX(reg));

		if (ret == 0) {
			eprintf("oops\n");
			return 0;
		}
	}

	/* restore memory */
	//debug_write_at(ps.tid, (unsigned char*)bak, 4, R_RSP(reg_saved) - 4);
	//debug_write_at(ps.tid, (unsigned char*)bak, 4, R_RIP(reg_saved));

	/* restore registers */
	debug_setregs(ps.tid, &reg_saved);

	return (u64) ret;
#else
	eprintf("Not supported by this OS\n");
	return 0;
#endif
} 

u64 arch_alloc_page(unsigned long size, unsigned long *rsize)
{
#ifdef __linux__
	regs_t  reg, reg_saved;
	int     status;
	char    bak[4];
	void*   ret = (void *)-1;

	/* save old registers */
	debug_getregs(ps.tid, &reg_saved);
	memcpy(&reg, &reg_saved, sizeof(reg));

	/* mmap call */
	R_RAX(reg) = 0xc0;
	R_RSI(reg) = 0x21;
	R_RDI(reg) = 0;
	R_RBP(reg) = 0;
	R_RDX(reg) = 5;
	R_RCX(reg) = (size + PAGE_SIZE - 1) & PAGE_MASK;
	R_RBX(reg) = 0;

	/* write syscall interrupt code */
	R_RIP(reg) = R_RSP(reg) - 4;

	/* read stack values */
	debug_read_at(ps.tid, bak, 4, R_RIP(reg));

	/* write SYSCALL OPS */
	debug_write_at(ps.tid, (unsigned char*)SYSCALL_OPS, 4, R_RIP(reg));

	/* set new registers value */
	debug_setregs(ps.tid, &reg);

	/* continue */
	debug_contp(ps.tid);

	/* set real allocated size */
	*rsize = R_RCX(reg);

	/* wait to stop process */
	waitpid(ps.tid, &status, 0);

	if(WIFSTOPPED(status)) {
		/* get new registers value */
		debug_getregs(ps.tid, &reg);

		/* read allocated address */
		ret = (void *)R_RAX(reg);
	}

	/* restore memory */
	debug_write_at(ps.tid, (unsigned char*)bak, 4, R_RSP(reg_saved) - 4);

	/* restore registers */
	debug_setregs(ps.tid, &reg_saved);

	return (u64) ret;
#else
	eprintf("Not supported by this OS\n");
	return 0;
#endif
} 

u64 arch_get_sighandler(int signum)
{
#ifdef __linux__
	regs_t  reg, reg_saved;
	int     status;
	char    bak[8];
	void*   ret = (void *)-1;

	/* save old registers */
	debug_getregs(ps.tid, &reg_saved);
	memcpy(&reg, &reg_saved, sizeof(reg));

	R_RAX(reg) = 13;
	R_RCX(reg) = 8;
	R_RDX(reg) = R_RSP(reg) - 8;
	R_RSI(reg) = 0;
	R_RDI(reg) = signum;

	/* save memory */
	debug_read_at(ps.tid, bak, 8, R_RDX(reg));

	/* write -1 */
	debug_write_at(ps.tid, (unsigned char*)&ret, 4, R_RDX(reg));

	/* write syscall interrupt code */
	//R_RIP(reg) = R_RSP(reg) - 4;
	debug_write_at(ps.tid, (unsigned char*)SYSCALL_OPS64, 4, R_RIP(reg));

	/* set new registers value */
	debug_setregs(ps.tid, &reg);

	/* continue */
	debug_contp(ps.tid);

	/* wait to stop process */
	waitpid(ps.tid, &status, 0);

	if(WIFSTOPPED(status)) {
		/* read sighandler address */
		//debug_read_at(ps.tid, &ret, 4, R_RSP(reg_saved) - 8);
		debug_read_at(ps.tid, &ret, 8, R_RSP(reg_saved) - 8);
	}

	/* restore memory */
	//debug_write_at(ps.tid, (unsigned char*)bak, 8, R_RSP(reg_saved) - 8);
	debug_write_at(ps.tid, (unsigned char*)bak, 4, R_RIP(reg_saved));

	/* restore registers */
	debug_setregs(ps.tid, &reg_saved);

	return (u64) ret;
#else
	eprintf("Not supported by this OS\n");
	return 0;
#endif
}

// XXX this code demonstrate how buggy is debug_inject
#if 0
void signal_set(int signum, off_t address)
{
	int i;
	unsigned char shellcode[18];
	unsigned int *sign = shellcode+6;
	unsigned int *sigh = shellcode+11;

	/* generate shellcode */
	memcpy(shellcode,
		"\xb8\x30\x00\x00\x00" // mov eax, SYS_signal
		"\xbb\x00\x00\x00\x00" // mov ebx, 0x????????
		"\xb9\x00\x00\x00\x00" // mov ecx, 0x????????
		"\xcd\x80",17);        // int 80h
	*sign = signum;
	*sigh = (unsigned int)address;

	/* nfo */
	fprintf(stderr, "Signal %d to 0x%08llx\nshellcode: ", signum,address);

	debug_inject_buffer(shellcode, 17);
}
#endif

int arch_mprotect(u64 addr, unsigned int size, int perms)
{
#ifdef __linux__
	regs_t  reg, reg_saved;
	int     status;
	char    bak[4];
	int     ret = -1;

	/* save old registers */
	debug_getregs(ps.tid, &reg_saved);
	memcpy(&reg, &reg_saved, sizeof(reg));

	R_RAX(reg) = 10;
	R_RSI(reg) = size;
	R_RDX(reg) = perms;
	R_RDI(reg) = (long long) addr;

	//R_RIP(reg) = R_RSP(reg) - 4;

	/* read stack values */
	debug_read_at(ps.tid, bak, 4, R_RIP(reg));

	/* write syscall interrupt code */
	debug_write_at(ps.tid, (unsigned char*)SYSCALL_OPS64, 4, R_RIP(reg));

	/* set new registers value */
	debug_setregs(ps.tid, &reg);

	/* continue */
	debug_contp(ps.tid);

	/* wait to stop process */
	waitpid(ps.tid, &status, 0);

	if(WIFSTOPPED(status)) {
		/* get new registers value */
		debug_getregs(ps.tid, &reg);

		/* get return code */
		ret = (int) R_RAX(reg);
	}

	/* restore memory */
	//debug_write_at(ps.tid, (unsigned char*)bak, 4, R_RSP(reg_saved) - 4);
	debug_write_at(ps.tid, (unsigned char*)bak, 4, R_RIP(reg_saved));

	/* restore registers */
	debug_setregs(ps.tid, &reg_saved);

	return ret;
#else
	eprintf("Not supported by this OS\n");
	return -1;
#endif
}

u64 arch_set_sighandler(int signum, u64 handler)
{
#ifdef __linux__
	regs_t  reg, reg_saved;
	int     status;
	char    bak[8];
	void*   ret = (void *)-1;

	/* save old registers */
	debug_getregs(ps.tid, &reg_saved);
	memcpy(&reg, &reg_saved, sizeof(reg));

	R_RAX(reg) = 0x30;
	R_RBX(reg) = signum;
	R_RCX(reg) = handler;
	R_RDX(reg) = R_RSP(reg) - 8;

	/* save memory */
	debug_read_at(ps.tid, bak, 8, R_RDX(reg));

	/* write -1 */
	debug_write_at(ps.tid, (unsigned char*)&ret, 4, R_RDX(reg));

	/* write syscall interrupt code */
	R_RIP(reg) = R_RSP(reg) - 4;
	debug_write_at(ps.tid, (unsigned char*)SYSCALL_OPS, 4, R_RIP(reg));

	/* set new registers value */
	debug_setregs(ps.tid, &reg);

	/* continue */
	debug_contp(ps.tid);

	/* wait to stop process */
	waitpid(ps.tid, &status, 0);

	if(WIFSTOPPED(status)) {
		/* read sighandler address */
		debug_read_at(ps.tid, &ret, 4, R_RSP(reg_saved) - 8);
	}

	/* restore memory */
	debug_write_at(ps.tid, (unsigned char*)bak, 8, R_RSP(reg_saved) - 8);
	debug_getregs(ps.tid, &reg);
	if (R_RAX(reg)==0) {
		eprintf("Signal %d handled by 0x%08x\n", signum, handler);
	} else {
		if (handler == 0)
			eprintf("(DEFAULT)\n");
		else
			eprintf("Error\n");
	}

	/* restore registers */
	debug_setregs(ps.tid, &reg_saved);

	return (u64) ret;
#else
	eprintf("Not supported by this OS\n");
	return 0;
#endif
}

/* THIS CODE MUST BE REMOVED */
/* I SHOULD USE THE AOP INTERFACE */
// XXX must handle current seek
// XXX must say if this is a forced jump or a conditional one
int arch_is_jmp(const u8 *cmd, u64 *addr)
{
	switch(cmd[0]) {
	case 0xe9: // jmp
	case 0xea: // far jmp
	case 0xeb: // jmp 8
		*addr = (long long)(cmd+1)+5;
		return 5;
	}
	/* conditional jump */
	if (cmd[0]>=0x80&&cmd[0]<=0x8F)
		return 5;
	
	return 0; //XXX this is just for compiler warning
}

int arch_is_call(const char *cmd)
{
	if (cmd[0] == (char)0xe8) // call
		return 5;
	return 0;
}

int arch_is_soft_stepoverable(const unsigned char *opcode)
{
	if (opcode[0]==0xf3) // repz
		return 2;

	if (opcode[0]==0xf2) // repnz
		return 2;
	return 0;
}

int arch_is_stepoverable(const unsigned char *cmd)
{
	if (cmd[0]==(unsigned char)0xff && (cmd[1] >= (unsigned char) 0xd0 && cmd[1] < (unsigned char) 0xdf))
		return 2; // call * reg

	if (cmd[0]==(unsigned char)0xe8) // call
		return 5;
	
	return arch_is_soft_stepoverable(cmd);
}

#warning "FIXME: following code is a party..."
u64 get_ret_sf(u64 rsp, u64 *ret_pos)
{
	u64 pos;
	u64 val;
	unsigned char aux;

	pos = rsp;
	/* read double-words on stack segment */
	while(debug_read_at(ps.tid, &val, sizeof(unsigned long), pos) 
		 ==
		 sizeof(unsigned long)) {

		/* FIXME: check call instruction more correctly */
		/* find call bytecodes */

		if(is_code(val) && (
	     	   (debug_read_at(ps.tid, &aux, sizeof(char), val - 7)
		    == sizeof(char) && aux == 0x65) /* far call */ || 
		   (debug_read_at(ps.tid, &aux, sizeof(char), val - 5)
		    == sizeof(char) && aux == 0xe8) /* call */ ||
		   (debug_read_at(ps.tid, &aux, sizeof(char), val - 3)
		    == sizeof(char) && aux == 0xff) )) /* near call */  {
			//printf("val: 0x%x pos: 0x%x\n", val, pos); 
			*ret_pos = pos;
			return val;
		} 

		pos += sizeof(unsigned long);
	} 

	*ret_pos = 0;
	return 0;
}

void next_sf(struct list_head *list, u64 rsp)
{
	u64 ret_pos, ret;
	struct sf_t  *sf;

	/* get return address of stack frame and return position */
	ret = get_ret_sf(rsp, &ret_pos);

	/* nothing else! */
	if(ret == 0)
		return;

	/* fill stack frame structure */
	sf = (struct sf_t *)malloc(sizeof(struct sf_t));
	if(!sf) {
		perror(":error malloc stack frame");
		return;
	}

	sf->ret_addr = ret;
	sf->ebp = 0;
	sf->vars_sz = ret_pos - rsp;
	sf->sz =  ret_pos - rsp + sizeof(u64);

	/* get next stack frame */
	next_sf(list, ret_pos + sizeof(u64));

	/* add stack frame */
	list_add(&sf->next, list);
}

struct list_head *arch_bt()
{
	struct list_head *bt_list;

	/* initialize backtrace list */
	bt_list = (struct list_head *)malloc(sizeof(struct list_head));
	if(!bt_list) {
		perror(":error malloc backtrace list");
		return NULL;
	}

	INIT_LIST_HEAD(bt_list);

	/* init mapped regions */
	debug_init_maps(1);

	/* get stack frames */
	next_sf(bt_list, R_RSP(WS(regs)));

	return bt_list;
}

int arch_stackanal()
{
	#warning "FIXME: code is missing"
	return 0;
}

u64 get_reg(const char *reg)
{
	int i;

	for(i = 0;  roff[i].reg != 0; i++) 
		if(!strncmp(roff[i].reg, reg, strlen(roff[i].reg)))
			return i;

	return -1;
}

void arch_view_bt(struct list_head *sf)
{
	#warning "FIXME: code is missing"
}

void free_bt(struct list_head *sf)
{
   struct list_head *p, *aux;
   struct sf_t *sf_e;

   p = sf->next;

   while(p && p != sf)
    {
	aux = p->next;

	sf_e = list_entry(p, struct sf_t, next);
        free(sf_e);

        p = aux;
    }

   free(sf);
}

u64 arch_dealloc_page(u64 addr, unsigned long size)
{
	#warning "FIXME: code is missing"
	return 0;
}

int arch_is_fork()
{
        char sc_ins[2];

	#if __linux__
       	/* clone or fork syscalls */
		return (R_ORAX(WS(regs)) == 120 || R_RAX(WS(regs)) == 2) &&
               	 debug_read_at(ps.tid, sc_ins, 2, R_RIP(WS(regs)) - 2) == 2 &&
                 memcmp(sc_ins, SYSCALL_INS, sizeof(SYSCALL_INS) - 1) == 0;
	#else
#warning arch_is_fork() is not implemetned for this platform
		return 0;
	#endif
}

#endif
