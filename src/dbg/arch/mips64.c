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
	u32 addr;
	int ret = ptrace (PTRACE_PEEKUSER, ps.tid, PTRACE_PC, &addr);
	if (ret == -1) {
		printf("PEEK PC = -1\n");
		exit(1);
		return -1;
	}

	return addr;
}

int arch_set_register(char *reg, char *value)
{
	int ret;
	regs_t regs;
	unsigned long *llregs = &regs;

	eprintf("arch_set-register\n");
	if (ps.opened == 0)
		return 0;

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

#if 0
reg      name    usage
---+-----------+-------------
0        zero   always zero
1         at    reserved for assembler
2-3     v0-v1   expression evaluation, result of function
4-7     a0-a3   arguments for functions
8-15    t0-t7   temporary (not preserved across calls)
16-23   s0-s7   saved temporary (preserved across calls)
24-25   t8-t9   temporary (not preserved across calls)
26-27   k0-k1   reserved for OS kernel
28      gp      points to global area
29      sp      stack pointer
30      fp      frame pointer
31      ra      return address
#endif

int arch_print_registers(int rad, const char *mask)
{
	int ret;
	regs_t regs;
	u64 *llregs = &regs;
	u64 *ollregs = &oregs;
	int color = config_get("scr.color");

	if (ps.opened == 0)
		return 0;

	if (mask && mask[0]=='o') { // orig
		memcpy(&regs, &oregs, sizeof(regs_t));
	} else {
		ret =0 ;
		ret = ptrace (PTRACE_GETREGS, ps.tid, 0, &regs);
		if (ret < 0) {
			perror("ptrace_getregs");
			return 1;
		}
	}

	if (rad) {
		cons_printf("f eip @ 0x%llx\n", arch_pc());
		cons_printf("f at  @ 0x%llx\n", llregs[1]);
		cons_printf("f v0  @ 0x%llx\n", llregs[2]);
		cons_printf("f v1  @ 0x%llx\n", llregs[3]);
		cons_printf("f v2  @ 0x%llx\n", llregs[4]);
		cons_printf("f a0  @ 0x%llx\n", llregs[5]);
		cons_printf("f a1  @ 0x%llx\n", llregs[6]);
		cons_printf("f a2  @ 0x%llx\n", llregs[7]);
		cons_printf("f a3  @ 0x%llx\n", llregs[8]);
		cons_printf("f t0  @ 0x%llx\n", llregs[9]);
		cons_printf("f t1  @ 0x%llx\n", llregs[10]);
		cons_printf("f t2  @ 0x%llx\n", llregs[11]);
		cons_printf("f t3  @ 0x%llx\n", llregs[12]);
		cons_printf("f t4  @ 0x%llx\n", llregs[13]);
		cons_printf("f t5  @ 0x%llx\n", llregs[13]);
		cons_printf("f t6  @ 0x%llx\n", llregs[14]);
		cons_printf("f t7  @ 0x%llx\n", llregs[15]);
		cons_printf("f s0  @ 0x%llx\n", llregs[15]);
		cons_printf("f s1  @ 0x%llx\n", llregs[16]);
		cons_printf("f s2  @ 0x%llx\n", llregs[17]);
		cons_printf("f s3  @ 0x%llx\n", llregs[18]);
		cons_printf("f s4  @ 0x%llx\n", llregs[19]);
		cons_printf("f s5  @ 0x%llx\n", llregs[20]);
		cons_printf("f s6  @ 0x%llx\n", llregs[21]);
		cons_printf("f s7  @ 0x%llx\n", llregs[22]);
		cons_printf("f t8  @ 0x%llx\n", llregs[23]);
		cons_printf("f t9  @ 0x%llx\n", llregs[24]);
		cons_printf("f k0  @ 0x%llx\n", llregs[25]);
		cons_printf("f k1  @ 0x%llx\n", llregs[26]);
		cons_printf("f gp  @ 0x%llx\n", llregs[27]);
		cons_printf("f sp  @ 0x%llx\n", llregs[28]);
		cons_printf("f fp  @ 0x%llx ; fp\n", llregs[29]);
		cons_printf("f esp @ 0x%llx ; fp\n", llregs[29]);
		cons_printf("f ra  @ 0x%llx\n", llregs[30]);
	} else {
		if (color) {
			cons_printf("  eip 0x%08llx\e[0m", arch_pc());
			if (llregs[2]!=ollregs[2]) cons_strcat("\e[35m");
			cons_printf("  v0  0x%08llx\e[0m", llregs[2]);
			if (llregs[3]!=ollregs[3]) cons_strcat("\e[35m");
			cons_printf("  v1  0x%08llx\e[0m", llregs[3]);
			if (llregs[4]!=ollregs[4]) cons_strcat("\e[35m");
			cons_printf("  v2  0x%08llx\e[0m\n", llregs[4]);
			//
			if (llregs[29]!=ollregs[29]) cons_strcat("\e[35m");
			cons_printf("  esp 0x%08llx\e[0m", llregs[29]);
			if (llregs[5]!=ollregs[5]) cons_strcat("\e[35m");
			cons_printf("  a0  0x%08llx\e[0m", llregs[5]);
			if (llregs[6]!=ollregs[6]) cons_strcat("\e[35m");
			cons_printf("  a1  0x%08llx\e[0m", llregs[6]);
			if (llregs[7]!=ollregs[7]) cons_strcat("\e[35m");
			cons_printf("  a2  0x%08llx\e[0m", llregs[7]);
			if (llregs[7]!=ollregs[7]) cons_strcat("\e[35m");
			cons_printf("  a3  0x%08llx\e[0m\n", llregs[8]);
			//
			if (llregs[2]!=ollregs[2]) cons_strcat("\e[35m");
			cons_printf("  r2  0x%08llx\e[0m", llregs[2]);
			if (llregs[7]!=ollregs[7]) cons_strcat("\e[35m");
			cons_printf("  r7  0x%08llx\e[0m", llregs[7]);
			if (llregs[11]!=ollregs[11]) cons_strcat("\e[35m");
			cons_printf(" r11  0x%08llx\e[0m", llregs[11]);
			if (llregs[15]!=ollregs[15]) cons_strcat("\e[35m");
			cons_printf(" r15  0x%08llx\e[0m\n", llregs[15]);
			//
			if (llregs[3]!=ollregs[3]) cons_strcat("\e[35m");
			cons_printf("  r3  0x%08llx\e[0m", llregs[3]);
			if (llregs[8]!=ollregs[8]) cons_strcat("\e[35m");
			cons_printf("  r8  0x%08llx\e[0m", llregs[8]);
			if (llregs[12]!=ollregs[12]) cons_strcat("\e[35m");
			cons_printf(" r12  0x%08llx\e[0m", llregs[12]);
			if (llregs[16]!=ollregs[16]) cons_strcat("\e[35m");
			cons_printf(" r16  0x%08llx\e[0m\n", llregs[16]);
			//
			if (llregs[4]!=ollregs[4]) cons_strcat("\e[35m");
			cons_printf("  r4  0x%08llx\e[0m", llregs[4]);
		} else {
			cons_printf("  r0 0x%08llx   r5 0x%08llx   r9 0x%08llx  r13 0x%08llx\n", llregs[0], llregs[5], llregs[9], llregs[13]);
			cons_printf("  r1 0x%08llx   r6 0x%08llx  r10 0x%08llx  r14 0x%08llx\n", llregs[1], llregs[6], llregs[10], llregs[14]);
			cons_printf("  r2 0x%08llx   r7 0x%08llx  r11 0x%08llx  r15 0x%08llx\n", llregs[2], llregs[7], llregs[11], llregs[15]);
			cons_printf("  r3 0x%08llx   r8 0x%08llx  r12 0x%08llx  r16 0x%08llx\n", llregs[3], llregs[8], llregs[12], llregs[16]);
			cons_printf("  r4 0x%08llx   ", llregs[4]);

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
	return NULL;
}

int arch_backtrace()
{
	// TODO
	return 0;
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
	return NULL;
}

void *arch_alloc_page(int size, int *rsize)
{
	return NULL;
}

addr_t arch_mmap(int fd, int size, u64 addr) //int *rsize)
{
	return NULL;
}

addr_t arch_get_sighandler(int signum)
{
	return NULL;
}

addr_t arch_set_sighandler(int signum, u64 handler)
{
	return NULL;
}

addr_t arch_get_entrypoint()
{
	unsigned long addr;
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
	u64 *llregs = &regs;
	int ret ;

	memset(&regs, '\0', sizeof(regs));
	ret = ptrace (PTRACE_GETREGS, ps.tid, 0, &regs);

	if (ret < 0) {
		perror("ptrace_getregs");
		return 1;
	}
	return 0;

	if (reg[0]=='r') {
		int r = atoi(reg+1);
		if (r>32)r = 32;
		if (r<0)r = 0;
		return llregs[r];
	}
}
