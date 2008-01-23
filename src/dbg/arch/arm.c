/*
 * Copyright (C) 2007
 *       pancake <youterm.com>
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

#include "../libps2fd.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <sys/procfs.h>
#include <sys/syscall.h>

long long arch_syscall(int pid, int sc, ...)
{
        long long ret = (off_t)-1;

#if __linux__
	va_list ap;
        //regs_t   reg, reg_saved;
	elf_gregset_t reg, reg_saved;
	int baksz = 128;
        int     i, status;
	char	bak[128];
	long long addr;
	unsigned char my_syscall[4];
	char *arg;
	char *file;

	/* get registers */
	ret = ptrace (PTRACE_GETREGS, ps.tid, 0, &reg);
	memcpy(&reg, &reg_saved, sizeof(reg));

        /* eip is in the stack now */
	reg[15] = reg[13]-4; // pc = sp - 4

	/* read stack values */
        debug_read_at(pid, bak, baksz, reg[15]); // read 4 bytes at eip

	/* set syscall */
	my_syscall[0] = sc;
	my_syscall[1] = 0x00;
	my_syscall[2] = 0x90;
	my_syscall[2] = 0xef;

	arg = &sc;
	va_start(ap, arg);
	switch(sc) {
	case SYS_gettid:
		break;
	case SYS_tkill:
		reg[0] = va_arg(ap, pid_t);
		reg[1] = va_arg(ap, int);
		break;
	case SYS_open:
/*
		addr = R_EIP(reg)+4;
		file = va_arg(ap, char *);
		debug_write_at(pid, file, strlen(file)+4, addr);
		R_EBX(reg) = addr;
		R_ECX(reg) = va_arg(ap, int);
		R_EDX(reg) = 0755; // TODO: Support create flags
*/
		break;
	case SYS_close:
		reg[0] = va_arg(ap, int);
		break;
	case SYS_dup2:
		reg[0] = va_arg(ap, int);
		reg[1] = va_arg(ap, int);
		break;
	case SYS_lseek:
		reg[0] = va_arg(ap, int);
		reg[1] = va_arg(ap, off_t);
		reg[2] = va_arg(ap, int);
		break;
	default:
		printf("ptrace-syscall %d not yet supported\n", sc);
		// XXX return ???
		return -1;
	}
	va_end(ap);

	/* write SYSCALL OPS */
	debug_write_at(pid, my_syscall, 4, reg[15]);

        /* set new registers value */
        debug_setregs(pid, &reg);

        /* continue */
        debug_contp(pid);

        /* wait to stop process */
        waitpid(ps.tid, &status, 0);

	if(WIFSTOPPED(status)) {
        	/* get new registers value */
		ret = ptrace (PTRACE_GETREGS, ps.tid, 0, &reg_saved);

        	/* read allocated address */
        	ret = (off_t)reg[0];
		if (((long long)ret)<0) ret=0;
	}

        /* restore memory */
	debug_write_at(ps.tid, (long *)bak, baksz, reg[15]);

        /* restore registers */
	ret = ptrace (PTRACE_SETREGS, ps.tid, 0, &reg_saved);
#else
	eprintf("not yet for this platform\n");
#endif

	return ret;
}

