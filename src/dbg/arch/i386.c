/*
 * Copyright (C) 2007, 2008
 *       pancake <youterm.com>
 *       th0rpe <nopcode.org>
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

//#if __i386__
#include "../libps2fd.h"
#include "../../radare.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>

#include <unistd.h>
#if __WINDOWS__
/* do nothing */
#else
#include <sys/syscall.h>
#include <sys/wait.h>
//#include <sys/ptrace.h>
#endif

#if __linux__
#include <sys/procfs.h>
#endif
#include "../debug.h"
#include "../mem.h"
#include "i386.h"
#include "../../arch/x86/instcount.c"
#include "../parser.h"
#include "arch.h"

struct regs_off roff[] = {
	{"eax", R_EAX_OFF},
	{"ebx", R_EBX_OFF},
	{"ecx", R_ECX_OFF},
	{"edx", R_EDX_OFF},
	{"esi", R_ESI_OFF},
	{"edi", R_EDI_OFF},
	{"eip", R_EIP_OFF},

#if __WINDOWS__
	{"dr0", R_DR0_OFF},
	{"dr1", R_DR1_OFF},
	{"dr2", R_DR2_OFF},
	{"dr3", R_DR3_OFF},
	{"dr6", R_DR6_OFF},
	{"dr7", R_DR7_OFF},
#endif
	{"eflags", R_EFLAGS_OFF},
	{0, 0}
};


/* return register offset */
unsigned long get_reg(char *reg)
{
	int i;

	for(i = 0;  roff[i].reg != 0; i++) 
		if(!strncmp(roff[i].reg, reg, strlen(roff[i].reg)))
			return i;

	return -1;
}

long long arch_syscall(int pid, int sc, ...)
{
        long long ret = (addr_t)-1;
#if __linux__
	va_list ap;
        regs_t   reg, reg_saved;
	int baksz = 128;
        int     status;
	char	bak[128];
	long long addr;
	char *arg;
	char *file;

	//printf("Seek pid=%d, fd=%d, addr=%08llx, whence=%d\n",
	//	pid,fd, addr, whence);

	/* save old registers */
        debug_getregs(pid, (regs_t *)&reg_saved);
	memcpy(&reg, &reg_saved, sizeof(reg));

        /* eip is in the stack now */
        R_EIP(reg) = R_ESP(reg) - 4;

	/* read stack values */
        debug_read_at(pid, bak, baksz, R_EIP(reg));

	/* set syscall */
        R_EAX(reg) = sc;

	arg = (char *)&sc;
	va_start(ap, sc);
	switch(sc) {
	case SYS_gettid:
		break;
	case SYS_getpid:
		break;
	case SYS_tkill:
		R_EBX(reg) = va_arg(ap, pid_t);
		R_ECX(reg) = va_arg(ap, int);
#if BSD
		R_ESP(reg)+=4;
		debug_write_at(pid, &(R_EBX(reg)), 4, R_ESP(reg));
		R_ESP(reg)+=4;
		debug_write_at(pid, &(R_ECX(reg)), 4, R_ESP(reg));
#endif
		break;
	case SYS_open:
		addr = R_EIP(reg)+4;
		file = va_arg(ap, char *);
		debug_write_at(pid, file, strlen(file)+4, addr);
		R_EBX(reg) = addr;
		R_ECX(reg) = va_arg(ap, int);
		R_EDX(reg) = 0755; // TODO: Support create flags
		break;
	case SYS_close:
		R_EBX(reg) = va_arg(ap, int);
		break;
	case SYS_dup2:
		R_EBX(reg) = va_arg(ap, int);
		R_ECX(reg) = va_arg(ap, int);
		break;
	case SYS_lseek:
		R_EBX(reg) = va_arg(ap, int);
		R_ECX(reg) = va_arg(ap, addr_t);
		R_EDX(reg) = va_arg(ap, int);
		break;
	default:
		eprintf("ptrace-syscall %d not yet supported\n", sc);
		// XXX return ???
		break;
	}
	va_end(ap);

	/* write SYSCALL OPS */
	debug_write_at(pid, (long *)SYSCALL_OPS, 4, R_EIP(reg));

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
        	ret = (addr_t)R_EAX(reg);
		if (((long long)ret)<0) ret=0;
	}

        /* restore memory */
	debug_write_at(ps.tid, (long *)bak, baksz, R_ESP(reg_saved) - 4);

        /* restore registers */
        debug_setregs(ps.tid, &reg_saved);
#else
	eprintf("not yet for this platform\n");
#endif

	return ret;
}

