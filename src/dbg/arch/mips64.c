/*
 * Copyright (C) 2008
 *       pancake <youterm.com>
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

#if 0
+       for (i = 0; i < 32; i++)
+               __put_user (regs->regs[i], data + i);
+       __put_user (regs->lo, data + EF_LO - EF_R0);
+       __put_user (regs->hi, data + EF_HI - EF_R0);
+       __put_user (regs->cp0_epc, data + EF_CP0_EPC - EF_R0);
+       __put_user (regs->cp0_badvaddr, data + EF_CP0_BADVADDR - EF_R0);
+       __put_user (regs->cp0_status, data + EF_CP0_STATUS - EF_R0);
+       __put_user (regs->cp0_cause, data + EF_CP0_CAUSE - EF_R0);
#endif

#include "../../radare.h"
#include "../debug.h"
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

regs_t cregs; // current registers
regs_t oregs; // old registers

#define REG_RA 31 // Return address
#define REG_SP 29 // Stack pointer

long long arch_syscall(int pid, int sc, ...)
{
        long long ret = (off_t)-1;
	return ret;
}

int arch_dump_registers()
{
	FILE *fd;
	int ret;
	regs_t regs;
	unsigned long long *llregs = &regs;

	printf("Dumping CPU to cpustate.dump...\n");
	ret = ptrace (PTRACE_GETREGS, ps.tid, 0, &regs);

	fd = fopen("cpustate.dump", "w");
	if (fd == NULL) {
		fprintf(stderr, "Cannot open\n");
		return;
	}
	fprintf(fd, "r00 0x%08x\n", (uint)llregs[0]);
	fprintf(fd, "r01 0x%08x\n", (uint)llregs[1]);
	fprintf(fd, "r02 0x%08x\n", (uint)llregs[2]);
	fprintf(fd, "r03 0x%08x\n", (uint)llregs[3]);
	fprintf(fd, "r04 0x%08x\n", (uint)llregs[4]);
	fprintf(fd, "r05 0x%08x\n", (uint)llregs[5]);
	fprintf(fd, "r06 0x%08x\n", (uint)llregs[6]);
	fprintf(fd, "r07 0x%08x\n", (uint)llregs[7]);
	fprintf(fd, "r08 0x%08x\n", (uint)llregs[8]);
	fprintf(fd, "r09 0x%08x\n", (uint)llregs[9]);
	fprintf(fd, "r10 0x%08x\n", (uint)llregs[10]);
	fprintf(fd, "r11 0x%08x\n", (uint)llregs[11]);
	fprintf(fd, "r12 0x%08x\n", (uint)llregs[12]);
	fprintf(fd, "r13 0x%08x\n", (uint)llregs[13]);
	fprintf(fd, "r14 0x%08x\n", (uint)llregs[14]);
	fprintf(fd, "r15 0x%08x\n", (uint)llregs[15]);
	fprintf(fd, "r16 0x%08x\n", (uint)llregs[16]);
	fprintf(fd, "r17 0x%08x\n", (uint)llregs[17]);
	fclose(fd);
}

int arch_stackanal()
{
	return 0;
}

int arch_restore_registers()
{
	FILE *fd;
	char buf[1024];
	char reg[10];
	unsigned int val;
	int ret;
	regs_t regs;
	unsigned long *llregs = &regs;

	printf("Dumping CPU to cpustate.dump...\n");
	ret = ptrace (PTRACE_GETREGS, ps.tid, 0, &regs);

	// TODO: show file date
	fd = fopen("cpustate.dump", "r");
	if (fd == NULL) {
		fprintf(stderr, "Cannot open cpustate.dump\n");
		return;
	}

	while(!feof(fd)) {
		fgets(buf, 1023, fd);
		if (feof(fd)) break;
		sscanf(buf, "%3s 0x%08x", reg, &val);
		//printf("	case %d: // %s \n", ( reg[0] + (reg[1]<<8) + (reg[2]<<16) ), reg);
		switch( reg[0] + (reg[1]<<8) + (reg[2]<<16) ) {
		case 3158130: llregs[0] = val; break;
		case 3223666: llregs[1] = val; break;
		case 3289202: llregs[2] = val; break;
		case 3354738: llregs[3] = val; break;
		case 3420274: llregs[4] = val; break;
		case 3485810: llregs[5] = val; break;
		case 3551346: llregs[6] = val; break;
		case 3616882: llregs[7] = val; break;
		case 3682418: llregs[8] = val; break;
		case 3747954: llregs[9] = val; break;
		case 3158386: llregs[10] = val; break;
		case 3223922: llregs[11] = val; break;
		case 3289458: llregs[12] = val; break;
		case 3354994: llregs[13] = val; break;
		case 3420530: llregs[14] = val; break;
		case 3486066: llregs[15] = val; break;
		case 3551602: llregs[16] = val; break;
		case 3617138: llregs[17] = val; break;
		}
	}
	fclose(fd);

	ret = ptrace (PTRACE_SETREGS, ps.tid, 0, &regs);

	return;
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

int arch_is_soft_stepoverable(const unsigned char *cmd)
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
	/* TODO: branch to %ra */