void arch_dump_registers()
{
	FILE *fd;
	int ret, regno;
	elf_gregset_t regs;

	printf("Dumping CPU to cpustate.dump...\n");
	ret = ptrace (PTRACE_GETREGS, ps.tid, 0, &regs);

	fd = fopen("cpustate.dump", "w");
	if (fd == NULL) {
		fprintf(stderr, "Cannot open\n");
		return 0;
	}
	fprintf(fd, "r00 0x%08x\n", (uint)regs[0]);
	fprintf(fd, "r01 0x%08x\n", (uint)regs[1]);
	fprintf(fd, "r02 0x%08x\n", (uint)regs[2]);
	fprintf(fd, "r03 0x%08x\n", (uint)regs[3]);
	fprintf(fd, "r04 0x%08x\n", (uint)regs[4]);
	fprintf(fd, "r05 0x%08x\n", (uint)regs[5]);
	fprintf(fd, "r06 0x%08x\n", (uint)regs[6]);
	fprintf(fd, "r07 0x%08x\n", (uint)regs[7]);
	fprintf(fd, "r08 0x%08x\n", (uint)regs[8]);
	fprintf(fd, "r09 0x%08x\n", (uint)regs[9]);
	fprintf(fd, "r10 0x%08x\n", (uint)regs[10]);
	fprintf(fd, "r11 0x%08x\n", (uint)regs[11]);
	fprintf(fd, "r12 0x%08x\n", (uint)regs[12]);
	fprintf(fd, "r13 0x%08x\n", (uint)regs[13]);
	fprintf(fd, "r14 0x%08x\n", (uint)regs[14]);
	fprintf(fd, "r15 0x%08x\n", (uint)regs[15]);
	fprintf(fd, "r16 0x%08x\n", (uint)regs[16]);
	fprintf(fd, "r17 0x%08x\n", (uint)regs[17]);
	fclose(fd);
}

int arch_stackanal()
{
	return 0;
}

void arch_restore_registers()
{
	FILE *fd;
	char buf[1024];
	char reg[10];
	unsigned int val;
	int key, ret;
	elf_gregset_t regs;

	printf("Dumping CPU to cpustate.dump...\n");
	ret = ptrace (PTRACE_GETREGS, ps.tid, 0, &regs);

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
		case 3158130: regs[0] = val; break;
		case 3223666: regs[1] = val; break;
		case 3289202: regs[2] = val; break;
		case 3354738: regs[3] = val; break;
		case 3420274: regs[4] = val; break;
		case 3485810: regs[5] = val; break;
		case 3551346: regs[6] = val; break;
		case 3616882: regs[7] = val; break;
		case 3682418: regs[8] = val; break;
		case 3747954: regs[9] = val; break;
		case 3158386: regs[10] = val; break;
		case 3223922: regs[11] = val; break;
		case 3289458: regs[12] = val; break;
		case 3354994: regs[13] = val; break;
		case 3420530: regs[14] = val; break;
		case 3486066: regs[15] = val; break;
		case 3551602: regs[16] = val; break;
		case 3617138: regs[17] = val; break;
		}
	}
	fclose(fd);

	ret = ptrace (PTRACE_SETREGS, ps.tid, 0, &regs);

	return 1;
}

int arch_mprotect(char *addr, unsigned int size, int perms)
{
	fprintf(stderr, "TODO: arch_mprotect\n");
	return 0;
}

long get_value(char *str)
{
	/* parse register name and return value */
	return 0;
}

int arch_is_soft_stepoverable(const char *cmd)
{
	return 0;
}

int arch_is_stepoverable(const unsigned char *cmd)
{
#warning TODO: arch_is_stepoverable()
	return 0;
}

