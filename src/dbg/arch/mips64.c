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

//#define REG_RA 31 // Return address
//#define REG_K0 25 // Stack pointer
//#define REG_SP 29 // Stack pointer

int arch_is_fork()
{
	return 0;
}

u64 arch_syscall(int pid, int sc, ...)
{
        long long ret = (off_t)-1;
	return ret;
}

/* clear high 64 bits */
static u64 reg(u64 reg)
{
	if (reg & 0xFFFFFFFF00000000LL)
		return reg & 0xFFFFFFFF;
	return reg;
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
	fprintf(fd, "r00 0x%08llx\n", reg(llregs[0]));
	fprintf(fd, "r01 0x%08llx\n", reg(llregs[1]));
	fprintf(fd, "r02 0x%08llx\n", reg(llregs[2]));
	fprintf(fd, "r03 0x%08llx\n", reg(llregs[3]));
	fprintf(fd, "r04 0x%08llx\n", reg(llregs[4]));
	fprintf(fd, "r05 0x%08llx\n", reg(llregs[5]));
	fprintf(fd, "r06 0x%08llx\n", reg(llregs[6]));
	fprintf(fd, "r07 0x%08llx\n", reg(llregs[7]));
	fprintf(fd, "r08 0x%08llx\n", reg(llregs[8]));
	fprintf(fd, "r09 0x%08llx\n", reg(llregs[9]));
	fprintf(fd, "r10 0x%08llx\n", reg(llregs[10]));
	fprintf(fd, "r11 0x%08llx\n", reg(llregs[11]));
	fprintf(fd, "r12 0x%08llx\n", reg(llregs[12]));
	fprintf(fd, "r13 0x%08llx\n", reg(llregs[13]));
	fprintf(fd, "r14 0x%08llx\n", reg(llregs[14]));
	fprintf(fd, "r15 0x%08llx\n", reg(llregs[15]));
	fprintf(fd, "r16 0x%08llx\n", reg(llregs[16]));
	fprintf(fd, "r17 0x%08llx\n", reg(llregs[17]));
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

int arch_mprotect(u64 addr, unsigned int size, int perms)
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
	//return ptrace(PTRACE_POKEUSER, ps.pid, PTRACE_PC, (unsigned int)ptr);
	u8 buf[4096];
	unsigned int ptr32 = (unsigned int )ptr;
	int ret = ptrace(PTRACE_GETREGS, ps.tid, 0, &buf);
	memcpy(buf+272, &ptr32, sizeof(ptr32));
	return ptrace(PTRACE_SETREGS, ps.tid, 0, &buf);
}

u64 arch_pc()
{
#if 0
	int ret;
	regs_t regs;
	unsigned long *llregs = &regs;

	memset(&regs, '\0', sizeof(regs));
	ret = ptrace (PTRACE_GETREGS, ps.tid, 0, &regs);
	if (ret == -1) {
		perror("ptrace_getregs==-1\n");
		return 0;
	}
	return llregs[25];
#endif

	u64 addr;
	u8 buf[1024];
	regs_t regs;
	int i, ret; 
	unsigned long long *llregs = &regs;
		
//	memset(buf, '\0', 100);
	//ret = ptrace (PTRACE_PEEKUSER, ps.tid, 64, &buf);
	//#define offsetof(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)
	//ret = ptrace(PTRACE_GETREGS, ps.tid, struct user, u_tsize), &buf);
	ret = ptrace(PTRACE_GETREGS, ps.tid, 0, &buf);
	if (ret == -1)
		return -1;
#if 0
		16 /* 0 - 31 are integer registers, 32 - 63 are fp registers.  */
		17 #define FPR_BASE        32
		18 #define PC              64
		19 #define CAUSE           65
		20 #define BADVADDR        66
		21 #define MMHI            67
		22 #define MMLO            68
		23 #define FPC_CSR         69
		24 #define FPC_EIR         70
#endif
		//eprintf("PC %08x\n", llregs[64]);
#if 0
	for(i=270;i<290;i++) {
		eprintf("%02x ", buf[i]);
	}
#endif
	addr = 0;
	memcpy(&addr, buf+272, sizeof(u64));
	return addr;
}