// NOTE: it is not possible to use read+write watchpoints.
// not implemented on all x86, and needs some CR4 hacking.
int debug_dr(char *cmd)
{
	char *ptr = strchr(cmd, ' ');
	addr_t addr;
	int reg = -1;

	if (cmd[0]>='0'&&cmd[0]<='3') {
		reg = cmd[0]-'0';
		cmd = cmd+1;
	}

	switch(cmd[0]) {
	case '?':
		eprintf("Usage: !dr[type] [args]\n"
		"  dr                   - show DR registers\n"
		"  dr-                  - reset DR registers\n"
		"  drr [addr]           - set a read watchpoint\n"
		"  drw [addr]           - set a write watchpoint\n"
		"  drx [addr]           - set an execution watchpoint\n"
		"  dr[0-3][rwx] [addr]  - set a rwx wp at a certain DR reg\n"
		"Use addr=0 to undefine a DR watchpoint\n");
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
			eprintf("Usage: !drb [address]\n");
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

int arch_is_jump(unsigned char *buf)
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

addr_t arch_get_entrypoint()
{
	unsigned long long addr;
	debug_read_at(ps.tid, &addr, 4, 0x8048018);
	return (addr_t)addr;
}

int arch_jmp(addr_t ptr)
{
	regs_t regs;
	int ret;

        ret = debug_getregs(ps.tid, &regs);

	/* XXX broken */
	// THINK: remove or fix ? //
	if (ret < 0)
		return 1;
	R_EIP(regs) = ptr;
	debug_setregs(ps.tid, &regs);
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
	fprintf(fd, "eax 0x%08x\n", (uint)R_EAX(regs));
	fprintf(fd, "ebx 0x%08x\n", (uint)R_EBX(regs));
	fprintf(fd, "ecx 0x%08x\n", (uint)R_ECX(regs));
	fprintf(fd, "edx 0x%08x\n", (uint)R_EDX(regs));
	fprintf(fd, "ebp 0x%08x\n", (uint)R_EBP(regs));
	fprintf(fd, "esi 0x%08x\n", (uint)R_ESI(regs));
	fprintf(fd, "edi 0x%08x\n", (uint)R_EDI(regs));
	fprintf(fd, "eip 0x%08x\n", (uint)R_EIP(regs));
	fprintf(fd, "esp 0x%08x\n", (uint)R_ESP(regs));
	fprintf(fd, "efl 0x%08x\n", (uint)R_EFLAGS(regs));
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

int dislen(unsigned char* opcode0, int limit);
int arch_opcode_size()
{
	return dislen(config.block, config.block_size);
}

int arch_restore_registers()
{
	FILE *fd;
	char buf[1024];
	char reg[10];
	unsigned int val;
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
		sscanf(buf, "%3s 0x%08x", reg, &val);
		//printf("	case %d: // %s \n", ( reg[0] + (reg[1]<<8) + (reg[2]<<16) ), reg);
		switch( reg[0] + (reg[1]<<8) + (reg[2]<<16) ) {
		case 7889253: R_EAX(regs) = val; break;
		case 7889509: R_EBX(regs) = val; break;
		case 7889765: R_ECX(regs) = val; break;
		case 7890021: R_EDX(regs) = val; break;
		case 7365221: R_EBP(regs) = val; break;
		case 6910821: R_ESI(regs) = val; break;
		case 6906981: R_EDI(regs) = val; break;
		case 7367013: R_EIP(regs) = val; break;
		case 7369573: R_ESP(regs) = val; break;
		case 7104101: R_EFLAGS(regs) = val; break;
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
//	ptrace(PTRACE_SETREGS, ps.tid, NULL, &regs);
	return 0;
}

addr_t arch_pc()
{
	regs_t regs;
	debug_getregs(ps.tid, &regs);
	return (addr_t)R_EIP(regs);
}

// XXX make it 
int arch_stackanal()
{
	/*
	%ebp points to the old ebp var
	%ebp+4 points to ret
	*/
	int ret, i,j;
	unsigned long long ptr;
	unsigned long long ebp2;
	regs_t regs;

	if (ps.opened == 0)
		return 0;

        ret = debug_getregs(ps.tid, &regs);
	if (ret < 0) return 1;

	// TODO: implement [stack] map uptrace method too
	cons_printf("esp backtrace\n");
	for(i=0;i<0xffff;i++) {
		unsigned long addr = (unsigned long)ptr;
		debug_read_at(ps.tid, &ebp2, 4, R_ESP(regs));
		debug_read_at(ps.tid, &ptr, 4, R_ESP(regs)+4);
//		if (ptr == 0xffffffff || R_ESP(regs) == 0x0) break;
//cons_printf("%d %08x\n", i, addr);

		{
			struct list_head *pos;
			char name[128];

			list_for_each_prev(pos, &ps.map_reg) {
				MAP_REG	*mr = (MAP_REG *)((char *)pos + \
						sizeof(struct list_head) - \
						sizeof(MAP_REG));

				if ( (mr->perms & REGION_EXEC) )
				if (addr >= mr->ini && addr <= mr->end) {
					strcpy(name, mr->bin);
					for(j=0;name[j];j++) {
						int ch = name[j];
						if (!is_printable(ch)||ch=='/'||ch=='.'||ch=='-') {
							ch = '_';
						}
						name[j] = ch;
					}
					name[j]='\0';
					cons_printf("[%08x] 0x%08x [%s] %s\n",
						R_ESP(regs), addr, name, flag_name_by_offset((addr_t)addr));
					break;
				}
			}
		}
		//cons_printf("#%d 0x%08x %s\n", i, addr, flag_name_by_offset((addr_t)ptr));
		R_ESP(regs) = R_ESP(regs)+4;
	}
	return i;
}

#if 0
	/*
	%ebp points to the old ebp var
	%ebp+4 points to ret
	*/
// XXX FIND STACK FRAME
	debug_read_at(ps.tid, &buf, 4, R_EIP(regs));
	if (!memcmp(buf, "\x55\x89\xe5", 3)
	||  !memcmp(buf, "\x89\xe5\x57", 3)) { /* push %ebp ; mov %esp, %ebp */
		debug_read_at(ps.tid, &ptr, 4, R_ESP(regs));
		cons_printf("#0 0x%08x %s", (unsigned long)ptr, flag_name_by_offset((addr_t)ptr));
		R_EBP(regs) = ptr;
	}

	for(i=0;i<10;i++) {
		debug_read_at(ps.tid, &ebp2, 4, R_EBP(regs));
		debug_read_at(ps.tid, &ptr, 4, R_EBP(regs)+4);
		if (ptr == 0x0 || R_EBP(regs) == 0x0) break;
		cons_printf("#%d 0x%08x %s\n", i, (unsigned long)ptr, flag_name_by_offset((addr_t)ptr));
		R_EBP(regs) = ebp2;
#endif

int arch_backtrace()
{
	int ret, i,j;
	unsigned long esp;
	unsigned long ebp2;
	regs_t regs;
	unsigned char buf[4];

	if (ps.opened == 0)
		return 0;

	ret = debug_getregs(ps.tid, &regs);
	if (ret < 0) return 1;

	// TODO: implement [stack] map uptrace method too
	esp = R_ESP(regs);
	for(i=0;i<1024;i++) {
		unsigned long addr;
		debug_read_at(ps.tid, &ebp2, 4, esp);
		buf[0]=0;
		debug_read_at(ps.tid, &buf, 4, (ebp2-5)-(ebp2-5)%4);

		addr = ebp2;
		// TODO: arch_is_call() here and this fun will be portable
		if (buf[(ebp2-5)%4]==0xe8) {
			// is call
			char name[128];
			struct list_head *pos;
			char flagname[128];
			char c=' ';
			name[0]='\0';

			/* find map */
			list_for_each_prev(pos, &ps.map_reg) {
				MAP_REG *mr = (MAP_REG *)((char *)pos + \
						sizeof(struct list_head) - \
						sizeof(MAP_REG));

				if ( (mr->perms & REGION_EXEC) )
					if (addr >= mr->ini && addr <= mr->end) {
						strcpy(name, mr->bin);
						c = (mr->flags & FLAG_USERCODE)?'u':'.';
						for(j=0;name[j];j++) {
							int ch = name[j];
							if (!is_printable(ch)||ch=='/'||ch=='.'||ch=='-') {
								ch = '_';
							}
							name[j] = ch;
						}
						name[j]='\0';
						break;
					}
			}

			/* find flag name + offset */
			string_flag_offset(flagname, addr);

			/* print out */
			D {
				if (c=='u')
				cons_printf("[esp+%04x] %c 0x%08x [%s] %s\n",
					esp-R_ESP(regs), c, ebp2, name, flagname);
			} else {
				cons_printf("[esp+%04x] %c 0x%08x [%s] %s\n",
					esp-R_ESP(regs), c, ebp2, name, flagname);
			}
		}
		esp+=4;
	}
	return i;
}

void dump_eflags(const int eflags)
{
    cons_printf("%c%c%c%c%c%c%c%c%c%c%c ",
	   eflags & (1 <<  0) ? 'C' : 'c',
	   eflags & (1 <<  2) ? 'P' : 'p',
	   eflags & (1 <<  4) ? 'A' : 'a',
	   eflags & (1 <<  6) ? 'Z' : 'z',
	   eflags & (1 <<  7) ? 'S' : 's',
	   eflags & (1 <<  8) ? 'T' : 't',
	   eflags & (1 <<  9) ? 'I' : 'i',
	   eflags & (1 << 10) ? 'D' : 'd',
	   eflags & (1 << 11) ? 'O' : 'o',
	   eflags & (1 << 16) ? 'R' : 'r',
	   ((eflags >> 12) & 3) + '0');
    cons_printf("(%s%s%s%s%s%s%s%s%s%s)\n",
	   eflags & (1 <<  0) ? "C" : "",
	   eflags & (1 <<  2) ? "P" : "",
	   eflags & (1 <<  4) ? "A" : "",
	   eflags & (1 <<  6) ? "Z" : "",
	   eflags & (1 <<  7) ? "S" : "",
	   eflags & (1 <<  8) ? "T" : "",
	   eflags & (1 <<  9) ? "I" : "",
	   eflags & (1 << 10) ? "D" : "",
	   eflags & (1 << 11) ? "O" : "",
	   eflags & (1 << 16) ? "R" : "");
}

int arch_ret()
{
	int ret;
	regs_t regs;

	if (ps.opened == 0)
		return 0;

	debug_getregs(ps.tid, &regs);
	if (ret < 0) return 1;
	debug_read_at(ps.tid, &R_EIP(regs), 4, R_ESP(regs));
	R_ESP(regs)+=4;
	debug_setregs(ps.tid, &regs);

	return 0;
}

int arch_call(char *arg)
{
	int ret;
	regs_t regs;
	unsigned long addr;

	if (ps.opened == 0)
		return 0;

	addr = get_offset(arg);
	ret = debug_getregs(ps.tid, &regs);
	if (ret < 0) return 1;
	R_ESP(regs)-=4;
	debug_write_at(ps.tid, &R_EIP(regs), 4, R_ESP(regs));
	if (arg[0]=='+')
		R_EIP(regs) += addr;
	else
	if (arg[0]=='-')
		R_EIP(regs) -= addr;
	else
		R_EIP(regs) = addr;
	ret = debug_setregs(ps.tid, &regs);

	return 0;
}


#if __WINDOWS__
int arch_print_fpregisters(int rad, const char *mask)
{
	eprintf("not work yet");		
	return 0;
}
#else

// NO RAD FOR FPREGS (only 32&64 bit vars)
int arch_print_fpregisters(int rad, const char *mask)
{
	int i, ret;
#if __linux__
	//struct user_fpregs_struct regs;
	elf_fpregset_t regs;
#else
	struct fpreg regs;
#endif

	if (ps.opened == 0)
		return 1;
#if __linux__
#warning THIS CODE IS BUGGY! LINUX AND GNU SUX A LOT
	ret = ptrace(PTRACE_GETFPREGS, ps.tid, NULL, (void *)&regs);
	printf("  cwd 0x%lx  ; control\n", regs.cwd);
	printf("  swd 0x%lx  ; status\n", regs.swd);
	printf("  twd 0x%lx  ; tag\n", regs.twd);
	printf("  fip 0x%lx  ; eip of fpu opcode\n", regs.fip);
	printf("  fcs 0x%lx\n", regs.fcs);
	printf("  foo 0x%lx  ; stack\n", regs.foo);
	printf("  fos 0x%lx\n", regs.fos);
	for(i=0;i<20;i++)
		cons_printf("  st%d %Lf\n", i, regs.st_space[i]);
#else
	ret = ptrace(PTRACE_GETFPREGS, ps.tid, &regs, (void *)sizeof(regs_t));
	for(i=0;i<8;i++)
#if __NetBSD__
		cons_printf("  st%d %f\n", i, regs.__data[i]); // XXX Fill this in with real info.
#else
		cons_printf("  st%d %f\n", i, regs.fpr_env[i]);
#endif
#endif

	return ret;
}

#endif

#if __linux__
/* syscall-linux */
struct syscall_t {
  char *name;
  int num;
  int args;
} syscalls_linux_x86[] = {
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
} syscalls_netbsd_x86[] = {
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

int arch_print_syscall()
{
#if __linux__ || __NetBSD__
	int i,j,ret;
	regs_t regs;
	struct syscall_t *ptr = (struct syscall_t *) 
#if __linux__
	&syscalls_linux_x86;
#endif
#if __NetBSD__
	&syscalls_netbsd_x86;
#endif

	ret = debug_getregs(ps.tid, &regs);
	if (ret < 0) {
		perror("getregs");
		return -1;
	}
	cons_printf("0x%08x syscall(%d) ", R_EIP(regs), R_OEAX(regs), R_EAX(regs));

	for(i=0;ptr[i].num;i++) {
		if (R_OEAX(regs) == ptr[i].num) {
			cons_printf("%s ( ", ptr[i].name);
			j = ptr[i].args;
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
	return -1;
#endif
}

#include <time.h>
void getHTTPDate(char *DATE)
{
#if __UNIX__
#include <sys/utsname.h>
	struct tm curt; /* current time */
	time_t l;
	char week_day[4], month[4];

	l=time(0);
	localtime_r(&l,&curt);

	DATE[0]=0;
	switch(curt.tm_wday)
	{
		case 0: strcpy(week_day, "Sun");
			break;
		case 1: strcpy(week_day, "Mon");
			break;
		case 2:	strcpy(week_day, "Tue");
			break;
		case 3:	strcpy(week_day, "Wed");
			break;
		case 4:	strcpy(week_day, "Thu");
			break;
		case 5:	strcpy(week_day, "Fri");
			break;
		case 6:	strcpy(week_day, "Sat");
			break;
		default: return;
	}
	
	switch(curt.tm_mon)
	{
		case 0: strcpy(month, "Jan");
			break;
		case 1: strcpy(month, "Feb");
			break;
		case 2: strcpy(month, "Mar");
			break;
		case 3: strcpy(month, "Apr");
			break;
		case 4: strcpy(month, "May");
			break;
		case 5: strcpy(month, "Jun");
			break;
		case 6: strcpy(month, "Jul");
			break;
		case 7: strcpy(month, "Aug");
			break;
		case 8: strcpy(month, "Sep");
			break;
		case 9: strcpy(month, "Oct");
			break;
		case 10: strcpy(month, "Nov");
			break;
		case 11: strcpy(month, "Dec");
			break;
		default: return;
	}
	
	sprintf(DATE, "%s, %02d %s %d %02d:%02d:%02d GMT", 
			week_day, curt.tm_mday, month, 
			curt.tm_year + 1900, curt.tm_hour, 
			curt.tm_min, curt.tm_sec);
#else
	DATE[0]='\0';

#endif
}

static char oregs_timestamp[128];
static regs_t oregs;
static regs_t nregs;

int arch_print_registers(int rad, const char *mask)
{
	int ret;
	regs_t regs;
	int color = (int)config_get("scr.color");

	if (ps.opened == 0)
		return 0;

	if (mask && mask[0]=='o') { // orig
		memcpy(&regs, &oregs, sizeof(regs_t));
		if (oregs_timestamp[0])
			cons_printf("%s\n", oregs_timestamp);
	} else {
		ret = debug_getregs(ps.tid, &regs);
		if (ret < 0) {
			perror("getregs");
			return 1;
		}
	}

	if (rad)
		rad = 1;
	if (mask&&mask[0]=='l')
		rad = 2;
	
	if (rad == 1) {
		//cons_printf("\n"); // stupid trick
		cons_printf("f oeax @ 0x%x\n", (int)R_OEAX(regs));
		cons_printf("f eax @ 0x%x\n", (int)R_EAX(regs));
		cons_printf("f ebx @ 0x%x\n", (int)R_EBX(regs));
		cons_printf("f ecx @ 0x%x\n", (int)R_ECX(regs));
		cons_printf("f edx @ 0x%x\n", (int)R_EDX(regs));
		cons_printf("f ebp @ 0x%x\n", (int)R_EBP(regs));
		cons_printf("f esi @ 0x%x\n", (int)R_ESI(regs));
		cons_printf("f edi @ 0x%x\n", (int)R_EDI(regs));
		cons_printf("f oeip @ 0x%x\n", (int)R_EIP(oregs));
		cons_printf("f eip @ 0x%x\n", (int)R_EIP(regs));
		cons_printf("f oesp @ 0x%x\n", (int)R_ESP(oregs));
		cons_printf("f esp @ 0x%x\n", (int)R_ESP(regs));
	} else
	if (rad == 2) {
			if (R_EAX(regs)!=R_EAX(oregs)) cons_printf("eax = 0x%08x (0x%08x) ", R_EAX(regs), R_EAX(oregs));
			if (R_EBX(regs)!=R_EBX(oregs)) cons_printf("ebx = 0x%08x (0x%08x) ", R_EBX(regs), R_EBX(oregs));
			if (R_ECX(regs)!=R_ECX(oregs)) cons_printf("ecx = 0x%08x (0x%08x) ", R_ECX(regs), R_ECX(oregs));
			if (R_EDX(regs)!=R_EDX(oregs)) cons_printf("edx = 0x%08x (0x%08x) ", R_EDX(regs), R_EDX(oregs));
			if (R_ESI(regs)!=R_ESI(oregs)) cons_printf("esi = 0x%08x (0x%08x) ", R_ESI(regs), R_ESI(oregs));
			if (R_EDI(regs)!=R_EDI(oregs)) cons_printf("edi = 0x%08x (0x%08x) ", R_EDI(regs), R_EDI(oregs));
			if (R_EBP(regs)!=R_EBP(oregs)) cons_printf("ebp = 0x%08x (0x%08x) ", R_EBP(regs), R_EBP(oregs));
			if (R_ESP(regs)!=R_ESP(oregs)) cons_printf("esp = 0x%08x (0x%08x) ", R_ESP(regs), R_ESP(oregs));
			if (R_EFLAGS(regs)!=R_EFLAGS(oregs)) cons_printf("eflags = 0x%04x (0x%04x)", R_EFLAGS(regs), R_EFLAGS(oregs));
			NEWLINE;
	} else {
		if (color) {
			if (R_EAX(regs)!=R_EAX(oregs)) cons_printf("\e[35m");
			else cons_printf("\e[0m");
			cons_printf("  eax  0x%08x\e[0m", R_EAX(regs));
			if (R_ESI(regs)!=R_ESI(oregs)) cons_printf("\e[35m");
			cons_printf("    esi  0x%08x\e[0m", R_ESI(regs));
			if (R_EIP(regs)!=R_EIP(oregs)) cons_printf("\e[35m");
			cons_printf("    eip    0x%08x\e[0m\n",  R_EIP(regs));
			if (R_EBX(regs)!=R_EBX(oregs)) cons_printf("\e[35m");
			cons_printf("  ebx  0x%08x\e[0m", R_EBX(regs));
			if (R_EDI(regs)!=R_EDI(oregs)) cons_printf("\e[35m");
			cons_printf("    edi  0x%08x\e[0m", R_EDI(regs));
			if (R_OEAX(regs)!=R_OEAX(oregs)) cons_printf("\e[35m");
			cons_printf("    oeax   0x%08x\e[0m\n",   R_OEAX(regs));
			if (R_ECX(regs)!=R_ECX(oregs)) cons_printf("\e[35m");
			cons_printf("  ecx  0x%08x\e[0m", R_ECX(regs));
			if (R_ESP(regs)!=R_ESP(oregs)) cons_printf("\e[35m");
			cons_printf("    esp  0x%08x\e[0m", R_ESP(regs));
			if (R_EFLAGS(regs)!=R_EFLAGS(oregs)) cons_printf("\e[35m");
			cons_printf("    eflags 0x%04x  \n", R_EFLAGS(regs));
			if (R_EDX(regs)!=R_EDX(oregs)) cons_printf("\e[35m");
			cons_printf("  edx  0x%08x\e[0m", R_EDX(regs));
			if (R_EBP(regs)!=R_EBP(oregs)) cons_printf("\e[35m");
			cons_printf("    ebp  0x%08x\e[0m    ", R_EBP(regs));
			
			dump_eflags(R_EFLAGS(regs));
//#if __linux__
//		printf("  cs: 0x%04x   ds: 0x%04x   fs: 0x%04x   gs: 0x%04x\n", regs.cs, regs.ds, regs.fs, regs.gs);
//#endif
		} else {
			cons_printf("  eax  0x%08x    esi  0x%08x    eip    0x%08x\n",   R_EAX(regs), (int)R_ESI(regs), R_EIP(regs));
			cons_printf("  ebx  0x%08x    edi  0x%08x    oeax   0x%08x\n",   R_EBX(regs), R_EDI(regs), R_OEAX(regs));
			cons_printf("  ecx  0x%08x    esp  0x%08x    eflags 0x%04x\n",   R_ECX(regs), (int)R_ESP(regs), R_EFLAGS(regs));
			cons_printf("  edx  0x%08x    ebp  0x%08x    ", (int)R_EDX(regs), (int)R_EBP(regs));
			dump_eflags(R_EFLAGS(regs));
		}
	}

	if (memcmp(&nregs,&regs, sizeof(regs_t))) {
		getHTTPDate((char *)&oregs_timestamp);
		memcpy(&oregs, &nregs, sizeof(regs_t));
		memcpy(&nregs, &regs, sizeof(regs_t));
	} else {
		memcpy(&nregs, &regs, sizeof(regs_t));
	}

	return 0;
}

long get_value(char *str)
{
	long tmp;

	if (str[0]&&str[1]=='x')
		sscanf(str, "0x%lx", &tmp);
	else	tmp = atol(str);
	return tmp;
}

int arch_set_register(char *reg, char *value)
{
	int ret;
	regs_t regs;

	if (ps.opened == 0)
		return 0;

        ret = debug_getregs(ps.tid, &regs);
	if (ret < 0) return 1;
	
	if (!strcmp(reg, "eax"))
		R_EAX(regs) = get_value(value);
	else if (!strcmp(reg, "ebx"))
		R_EBX(regs) = get_value(value);
	else if (!strcmp(reg, "ecx"))
		R_ECX(regs) = get_value(value);
	else if (!strcmp(reg, "edx"))
		R_EDX(regs) = get_value(value);
	else if (!strcmp(reg, "ebp"))
		R_EBP(regs) = get_value(value);
	else if (!strcmp(reg, "esp"))
		R_ESP(regs) = get_value(value);
	else if (!strcmp(reg, "esi"))
		R_ESI(regs) = get_value(value);
	else if (!strcmp(reg, "edi"))
		R_EDI(regs) = get_value(value);
	else if (!strcmp(reg, "eip"))
		R_EIP(regs) = get_value(value);
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
		eprintf("Valid registers:\n");
		eprintf(" eax, ebx, ecx, edx, esi, edi, ebp, esp, eflags\n");
	}

	debug_setregs(ps.tid, &regs);
	return 0;
}


int arch_continue()
{
	regs_t regs;

        debug_getregs(ps.tid, &regs);
	return debug_contp(ps.tid); /* ptrace(PTRACE_CONT, ps.tid, R_EIP(regs), (void *)0); */
}

addr_t arch_mmap(int fd, int size, addr_t addr) //int *rsize)
{
/*
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
*/
#ifdef __linux__
        regs_t   reg, reg_saved;
        int     status;
	char	bak[4];
        void*   ret = (void *)-1;

	/* save old registers */
        debug_getregs(ps.tid, &reg_saved);
	memcpy(&reg, &reg_saved, sizeof(reg));

	// XXX: FUCK MMAP GOES TO ESP!

	/* mmap call */
        R_EAX(reg) = 90;    // SYS_mmap
        R_EBX(reg) = addr;  // mmap addr
        R_ECX(reg) = size;  // size
        R_EDX(reg) = 0x7;   // perm
        R_EDX(reg) = 0x1;   // options
        R_ESI(reg) = fd;    // fd
        R_EDI(reg) = 0;     // offset

        /* write syscall interrupt code */
        R_EIP(reg) = R_ESP(reg) - 4;

	/* read stack values */
        debug_read_at(ps.tid, bak, 4, R_EIP(reg));

	/* write SYSCALL OPS */
	debug_write_at(ps.tid, (long *)SYSCALL_OPS, 4, R_EIP(reg));

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
        	ret = (void *)R_EAX(reg);
		if (ret != 0) {
			eprintf("oops\n");
			return 0;
		}
	}

        /* restore memory */
	debug_write_at(ps.tid, (long *)bak, 4, R_ESP(reg_saved) - 4);

        /* restore registers */
        debug_setregs(ps.tid, &reg_saved);
#else
	eprintf("Not supported on this OS\n");
	return 0;
#endif

	return (addr_t)addr;
} 

addr_t arch_alloc_page(unsigned long size, unsigned long *rsize)
{
#ifdef __linux__
        regs_t   reg, reg_saved;
        int     status;
	char	bak[4];
        addr_t  ret = (addr_t)-1;

	/* save old registers */
        debug_getregs(ps.tid, &reg_saved);
	memcpy(&reg, &reg_saved, sizeof(reg));

	/* mmap call */
        R_EAX(reg) = 0xc0;
	R_ESI(reg) = 0x21;
        R_EDI(reg) = 0;
        R_EBP(reg) = 0;
        R_EDX(reg) = 5;
        R_ECX(reg) = (size + PAGE_SIZE - 1) & PAGE_MASK;
        R_EBX(reg) = 0;

        /* write syscall interrupt code */
        R_EIP(reg) = R_ESP(reg) - 4;

	/* read stack values */
        debug_read_at(ps.tid, bak, 4, R_EIP(reg));

	/* write SYSCALL OPS */
	debug_write_at(ps.tid, (long *)SYSCALL_OPS, 4, R_EIP(reg));

        /* set new registers value */
        debug_setregs(ps.tid, &reg);

        /* continue */
        debug_contp(ps.tid);

	/* set real allocated size */
	*rsize = R_ECX(reg);

        /* wait to stop process */
        waitpid(ps.tid, &status, 0);

	if(WIFSTOPPED(status)) {
        	/* get new registers value */
        	debug_getregs(ps.tid, &reg);

        	/* read allocated address */
        	ret = (addr_t)R_EAX(reg);
	}

        /* restore memory */
	debug_write_at(ps.tid, (long *)bak, 4, R_ESP(reg_saved) - 4);

        /* restore registers */
        debug_setregs(ps.tid, &reg_saved);

	return ret;
#else
	eprintf("Not supported on this OS\n");
	return 0;
#endif
} 

addr_t arch_dealloc_page(addr_t addr, unsigned long size)
{
#ifdef	__linux__

        regs_t  reg, reg_saved;
        int     status;
	char	bak[4];
        addr_t  ret = (addr_t)-1;

	/* save old registers */
        debug_getregs(ps.tid, &reg_saved);
	memcpy(&reg, &reg_saved, sizeof(reg));

	/* mumap call */
        R_EAX(reg) = 0x5b;
	R_EBX(reg) = (long)addr;
        R_ECX(reg) = (size + PAGE_SIZE - 1) & PAGE_MASK;

        /* write syscall interrupt code */
        R_EIP(reg) = R_ESP(reg) - 4;

	/* read stack values */
	debug_read_at(ps.tid, bak, 4, R_EIP(reg) - 4);

	/* write SYSCALL OPS */
	debug_write_at(ps.tid, (long *)SYSCALL_OPS, 4, R_EIP(reg));

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
        	ret = (addr_t)R_EAX(reg);
	}

        /* restore memory */
	debug_write_at(ps.tid, (long *)bak, 8, R_ESP(reg_saved) - 4);

        /* restore registers */
        debug_setregs(ps.tid, &reg_saved);

	return ret;
#else
	eprintf("Not supported on this OS\n");
	return 0;
#endif
}

addr_t arch_get_sighandler(int signum)
{
#ifdef __linux__
        regs_t   reg, reg_saved;
        int     status;
	char	bak[8];
        addr_t	ret = (addr_t)-1;

	/* save old registers */
        debug_getregs(ps.tid, &reg_saved);
	memcpy(&reg, &reg_saved, sizeof(reg));

        R_EAX(reg) = 0xae;
        R_ESI(reg) = 8;
        R_EDX(reg) = R_ESP(reg) - 8;
        R_ECX(reg) = 0;
        R_EBX(reg) = signum;

        /* save memory */
        debug_read_at(ps.tid, bak, 8, R_EDX(reg));

        /* write -1 */
	debug_write_at(ps.tid, (long *)&ret, 4, R_EDX(reg));

        /* write syscall interrupt code */
        R_EIP(reg) = R_ESP(reg) - 4;
	debug_write_at(ps.tid, (long *)SYSCALL_OPS, 4, R_EIP(reg));

        /* set new registers value */
        debug_setregs(ps.tid, &reg);

        /* continue */
        debug_contp(ps.tid);

        /* wait to stop process */
        waitpid(ps.tid, &status, 0);

	if(WIFSTOPPED(status)) {
        	/* read sighandler address */
        	debug_read_at(ps.tid, &ret, 4, R_ESP(reg_saved) - 8);
	}

        /* restore memory */
	debug_write_at(ps.tid, (long *)bak, 8, R_ESP(reg_saved) - 8);

        /* restore registers */
        debug_setregs(ps.tid, &reg_saved);

	return ret;
#else
	eprintf("Not supported on this OS\n");
	return 0;
#endif
}

// XXX this code demonstrate how buggy is debug_inject
#if 0
void signal_set(int signum, addr_t address)
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

int arch_mprotect(addr_t addr, unsigned int size, int perms)
{
#ifdef __linux__
        regs_t   reg, reg_saved;
        int     status;
        char    bak[4];
        int   ret = -1;

        /* save old registers */
        debug_getregs(ps.tid, &reg_saved);
        memcpy(&reg, &reg_saved, sizeof(reg));

        R_EAX(reg) = 0x7d;
        R_ECX(reg) = size;
        R_EDX(reg) = perms;
        R_EBX(reg) = (int)addr;

        R_EIP(reg) = R_ESP(reg) - 4;

	/* read stack values */
	debug_read_at(ps.tid, bak, 4, R_EIP(reg));

        /* write syscall interrupt code */
        debug_write_at(ps.tid, (long *)SYSCALL_OPS, 4, R_EIP(reg));

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
        	ret = (int)R_EAX(reg);
	}

        /* restore memory */
	debug_write_at(ps.tid, (long *)bak, 4, R_ESP(reg_saved) - 4);

        /* restore registers */
        debug_setregs(ps.tid, &reg_saved);

	return ret;
#else
	eprintf("Not supported on this OS\n");
	return -1;
#endif
}

addr_t arch_set_sighandler(int signum, addr_t handler)
{
#ifdef __linux__
        regs_t   reg, reg_saved;
        int     status;
	char	bak[8];
        addr_t  ret = (addr_t)-1;

	/* save old registers */
        debug_getregs(ps.tid, &reg_saved);
	memcpy(&reg, &reg_saved, sizeof(reg));

        R_EAX(reg) = 0x30;
        R_EBX(reg) = signum;
        R_ECX(reg) = handler;
        R_EDX(reg) = R_ESP(reg) - 8;

        /* save memory */
        debug_read_at(ps.tid, bak, 8, R_EDX(reg));

        /* write -1 */
	debug_write_at(ps.tid, (long *)&ret, 4, R_EDX(reg));

        /* write syscall interrupt code */
        R_EIP(reg) = R_ESP(reg) - 4;
	debug_write_at(ps.tid, (long *)SYSCALL_OPS, 4, R_EIP(reg));

        /* set new registers value */
        debug_setregs(ps.tid, &reg);

        /* continue */
        debug_contp(ps.tid);

        /* wait to stop process */
        waitpid(ps.tid, &status, 0);

	if(WIFSTOPPED(status)) {
        	/* read sighandler address */
        	debug_read_at(ps.tid, &ret, 4, R_ESP(reg_saved) - 8);
	}

        /* restore memory */
	debug_write_at(ps.tid, (long *)bak, 8, R_ESP(reg_saved) - 8);
        debug_getregs(ps.tid, &reg);
	if (R_EAX(reg)==0) {
		eprintf("Signal %d handled by 0x%08x\n", signum, handler);
	} else {
		if (handler == 0)
			eprintf("(DEFAULT)\n");
		else
			eprintf("Error\n");
	}

        /* restore registers */
        debug_setregs(ps.tid, &reg_saved);

	return ret;
#else
	eprintf("Not supported on this OS\n");
	return 0;
#endif
}

/* THIS CODE MUST BE REMOVED */
/* I SHOULD USE THE AOP INTERFACE */
// XXX must handle current seek
// XXX must say if this is a forced jump or a conditional one
int arch_is_jmp(const unsigned char *cmd, unsigned long *addr)
{
	switch(cmd[0]) {
	case 0xe9: // jmp
	case 0xea: // far jmp
	case 0xeb: // jmp 8
		*addr = (unsigned long)(cmd+1)+5;
		return 5;
	}
	/* conditional jump */
	if (cmd[0]>=0x80&&cmd[0]<=0x8F)
		return 5;

	return 0;
}

int arch_is_call(const unsigned char *cmd)
{
	if (cmd[0] == 0xe8) // call
		return 5;
	return 0;
}

int arch_is_soft_stepoverable(const unsigned char *cmd)
{
	if (cmd==NULL)
		return 0;
	if (cmd[0]==0xf3) // repz
		return 2;

	if (cmd[0]==0xf2) // repnz
		return 2;
	return 0;
}

int arch_is_stepoverable(const unsigned char *cmd)
{
	if (cmd[0]==0xff && (cmd[1]>=0xd0&&cmd[1]<0xdf))
		return 2; // call * reg

	if (cmd[0]==0xe8) // call
		return 5;
	
	return arch_is_soft_stepoverable(cmd);
}

unsigned long get_ret_sf(unsigned long top, unsigned long *ret_pos);

void next_sf(struct list_head *list, unsigned long esp)
{
	unsigned long  ret_pos, ret;
	struct sf_t  *sf;

	/* get return address of stack frame and return position */
	ret = get_ret_sf(esp, &ret_pos);

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
	sf->vars_sz = ret_pos - esp;
	sf->sz =  ret_pos - esp + sizeof(unsigned long);

	/* get next stack frame */
	next_sf(list, ret_pos + sizeof(unsigned long));

	/* add stack frame */
	list_add(&sf->next, list);
}

#if 0
void next_sf(struct list_head *list, unsigned long ebp, unsigned long top)
{
	unsigned long  ret_sf;
	struct sf_t  *sf;

	/* get addr */

	/* get ebp and return address of stack frame */
	if(debug_read_at(ps.tid, &ebp_sf, sizeof(unsigned long),
		 ebp) !=
		 sizeof(unsigned long)  ||
	   debug_read_at(ps.tid, &ret_sf, sizeof(unsigned long),
		 ebp + sizeof(unsigned long)) !=
		 sizeof(unsigned long)
	) 
		return;

	/* fill stack frame structure */
	sf = (struct sf_t *)malloc(sizeof(struct sf_t));
	if(!sf) {
		perror(":error malloc stack frame");
		return;
	}

	sf->ret_addr = ret_sf;
	sf->ebp = ebp;
	sf->vars_sz = ebp - top;
	sf->sz = ebp - top + sizeof(unsigned long) * 2;

	/* get next stack frame */
	next_sf(list, ebp + sizeof(unsigned long) * 2);

	/* add stack frame */
	list_add(&sf->next, list);
}
#endif

unsigned long get_ret_sf(unsigned long esp, unsigned long *ret_pos)
{
	unsigned long pos;
	unsigned long val;
	unsigned char aux;

	pos = esp;
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
	next_sf(bt_list, R_ESP(WS(regs)));

	return bt_list;
}

void arch_view_bt(struct list_head *sf)
{
	char label[256];
	struct list_head *pos;
	struct sf_t *sf_e;
	int i = 0;

	list_for_each(pos, sf) {
                sf_e = list_entry(pos, struct sf_t, next);
		label[0] = '\0';
		string_flag_offset((char *)&label, sf_e->ret_addr);
		cons_printf("%02d 0x%08x (framesz=%03d varsz=%d) %s\n",
			i++, (uint)sf_e->ret_addr,
			(uint)sf_e->sz, (uint)sf_e->vars_sz, label);
	}

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