#if 0
#define uregs regs
	regs_t regs;
	int ret = ptrace(PTRACE_GETREGS, ps.tid, NULL, &regs);
	if (ret < 0) return 1;
	ARM_pc = ARM_lr;
	ptrace(PTRACE_SETREGS, ps.tid, NULL, &regs);
	return ARM_lr;
#endif
}

int arch_jmp(u64 ptr)
{
#if 0
	regs_t regs;
	int ret = ptrace(PTRACE_GETREGS, ps.tid, NULL, &regs);
	if (ret < 0) return 1;
	ARM_pc = ptr;
	ptrace(PTRACE_SETREGS, ps.tid, NULL, &regs);
	return 0;
#endif
}

u64 arch_pc()
{
	return 0x4000000;
#if 0
	regs_t regs;
	int ret = ptrace(PTRACE_GETREGS, ps.tid, NULL, &regs);
	if (ret < 0) return 1;
	return ARM_pc;
#endif
}

int arch_set_register(char *reg, char *value)
{
	int ret;
	regs_t regs;
	unsigned long *llregs = &regs;

	if (ps.opened == 0)
		return 0;

	printf("set reg\n"); fflush(stdout);
	ret = ptrace(PTRACE_GETREGS, ps.tid, NULL, &regs);
	if (ret < 0) return 1;

	ret = atoi(reg+1);
	if (ret > 17 || ret < 0) {
		eprintf("Invalid register\n");
	}
	llregs[atoi(reg+1)] = (int)get_offset(value);

	ret = ptrace(PTRACE_SETREGS, ps.tid, NULL, &regs);

	return 0;
}

int arch_print_fpregisters(int rad, const char *mask)
{
	eprintf("TODO\n");
	return 0;
}