int arch_set_register(char *reg, char *value)
{
	int ret;
	regs_t regs;
	unsigned long long *llregs = &regs;

	if (ps.opened == 0)
		return 0;

	ret = ptrace(PTRACE_GETREGS, ps.tid, NULL, &regs);
	if (ret < 0) return 1;

	ret = vm_get_reg(reg);
	if (ret == -1) {
		eprintf("Invalid register name '%s'\n", reg);
		return 1;
	}
	printf("reg idx = %d\n", ret);
	llregs[ret] = (int)get_offset(value);

	ret = ptrace(PTRACE_SETREGS, ps.tid, NULL, &regs);

	return 0;
}


int arch_print_fpregisters(int rad, const char *mask)
{
        u64 ptr[128];
        int i, ret = ptrace(PTRACE_GETFPREGS, ps.tid, 0, &ptr);
        if (rad)
                for(i=0;i<32;i+=2)
                        cons_printf("f f%d @ 0x%08llx\n", i, ptr[i]);
        else {
                for(i=0;i<16;i++)
                        cons_printf("f%02d: 0x%016llx%c", i<<1, ptr[i*2], (i%2)?'\n':'\t');
                /* packed autodetect */
                for(i=0;i<16;i++) {
                        u64 val = ptr[i*2];
                        if (val != -1 && val != 0 && ((val & 0xf000f000f000f000LL)==0)) {
                                u16 *val16 = (u16*)&val;
                                cons_printf("f%02d: packed (%4d, %4d, %4d, %4d)\n",  i<<1,
                                        val16[0], val16[1], val16[2], val16[3]);
                        }
                }
        }

        return ret;
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

// TODO: control 63-32 fffff bits and drop them :)
int arch_print_registers(int rad, const char *mask)
{
	int ret;
	regs_t regs;
	unsigned long long *llregs = &regs;
	unsigned long long *ollregs = &oregs;
	unsigned char *qregs = &regs;
	int color = config_get("scr.color");

	if (ps.opened == 0)
		return 0;

	if (mask && mask[0]=='o') { // orig
		memcpy(&regs, &oregs, sizeof(regs_t));
	} else {
		memset(&regs, '\0', sizeof(regs));
		ret = ptrace (PTRACE_GETREGS, ps.tid, 0, &regs);
		if (ret < 0) {
			perror("ptrace_getregs");
			return 1;
		}
		//for(ret=0;ret<(45*8);ret++) printf("%d:%02x ", ret, qregs[ret]); printf("\n");
	}

	if (rad) {
		cons_printf("f pc  @ 0x%llx\n", reg(arch_pc())); // dupgetregs
		cons_printf("f at  @ 0x%llx\n", reg(llregs[1])); // is this t1 ?!?!?
		cons_printf("f v0  @ 0x%llx\n", reg(llregs[2]));
		cons_printf("f v1  @ 0x%llx\n", reg(llregs[3]));

		cons_printf("f a0  @ 0x%llx\n", reg(llregs[4]));
		cons_printf("f a1  @ 0x%llx\n", reg(llregs[5]));
		cons_printf("f a2  @ 0x%llx\n", reg(llregs[6]));
		cons_printf("f a3  @ 0x%llx\n", reg(llregs[7]));

		cons_printf("f t0  @ 0x%llx\n", reg(llregs[8]));
		cons_printf("f t1  @ 0x%llx\n", reg(llregs[9]));
		cons_printf("f t2  @ 0x%llx\n", reg(llregs[10]));
		cons_printf("f t3  @ 0x%llx\n", reg(llregs[11]));
		cons_printf("f t4  @ 0x%llx\n", reg(llregs[12]));
		cons_printf("f t5  @ 0x%llx\n", reg(llregs[13]));
		cons_printf("f t6  @ 0x%llx\n", reg(llregs[14]));
		cons_printf("f t7  @ 0x%llx\n", reg(llregs[15]));

		cons_printf("f s0  @ 0x%llx\n", reg(llregs[16]));
		cons_printf("f s1  @ 0x%llx\n", reg(llregs[17]));
		cons_printf("f s2  @ 0x%llx\n", reg(llregs[18]));
		cons_printf("f s3  @ 0x%llx\n", reg(llregs[19]));
		cons_printf("f s4  @ 0x%llx\n", reg(llregs[20]));
		cons_printf("f s5  @ 0x%llx\n", reg(llregs[21]));
		cons_printf("f s6  @ 0x%llx\n", reg(llregs[22]));
		cons_printf("f s7  @ 0x%llx\n", reg(llregs[23]));

		cons_printf("f t8  @ 0x%llx\n", reg(llregs[24]));
		cons_printf("f t9  @ 0x%llx\n", reg(llregs[25]));

		cons_printf("f k0  @ 0x%llx\n", reg(llregs[26])); // k0 - the context where it was pwned
		cons_printf("f k1  @ 0x%llx\n", reg(llregs[27]));

		cons_printf("f gp  @ 0x%llx\n", reg(llregs[28]));
		cons_printf("f sp  @ 0x%llx\n", reg(llregs[29]));
		cons_printf("f esp  @ sp\n");
		cons_printf("f eip  @ pc\n");

		cons_printf("f fp  @ 0x%llx ; fp\n", reg(llregs[30]));
		cons_printf("f ra  @ 0x%llx\n", reg(llregs[31]));

		cons_printf("f fir @ 0x%llx\n", reg(llregs[32]));
		cons_printf("f fsr @ 0x%llx\n", reg(llregs[33]));
		cons_printf("f pc_reg @ 0x%llx\n", reg(llregs[34]));
		cons_printf("f bad @ 0x%llx\n", reg(llregs[35]));
		cons_printf("f sr @ 0x%llx\n", reg(llregs[36]));
		cons_printf("f cause @ 0x%llx\n", reg(llregs[37]));
		// 32 == fir
		// 33 == fsr
		// 34 == pc
		// 35 == bad
		// 36 == sr
		// 37 == cause
	} else {
		#define PRINT_REG(name, tail, idx) \
			if (!color) cons_printf("  %4s  0x%08llx"tail, name, reg(llregs[idx])); \
			else { if (llregs[idx] != ollregs[idx]) cons_strcat("\x1b[35m"); \
			cons_printf("  %4s  0x%08llx\x1b[0m"tail, name, reg(llregs[idx])); }
		cons_printf("    pc  0x%08llx\x1b[0m", arch_pc());
		PRINT_REG("ra", "", 31);
		PRINT_REG("fp", "", 30);
		PRINT_REG("gp", "", 28);
		PRINT_REG("at", "\n", 1);
		/* -- */
		PRINT_REG("sp", "", 29);
		PRINT_REG("a0", "", 4);
		PRINT_REG("a1", "", 5);
		PRINT_REG("a2", "", 6);
		PRINT_REG("a3", "\n", 7);
		/* -- */
		PRINT_REG("k0", "", 26);
		PRINT_REG("k1", "", 27);
		PRINT_REG("v0", "", 2);
		PRINT_REG("v1", "\n", 3);


		PRINT_REG("s0", "", 16);
		PRINT_REG("s1", "", 17);
		PRINT_REG("s2", "", 18);
		PRINT_REG("s3", "", 19);
		PRINT_REG("s4", "\n", 20);

		PRINT_REG("s5", "", 21);
		PRINT_REG("s6", "", 22);
		PRINT_REG("s7", "", 23);

		PRINT_REG("bad", "", 35);
		PRINT_REG("caus", "\n", 37);

		PRINT_REG("t0", "", 8);
		PRINT_REG("t1", "", 9);
		PRINT_REG("t2", "", 10);
		PRINT_REG("t3", "", 11);
		PRINT_REG("t4", "\n", 12);

		PRINT_REG("t5", "", 13);
		PRINT_REG("t6", "", 14);
		PRINT_REG("t7", "", 15);
		PRINT_REG("t8", "", 24);
		PRINT_REG("t9", "\n", 25);
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
	//unsigned char bp[4]="\x00\x00\x00\x0d";
	unsigned char bp[4]="\x0d\x00\x00\x00";
	debug_write_at(ps.tid, bp, 4, addr);
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

u64 get_reg(char *reg)
{
	regs_t regs;
	u64 *llregs = &regs; // 45 elements of 64 bits here
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