int arch_call(char *arg)
{
	return 0;
}
#if 0
   >  * `gregset' for the general-purpose registers.
   > 
   >  * `fpregset' for the floating-point registers.
   > 
   >  * `xregset' for any "extra" registers.
#endif

int arch_ret()
{
#define uregs regs
	elf_gregset_t regs;
	int ret = ptrace(PTRACE_GETREGS, ps.tid, NULL, &regs);
	if (ret < 0) return 1;
	ARM_pc = ARM_lr;
	ptrace(PTRACE_SETREGS, ps.tid, NULL, &regs);
	return ARM_lr;
}

int arch_jmp(off_t ptr)
{
	elf_gregset_t regs;
	int ret = ptrace(PTRACE_GETREGS, ps.tid, NULL, &regs);
	if (ret < 0) return 1;
	ARM_pc = ptr;
	ptrace(PTRACE_SETREGS, ps.tid, NULL, &regs);
	return 0;
}

off_t arch_pc()
{
	elf_gregset_t regs;
	int ret = ptrace(PTRACE_GETREGS, ps.tid, NULL, &regs);
	if (ret < 0) return 1;
	return ARM_pc;
}

int arch_set_register(char *reg, char *value)
{
	int ret;
	elf_gregset_t regs;

	if (ps.opened == 0)
		return 0;

	ret = ptrace(PTRACE_GETREGS, ps.tid, NULL, &regs);
	if (ret < 0) return 1;

	regs[atoi(reg+1)] = get_value(value);

	ret = ptrace(PTRACE_SETREGS, ps.tid, NULL, &regs);
	return 0;
}

int arch_print_fpregisters(int rad, const char *mask)
{
	fprintf(stderr, "TODO\n");
	return 0;
}

elf_gregset_t cregs; // current registers
elf_gregset_t oregs; // old registers

int arch_print_registers(int rad, const char *mask)
{
	int ret, regno;
	elf_gregset_t regs;
	int color = 0;
	char *c = config_get("cfg.color");
	if (c&&*c=='1')color=1;

	/* Get the thread id for the ptrace call.  */
	//tid = GET_THREAD_ID (inferior_ptid);

	if (mask[0]=='o') { // orig
		memcpy(&regs, &oregs, sizeof(regs_t));
	} else {
		ret = ptrace (PTRACE_GETREGS, ps.tid, 0, &regs);
		if (ret < 0) {
			perror("ptrace_getregs");
			return 1;
		}
	}

	if (rad) {
		pprintf("f r0_orig @ 0x%x\n", regs[17]);
		pprintf("f r0  @ 0x%x\n", regs[0]);
		pprintf("f r1  @ 0x%x\n", regs[1]);
		pprintf("f r2  @ 0x%x\n", regs[2]);
		pprintf("f r3  @ 0x%x\n", regs[3]);
		pprintf("f r4  @ 0x%x\n", regs[4]);
		pprintf("f r5  @ 0x%x\n", regs[5]);
		pprintf("f r6  @ 0x%x\n", regs[6]);
		pprintf("f r7  @ 0x%x\n", regs[7]);
		pprintf("f r8  @ 0x%x\n", regs[8]);
		pprintf("f r9  @ 0x%x\n", regs[9]);
		pprintf("f r10 @ 0x%x\n", regs[10]);
		pprintf("f r11 @ 0x%x ; fp\n", regs[11]);
		pprintf("f r12 @ 0x%x ; ip\n", regs[12]);
		pprintf("f r13 @ 0x%x ; sp\n", regs[13]);
		pprintf("f esp @ 0x%x\n", regs[13]);
		pprintf("f r14 @ 0x%x ; lr\n", regs[14]);
		pprintf("f r15 @ 0x%x ; pc\n", regs[15]);
		pprintf("f eip @ 0x%x\n", regs[15]);
		pprintf("f r16 @ 0x%x ; cpsr\n", regs[16]);
	} else {
		if (color) {
			if (regs[0]!=oregs[0]) pprintf("\e[35m");
			pprintf("  r0  0x%08x\e[0m", regs[0]);
			if (regs[1]!=oregs[1]) pprintf("\e[35m");
			pprintf("  r1  0x%08x\e[0m", regs[1]);
			if (regs[2]!=oregs[2]) pprintf("\e[35m");
			pprintf("  r2  0x%08x\e[0m", regs[2]);
#if 0
			pprintf("  r0 0x%08x   r5 0x%08x   r10 0x%08x    pc %08x\n", regs[0], regs[4], regs[8]);
			pprintf("  r1 0x%08x   r6 0x%08x    fp 0x%08x  cpsr %08x\n", regs[1], regs[5], regs[9]);
			pprintf("  r2 0x%08x   r7 0x%08x    ip 0x%08x\n", regs[2], regs[6], regs[10]);
			pprintf("  r3 0x%08x   r8 0x%08x    sp 0x%08x\n", regs[3], regs[7], regs[11]);
			pprintf("  r4 0x%08x   r9 0x%08x    lr 0x%08x\n", regs[17], regs[14]);
#endif
// TODO		
		} else {
			pprintf("  r0 0x%08x   r4 0x%08x   r8 0x%08x\n", regs[0], regs[4], regs[8]);
			pprintf("  r1 0x%08x   r5 0x%08x   r9 0x%08x\n", regs[1], regs[5], regs[9]);
			pprintf("  r2 0x%08x   r6 0x%08x  r10 0x%08x\n", regs[2], regs[6], regs[10]);
			pprintf("  r3 0x%08x   r7 0x%08x r11(fp)0x%08x\n", regs[3], regs[7], regs[11]);
			pprintf("  r0.orig   0x%08x   r14 (lr)   0x%08x\n", regs[17], regs[14]);
			pprintf("  r12 (ip)  0x%08x   r15 (pc)   0x%08x\n", regs[12], regs[15]);
			pprintf("  r13 (sp)  0x%08x   r16 (cpsr) 0x%08x\n", regs[13], regs[16]);
		}
	}

	if (memcmp(&cregs,&regs, sizeof(elf_gregset_t))) {
		memcpy(&oregs, &cregs, sizeof(elf_gregset_t));
		memcpy(&cregs, &regs, sizeof(elf_gregset_t));
	} else {
		memcpy(&cregs, &regs, sizeof(elf_gregset_t));
	}

	return 0;
}

int arch_continue()
{
	int ret;

	elf_gregset_t regs;
	ret = ptrace(PTRACE_GETREGS, ps.tid, NULL, &regs);
	ret = ptrace(PTRACE_CONT, ps.tid, 0, 0); // XXX

	return ret;
}

// TODO
struct bp_t *arch_set_breakpoint(off_t addr)
{
}

int arch_backtrace()
{
	// TODO
}

#if 0
int arch_is_breakpoint(int pre)
{
}

int arch_restore_breakpoint(int pre)
{
}

int arch_reset_breakpoint(int step)
{
}

#endif
int arch_opcode_size()
{
	return 4;
}

void *arch_dealloc_page(void *addr, int size)
{
}

void *arch_alloc_page(int size, int *rsize)
{
}

void *arch_mmap(int fd, int size, off_t addr) //int *rsize)
{
}

void *arch_get_sighandler(int signum)
{
}

void *arch_set_sighandler(int signum, off_t handler)
{
}

off_t arch_get_entrypoint()
{
	unsigned long long addr;
	debug_read_at(ps.tid, &addr, 4, 0x8048018);
	return (off_t)addr;
}

struct syscall_t {
  char *name;
  int num;
  int args;
} syscalls_linux_arm[] = {
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

int arch_print_syscall()
{
	unsigned int sc;
	int i,j;

	/* read 4 previous bytes to ARM_pc and get syscall number from there */
	debug_read_at(ps.tid, &sc, 4, arch_pc()-4);
	sc<<=8; // drop opcode
	for(i=0;syscalls_linux_arm[i].num;i++) {
		if (sc == 0x900000 + syscalls_linux_arm[i].num) {
			printf("%s ( ", syscalls_linux_arm[i].name);
			j = syscalls_linux_arm[i].args;
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

#if 0
d reference to `arch_bt'
:debug.c:(.text+0x990): undefined reference to `arch_view_bt'
:debug.c:(.text+0x998): undefined reference to `free_bt'
:debug.c:(.text+0x9dc): undefined reference to `arch_stackanal'
dbg/system.o:(.data+0x74): undefined reference to `arch_stackanal'
dbg/parser.o: In function `get_tok':parser.c:(.text+0x658): undefined reference to `get_reg'

#endif

struct list_head *arch_bt()
{
	/* ... */
	return NULL;
}

void arch_view_bt(struct list_head *sf)
{
	/* ... */
	return NULL;
}

void free_bt(struct list_head *sf)
{
	/* ... */
	return NULL;
}

int get_reg(char *reg)
{
	elf_gregset_t regs;
	int ret = ptrace (PTRACE_GETREGS, ps.tid, 0, &regs);

	if (ret < 0) {
		perror("ptrace_getregs");
		return 1;
	}

	if (reg[0]=='r') {
		int r = atoi(reg+1);
		if (r>17)r = 17;
		if (r<0)r = 0;
		return regs[r];
	}
}