int arch_print_registers(int rad, const char *mask)
{
	int ret;
	regs_t regs;
	unsigned long *llregs = &regs;
	unsigned long *ollregs = &oregs;
	int color = config_get("scr.color");

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

	return;

	if (rad) {
		cons_printf("f r0_orig @ 0x%x\n", llregs[17]);
		cons_printf("f r0  @ 0x%x\n", llregs[0]);
		cons_printf("f r1  @ 0x%x\n", llregs[1]);
		cons_printf("f r2  @ 0x%x\n", llregs[2]);
		cons_printf("f r3  @ 0x%x\n", llregs[3]);
		cons_printf("f r4  @ 0x%x\n", llregs[4]);
		cons_printf("f r5  @ 0x%x\n", llregs[5]);
		cons_printf("f r6  @ 0x%x\n", llregs[6]);
		cons_printf("f r7  @ 0x%x\n", llregs[7]);
		cons_printf("f r8  @ 0x%x\n", llregs[8]);
		cons_printf("f r9  @ 0x%x\n", llregs[9]);
		cons_printf("f r10 @ 0x%x\n", llregs[10]);
		cons_printf("f r11 @ 0x%x ; fp\n", llregs[11]);
		cons_printf("f r12 @ 0x%x ; ip\n", llregs[12]);
		cons_printf("f r13 @ 0x%x ; sp\n", llregs[13]);
		cons_printf("f esp @ 0x%x\n", llregs[13]);
		cons_printf("f r14 @ 0x%x ; lr\n", llregs[14]);
		cons_printf("f r15 @ 0x%x ; pc\n", llregs[15]);
		cons_printf("f eip @ 0x%x\n", llregs[15]);
		cons_printf("f r16 @ 0x%x ; cpsr\n", llregs[16]);
	} else {
		if (color) {
			if (llregs[0]!=ollregs[0]) cons_strcat("\e[35m");
			cons_printf("  r0  0x%08x\e[0m", llregs[0]);
			if (llregs[5]!=ollregs[5]) cons_strcat("\e[35m");
			cons_printf("  r5  0x%08x\e[0m", llregs[5]);
			if (llregs[9]!=ollregs[9]) cons_strcat("\e[35m");
			cons_printf("  r9  0x%08x\e[0m", llregs[9]);
			if (llregs[13]!=ollregs[13]) cons_strcat("\e[35m");
			cons_printf(" r13  0x%08x\e[0m\n", llregs[13]);
			//
			if (llregs[1]!=ollregs[1]) cons_strcat("\e[35m");
			cons_printf("  r1  0x%08x\e[0m", llregs[1]);
			if (llregs[6]!=ollregs[6]) cons_strcat("\e[35m");
			cons_printf("  r6  0x%08x\e[0m", llregs[6]);
			if (llregs[10]!=ollregs[10]) cons_strcat("\e[35m");
			cons_printf(" r10  0x%08x\e[0m", llregs[10]);
			if (llregs[14]!=ollregs[14]) cons_strcat("\e[35m");
			cons_printf(" r14  0x%08x\e[0m\n", llregs[14]);
			//
			if (llregs[2]!=ollregs[2]) cons_strcat("\e[35m");
			cons_printf("  r2  0x%08x\e[0m", llregs[2]);
			if (llregs[7]!=ollregs[7]) cons_strcat("\e[35m");
			cons_printf("  r7  0x%08x\e[0m", llregs[7]);
			if (llregs[11]!=ollregs[11]) cons_strcat("\e[35m");
			cons_printf(" r11  0x%08x\e[0m", llregs[11]);
			if (llregs[15]!=ollregs[15]) cons_strcat("\e[35m");
			cons_printf(" r15  0x%08x\e[0m\n", llregs[15]);
			//
			if (llregs[3]!=ollregs[3]) cons_strcat("\e[35m");
			cons_printf("  r3  0x%08x\e[0m", llregs[3]);
			if (llregs[8]!=ollregs[8]) cons_strcat("\e[35m");
			cons_printf("  r8  0x%08x\e[0m", llregs[8]);
			if (llregs[12]!=ollregs[12]) cons_strcat("\e[35m");
			cons_printf(" r12  0x%08x\e[0m", llregs[12]);
			if (llregs[16]!=ollregs[16]) cons_strcat("\e[35m");
			cons_printf(" r16  0x%08x\e[0m\n", llregs[16]);
			//
			if (llregs[4]!=ollregs[4]) cons_strcat("\e[35m");
			cons_printf("  r4  0x%08x\e[0m", llregs[4]);

			if (llregs[11]!=ollregs[11]) cons_strcat("\e[35m");
			cons_strcat("  fp=r11\e[0m");
			if (llregs[12]!=ollregs[12]) cons_strcat("\e[35m");
			cons_strcat("  ip=r12\e[0m");
			if (llregs[13]!=ollregs[13]) cons_strcat("\e[35m");
			cons_strcat("  sp=r13\e[0m");
			if (llregs[14]!=ollregs[14]) cons_strcat("\e[35m");
			cons_strcat("  lr=r14\e[0m");
			if (llregs[15]!=ollregs[15]) cons_strcat("\e[35m");
			cons_strcat("  pc=r15\e[0m");
			if (llregs[16]!=ollregs[16]) cons_strcat("\e[35m");
			cons_strcat("  cpsr=r16\e[0m\n");
		} else {
			cons_printf("  r0 0x%08x   r5 0x%08x   r9 0x%08x  r13 0x%08x\n", llregs[0], llregs[5], llregs[9], llregs[13]);
			cons_printf("  r1 0x%08x   r6 0x%08x  r10 0x%08x  r14 0x%08x\n", llregs[1], llregs[6], llregs[10], llregs[14]);
			cons_printf("  r2 0x%08x   r7 0x%08x  r11 0x%08x  r15 0x%08x\n", llregs[2], llregs[7], llregs[11], llregs[15]);
			cons_printf("  r3 0x%08x   r8 0x%08x  r12 0x%08x  r16 0x%08x\n", llregs[3], llregs[8], llregs[12], llregs[16]);
			cons_printf("  r4 0x%08x   ", llregs[4]);

			if (llregs[11]!=ollregs[11]) cons_strcat("[ fp=r11 ]");
			else cons_strcat("  fp=r11");
			if (llregs[12]!=ollregs[12]) cons_strcat("[ ip=r12 ]");
			else cons_strcat("  ip=r12");
			if (llregs[13]!=ollregs[13]) cons_strcat("[ sp=r13 ]");
			else cons_strcat("  sp=r13");
			if (llregs[14]!=ollregs[14]) cons_strcat("[ lr=r14 ]");
			cons_strcat("  lr=r14");
			if (llregs[15]!=ollregs[15]) cons_strcat("[ pc=r15 ]");
			cons_strcat("  pc=r15");
			if (llregs[16]!=ollregs[16]) cons_strcat("[ cpsr=r16 ]\n");
			cons_strcat("  cpsr=r16\n");
		}
	}

	if (memcmp(&cregs,&regs, sizeof(regs_t))) {
		memcpy(&oregs, &cregs, sizeof(regs_t));
		memcpy(&cregs, &regs, sizeof(regs_t));
	} else
		memcpy(&cregs, &regs, sizeof(regs_t));

	return 0;
}

