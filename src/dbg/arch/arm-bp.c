/*
 * Copyright (C) 2007, 2008
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

// GXEMUL USES 0x00004040


// XXX must alias pc, lr, sp, etc..
struct regs_off roff[] = {
	{"r0", 0},
	{"r1", 4},
	{"r2", 8},
	{"r3", 16},
	{"r4", 24},
	{"r5", 32},
	{"r6", 48},
	{"r7", 56},
	{"r8", 64},
	{"r9", 72},
	{"r10", 80},
	{"r11", 88},
	{"r12", 96},
	{"r13", 104},
	{"r14", 112},
	{"r15", 120},
	{"r16", 128},
	{0, 0}
};

unsigned char *arm_bps[] = {
 "\x01\x00\x9f\xef", // le         - linux only? (undefined instruction)
 "\xef\x9f\x00\x01", // be
 "\xfe\xde\xff\xe7", // arm-le     - from a gdb patch
 "\xe7\xff\xde\xfe", // arm-be
 "\xf0\x01\xf0\xe7", // eabi-le    - undefined instruction - for all kernels
 "\xe7\xf0\x01\xf0", // eabi-be
 "\xde\x01",         // thumb-le
 "\x01\xde",         // thumb-be
 "\xfe\xdf",         // arm-thumb-le
 "\xdf\xfe"          // arm-thumb-be
};

/*
static unsigned long dr_get(int reg);
static int dr_set(int reg, unsigned long val);
inline static unsigned dr_get_control ()
inline static void dr_set_addr (int regnum, unsigned long addr)
inline static void dr_set_control (unsigned long control)
inline static void dr_set_addr (int regnum, unsigned long addr)
*/
int arch_bpsize()
{
	return (arm_mode==16)?2:4;
}

int arch_set_wp_hw_n(int dr_free, unsigned long addr, int type)
{
	return -1;
}

int arch_set_wp_hw(unsigned long addr, int type)
{
	return -1;
}

/* hook hardware bps to software ones..arm can't :/ */
int arch_set_bp_hw(struct bp_t *bp, unsigned long addr)
{
	return arch_set_bp_soft(bp, addr);
}

int arch_bp_rm_hw(struct bp_t *bp)
{
	return arch_bp_rm_soft(bp);
}

/*
static int arch_bp_hw_state(unsigned long addr, int enable)
{
}
*/

/*
int input_hook_x(void* u)
{
}
*/

int get_len_ins(char *buf, int len)
{
	return 4;
}

int arch_set_bp_soft(struct bp_t *bp, unsigned long addr)
{
	int endian = config_get("cfg.endian");
	char *breakpoint = arm_bps[endian&1];

	debug_read_at(ps.tid, bp->data, 16, addr);
	bp->len = get_len_ins(bp->data, 16);
	debug_write_at(ps.tid, breakpoint, 4, addr);

	return 0;
}

int arch_bp_rm_soft(struct bp_t *bp)
{
	return debug_write_at(ps.tid, bp->data, bp->len, bp->addr);
}

inline int arch_bp_hw_enable(struct bp_t *bp)
{
	return arch_bp_soft_enable(bp);
}

inline int arch_bp_hw_disable(struct bp_t *bp)
{
	return arch_bp_soft_disable(bp);
}

inline int arch_bp_soft_enable(struct bp_t *bp)
{
	int endian = config_get("cfg.endian");
	char *breakpoint = arm_bps[endian&1];
	debug_write_at(ps.tid, breakpoint, bp->len, bp->addr);
}

inline int arch_bp_soft_disable(struct bp_t *bp)
{
	return debug_write_at(ps.tid, bp->data, bp->len, bp->addr);
}

int arch_restore_bp(struct bp_t *bp)
{
	regs_t	regs;

	arch_bp_soft_disable(bp);
	debug_getregs(ps.tid, &regs);
	regs[15] -= 4; // pc-=4;

	debug_setregs(ps.tid, &regs);
	debug_os_steps();

	debug_dispatch_wait();

	debug_getregs(ps.tid, &regs);

	//arch_bp_soft_enable(bp);
}

// XXX this func is dupped! should not be here (read i386-bp.c
struct bp_t *arch_stopped_bp()
{
        int i;
	int bps = ps.bps_n;
	unsigned long addr;
	struct bpt_t *bp_w = 0, *bp_s = 0;

	addr = arch_pc();

        for(i = 0; i < MAX_BPS && bps > 0; i++) {
                if(ps.bps[i].addr != 0) {
			if(ps.bps[i].addr == addr - 4)
				bp_s = &ps.bps[i];
			bps--;	
		}
	}

	if(bp_w && bp_s)
		return bp_s;
	
	if(bp_w)
		return bp_w;

        return  bp_s;
}
