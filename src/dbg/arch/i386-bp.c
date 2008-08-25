/*
 * Copyright (C) 2007, 2008
 *       th0rpe <nopcode.org>
 *       pancake <youterm.com>
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

#include "../libps2fd.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <x86/udis86/types.h>
#include <x86/udis86/extern.h>
#include "../mem.h"
#include "i386.h"

static int ins_c;
static char *ins_buf;
extern struct regs_off roff[];

/* software breakpoints */
struct arch_bp_t arch_x86_bps[] = {
	{ "\xCC", 1 },
	{ "\xCD\x03", 2 },
	{ NULL, 0 }
};

/* hardware breakpoints */
// TODO: unsigned long -> u64
// XXX: 64 bit hw registers working properly??
//
#ifdef __linux__

static unsigned long dr_get (int reg)
{
  	return ptrace (PTRACE_PEEKUSER, ps.tid,
                  offsetof(struct user, u_debugreg[reg]), 0);
}

static int dr_set (int reg, unsigned long val)
{
  	return ptrace (PTRACE_POKEUSER, ps.tid, offsetof(struct user, u_debugreg[reg]), val);
}

#elif __FreeBSD__

static unsigned long dr_get (int reg)
{
	unsigned long val;
	int ret;
	struct dbreg dbr;

  	ret = ptrace (PT_GETDBREGS, ps.tid, &dbr, sizeof(struct dbreg));

	if (reg>=0 && reg<=7)
		return dbr.dr[reg];

	return 0;
}

static int dr_set (int reg, unsigned long val)
{
	int ret;
	struct dbreg dbr;

  	ret = ptrace (PT_GETDBREGS, ps.tid, &dbr, sizeof(struct dbreg));
	if (ret == -1)
		return -1;

	if (reg>=0 && reg<=7)
		dbr.dr[reg] = val;
	else return -1;

  	return ptrace (PT_SETDBREGS, ps.tid, &dbr, sizeof(struct dbreg));
}

#elif __WINDOWS__

static unsigned long dr_get(int reg)
{
	regs_t regs;
	unsigned int off;

	debug_getregs(ps.tid, &regs);

	if(reg == 7)
		off = offsetof(CONTEXT, Dr7);
	else	off = sizeof(DWORD) + (reg * sizeof(DWORD));

	return debug_get_regoff(&regs, off);
}

static int dr_set(int reg, unsigned long val)
{
	regs_t regs;
	unsigned int off;

	if(reg == 7)
		off = offsetof(CONTEXT, Dr7);
	else	off = sizeof(DWORD) + (reg * sizeof(DWORD));

	debug_getregs(ps.tid, &regs);
	debug_set_regoff(&regs, (int)off, val);
	debug_setregs(ps.tid, &regs);

	debug_getregs(ps.tid, &regs);
	/*
	printf("REG_RE_VAL3: 0x%x\n", regs.Dr3);
	printf("REG_CONTROL: 0x%x\n", regs.Dr7);
	*/
	return 0;
}

#else

/* NOT YET IMPLEMENTED: USE u64!!! */
static unsigned long dr_get(int reg)
{
	return 0L;
}

static int dr_set(int reg, unsigned long val)
{
	return 0;
}

#endif

inline static void dr_set_control (unsigned long control)
{
	dr_set(DR_CONTROL, control);
}

inline static unsigned long dr_get_control ()
{
	return dr_get(DR_CONTROL);
}

inline static void dr_set_addr (int regnum, unsigned long addr)
{
	dr_set(regnum, addr);
}

inline static void dr_reset_addr (int regnum)
{
	dr_set(regnum, 0L);
}

inline static unsigned long dr_get_status (void)
{
	return dr_get(DR_STATUS);
}

void dr_init()
{
	int i;
	for(i = 0; i <= DR_CONTROL; i++)
		dr_set(i, 0);
}

int arch_set_wp_hw_n(int dr_free, unsigned long addr, int type)
{
	int i;
	unsigned long control = dr_get_control();

	if (dr_free == -1) {
		for(i = 0; i < DR_NADDR; i++) {

			if(dr_get(i)==0) { //I386_DR_VACANT(control, i)) {
				dr_free = i;
			} else {
				/* check if exist a watchpoint
				   with same address and access type  */
				if(dr_get(i) == addr && 
				   I386_DR_GET_RW_LEN(control, i) == type )
					return i;
			}
		}
		if(dr_free == -1)
			return -1;
	} else
	if (dr_free<0||dr_free>3)
		return -2;

	/* set access type */
	I386_DR_SET_RW_LEN(control, dr_free, type);

	/* local watchpoint (current process only is active) */
  	I386_DR_LOCAL_ENABLE(control, dr_free);

  	control |= DR_LOCAL_SLOWDOWN;
  	control &= I386_DR_CONTROL_MASK;

	/* set address watchpoint */
  	dr_set(dr_free, addr);

	/* upgrade control register */
	dr_set_control(control);

	return dr_free;
}


// TODO: Move to macros in .h
int arch_set_wp_hw(unsigned long addr, int type)
{
	return arch_set_wp_hw_n(-1, addr, type);
}

int arch_set_bp_hw(struct bp_t *bp, unsigned long addr)
{
	return arch_set_wp_hw(addr, DR_RW_EXECUTE);
}

int arch_bp_rm_hw(struct bp_t *bp)
{
	int i; 
	unsigned long addr;

	addr = bp->addr;

	for(i = 0; i < 4; i++) {
		if(dr_get(i)  == addr) {
			dr_set(i, 0);
			return 0;
		}
	}

	return -1;
}

#define flags_to_char(x) (x==DR_RW_EXECUTE)?'x':(x==DR_RW_READ)?'r':(x==DR_RW_WRITE)?'w':'?'