int arch_continue()
{
	int ret;

	ret = ptrace(PTRACE_CONT, ps.tid, 0, 0); // XXX

	return ret;
}

// TODO
struct bp_t *arch_set_breakpoint(u64 addr)
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

addr_t arch_mmap(int fd, int size, u64 addr) //int *rsize)
{
}

addr_t arch_get_sighandler(int signum)
{
}

addr_t arch_set_sighandler(int signum, u64 handler)
{
}

addr_t arch_get_entrypoint()
{
	unsigned long long addr;
	printf("set entrypo\n"); fflush(stdout);
	debug_read_at(ps.tid, &addr, 4, 0x00400018);
	return (u64)addr;
}
#if 0

struct syscall_t {
  char *name;
  int num;
  int args;
} syscalls_linux_mips64[] = {
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
  { "accept", 254, 1},
  { NULL, 0, 0 }
};
#endif

int arch_print_syscall()
{
#if 0
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
#endif
}

struct list_head *arch_bt()
{
	/* ... */
	return NULL;
}

void arch_view_bt(struct list_head *sf)
{
	/* ... */
	return;
}

void free_bt(struct list_head *sf)
{
	/* ... */
	return;
}

int get_reg(char *reg)
{
	regs_t regs;
	unsigned long *llregs = &regs;
	int ret = ptrace (PTRACE_GETREGS, ps.tid, 0, &regs);

	if (ret < 0) {
		perror("ptrace_getregs");
		return 1;
	}
	return 0;

	if (reg[0]=='r') {
		int r = atoi(reg+1);
		if (r>17)r = 17;
		if (r<0)r = 0;
		return llregs[r];
	}
}