void dr_list()
{
	int i, flags;
	unsigned long addr, control;
	
	control = dr_get_control();

	for(i=0;i<4;i++) {
		addr    = dr_get(i);
		flags   = I386_DR_GET_RW_LEN(control, i);
		fprintf(stderr, "DR%d 0x%08x %c\n", i, (unsigned int)addr, flags_to_char(flags));
	}
}

static int arch_bp_hw_state(unsigned long addr, int enable)
{
	unsigned long control;
	int i;

	control = dr_get_control();

	for(i = 0; i < DR_NADDR; i++)
		if(dr_get(i) == addr)  {
			if(enable)
				I386_DR_ENABLE(control, i);
			else	I386_DR_DISABLE(control, i);
			dr_set_control(control);
			return 0;
		}
			
	return -1;
}

int input_hook_x(ud_t* u)
{
	char c;
	if(!ins_c)
		return UD_EOI;

	c = *ins_buf;
	ins_c--;
	ins_buf++;
	return c;
}

int get_len_ins(char *buf, int len)
{
	ud_t ud_obj;

        ud_init(&ud_obj);
        ud_set_mode(&ud_obj, 32);
	ud_set_syntax(&ud_obj, UD_SYN_INTEL);
	ins_buf = buf;
	ins_c = len;
	ud_set_input_hook(&ud_obj, input_hook_x);

	return ud_disassemble(&ud_obj);
}

int arch_set_bp_soft(struct bp_t *bp, unsigned long addr)
{
	char breakpoint = '\xCC';

	debug_read_at(ps.tid, bp->data, 16, addr);
	bp->len = get_len_ins((char *)bp->data, 16);
	debug_write_at(ps.tid, &breakpoint, 1, addr);

	return 0;
}

int arch_bp_rm_soft(struct bp_t *bp)
{
	debug_write_at(ps.tid, bp->data, bp->len, bp->addr);
	return 0;
}

/* software breakpoints */
inline int arch_bp_soft_enable(struct bp_t *bp)
{
	char breakpoint = '\xCC'; // XXX : support to choose which bp instruction use

	return debug_write_at(ps.tid, &breakpoint, 1, bp->addr);
}

inline int arch_bp_soft_disable(struct bp_t *bp)
{
	return debug_write_at(ps.tid, bp->data, bp->len, bp->addr);
}

/* hardware breakpoints */
inline int arch_bp_hw_enable(struct bp_t *bp)
{
	return arch_bp_hw_state(bp->addr, 1);
}

inline int arch_bp_hw_disable(struct bp_t *bp)
{
	return arch_bp_hw_state(bp->addr, 0);
}

/* DRY! MOVE TO BP.C IT IS CASI ARCH INDEPENDENT OW YEAH */
int arch_restore_bp(struct bp_t *bp)
{
	regs_t	regs;

printf("arch_restore_bp\n");
	if (bp == NULL) {
		eprintf("WARNING: Cannot restore bp at eip = 0x%08llx\n",(u64)arch_pc());
		return 0;
	}

	if(WS(bp)->hw) {
printf("restore hard bp\n");
		arch_bp_hw_disable(bp);
		debug_os_steps();
		debug_dispatch_wait();
		arch_bp_hw_enable(bp);
	} else {
printf("restore soft bp\n");
	//	arch_bp_soft_disable(bp);
		debug_getregs(ps.tid, &regs);
//	  arch_jmp(arch_pc()-1);
//	  arch_jmp(arch_pc()-1);
#define CODE_GUAI 0
// CODE EXPERIMENTAL TO CLEAN THE CACHE (NOT NEEDED!! NOISE!!! )
#if 0
	{
		char buf[4];
		u64 off = arch_pc()-2;
		arch_jmp(off);
		arch_bp_soft_disable(bp);
		debug_read_at(ps.tid, buf, 4, off);
		debug_write_at(ps.tid, "\x90", 1, off);
		debug_os_steps();
		R_EIP(regs) = R_EIP(regs) - 1;
		debug_dispatch_wait();
		debug_os_steps();
		debug_dispatch_wait();
//	printf("WRITE 4 bytes (%02x%02x%02x%02x) %08llx\n", buf[0], buf[1], buf[2], buf[3], off);
		debug_write_at(ps.tid, buf, 4, off);
//		arch_bp_soft_enable(bp);
	}
#endif
#if 0
		R_EIP(regs) = R_EIP(regs) - 1;
		printf("EIP = %08x\n", R_EIP(regs));
		debug_setregs(ps.tid, &regs);
		debug_os_steps();
		debug_dispatch_wait();
#endif
	//	arch_bp_soft_enable(bp);
#if 0
#if ARCH_I386

	if (!bp->hw)
	  arch_jmp(arch_pc()-1);
	else
#else
	  arch_jmp(arch_pc()-4);
		debug_getregs(ps.tid, &regs);
#endif
#endif
	}

	return 0;
}

struct bp_t *arch_stopped_bp()
{
        int i;
	int bps = ps.bps_n;
	u64 addr;
	struct bp_t *bp_w = 0, *bp_s = 0;

	addr = R_EIP(WS(regs));

        for(i = 0; i < MAX_BPS && bps > 0; i++) {
                if(ps.bps[i].addr != 0) {
			if(ps.bps[i].hw) {
				if(ps.bps[i].addr == addr)
					bp_w = &ps.bps[i];
			} else {
				if(ps.bps[i].addr == addr - ps.bps[i].len) {
//printf("BARATA IT IS SOFT AND needs to 0x%08llx\n", arch_pc());
					bp_s = &ps.bps[i];
				}
			}
			bps--;	
		}
	}

	if(bp_w && bp_s)
		return bp_s;
	
	if(bp_w)
		return bp_w;

        return  bp_s;
}
