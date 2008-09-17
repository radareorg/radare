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

#include "libps2fd.h"
#include "debug.h"
#include "arch/arch.h"
#include "../radare.h"
#include "../config.h"

int debug_bp(const char *str)
{
	u64 addr = 0;
	const char *ptr = str;
	const char *type;
	int hwbp = BP_NONE;
	int num;

	hwbp = (int)config_get_i("dbg.hwbp");
	switch(str[0]) {
	case 's':
		hwbp = BP_SOFT;
		break;
	case 'h':
		hwbp = BP_HARD;
		break;
	case '?':
		cons_printf("Usage: !bp[?|s|h|*] ([addr|-addr])\n"
		"  !bp [addr]    add a breakpoint\n"
		"  !bp -[addr]   remove a breakpoint\n"
		"  !bp*          remove all breakpoints\n"
		"  !bps          software breakpoint\n"
		"  !bph          hardware breakpoint\n");
		return 0;
	}

	while ( *ptr && *ptr != ' ') ptr = ptr + 1;
	while ( *ptr && *ptr == ' ' ) ptr = ptr + 1;

	type = ptr;

	while ( *type && *type != ' ') type = type + 1;
	while ( *type && *type == ' ' ) type = type + 1;

	switch(ptr[0]) {
	case '-':
		addr = get_offset(ptr+1);
		flag_clear_by_addr(addr);
		if(debug_bp_rm_addr(addr) == 0)
			eprintf("breakpoint at 0x%x dropped\n", addr);
		break;
	case '+': // relative from eip
		addr = config.seek + get_offset(ptr+1);
		flag_set("breakpoint", addr, 3);
		debug_bp_set(NULL, addr, hwbp);
		eprintf("new breakpoint at 0x%lx\n", addr);
		break;
	case '*':
		eprintf("%i breakpoint(s) removed\n", debug_bp_rms());
		break;
	default:
		addr = get_offset(ptr);
		if (ptr[0]==0 || addr == 0)
			debug_bp_list();
		else {
			flag_set("breakpoint", addr, 3);
			num = debug_bp_set(NULL, addr, hwbp);
			eprintf("new %s breakpoint %d at 0x%lx\n", hwbp?"hw":"sw", num, addr);
		}
		break;
	}

	return 0;
}

struct bp_t *debug_bp_get(addr_t addr)
{
	int i = 0;

	for(i = 0; i < MAX_BPS; i++) 
		if(ps.bps[i].addr == addr)
			return &ps.bps[i]; 

	//printf("BP NOT FOUND FOR %08x\n", addr);
	return  NULL;
}

/* HACK: save a hardware/software breakpoint */
int debug_bp_restore_after()
{
	struct bp_t *bp;
	u64 addr = arch_pc(ps.tid); // x86
	int bpsize = arch_bpsize();

	/* hardware */
	bp = debug_bp_get(addr);
	if (bp!=NULL && bp->hw) {
		eprintf("HW breakpoint hit!\n");
		bp->count++;
		return 0;
	}

	/* software */
	bp = debug_bp_get(addr-bpsize);
	if (bp!=NULL) {
		bp->count++;
		eprintf("post-breakpoint restored %08llx\n", addr);
		arch_jmp(addr-bpsize);
	}

	return 0;
}

/* called before the step */
int debug_bp_restore(int pos)
{
	struct bp_t *bp;
	u64 addr = arch_pc(ps.tid); // x86

	if (pos==-1)
		bp = debug_bp_get(addr);
	else	bp = debug_bp_get_num(pos);

	if (bp == NULL) {
		bp = debug_bp_get(addr-1);
		if (bp==NULL) {
	//	eprintf("CaNnot restore no bp found here :/ %08llx\n", addr);
			return 0;
		}
		printf("pre-Breakpoint -1 %08llx\n", addr);
	} else {
		printf("pre-Breakpoint restored %08llx\n", addr);
	}
	if (bp == NULL || WS(bp)==NULL)
		return 0;
	//printf("go forward with bp found here !! %08llx and bp = %08x\n", addr, bp);
        if(WS(event) == BP_EVENT) {
		eprintf("restore: restoring bp at %llx\n", addr);
		arch_restore_bp(WS(bp));
		return 1;
        }
	return arch_restore_bp(WS(bp));
}

int debug_bp_rm(u64 addr, int type)
{
	struct bp_t *bp;
	int ret;

eprintf("RM BP!!\n");
	bp = debug_bp_get(addr);	
	if (bp == NULL) {
		eprintf("debug_bp_rm: No breakpoint found at this address\n");
		return -1;
	}
	if(bp->hw)
		ret = arch_bp_rm_hw(bp);
	else	ret = arch_bp_rm_soft(bp);

	if(ret < 0)
		return ret;

	flag_clear_by_addr((addr_t)addr); //"breakpoint", addr, 1);

	bp->addr = 0;
	ps.bps_n--;

	return 0;
}

struct bp_t *debug_bp_get_num(int num)
{
	if (num>=0&& num<ps.bps_n)
		return &ps.bps[num];
	return NULL;
}

int debug_bp_rm_num(int num)
{
#warning XXX THIS IS BUGGY! num != addr, addr_t != int !!
	if (num>=0&& num<ps.bps_n)
		return debug_bp_rm(ps.bps[num].addr, ps.bps[num].hw);
	return 0;
}

int debug_bp_rm_addr(u64 addr)
{
	return debug_bp_rm(addr, 0);
}

int debug_bp_set(struct bp_t *bp, u64 addr, int type)
{
	int i, ret, bp_free = -1;

	if (addr == 0)
		addr = arch_pc(ps.tid); // WS_PC();

	/* all slots busy */
	if(ps.bps_n == MAX_BPS)
		return -2;

	/* search for breakpoint */
	for(i=0;i < MAX_BPS;i++) {
		/* breakpoint found it */
		if(ps.bps[i].addr == addr) {
			if(bp)
				bp = &ps.bps[i];
			return 0;
		}

		if ((bp_free == -1) && (ps.bps[i].addr == 0))
			bp_free = i;
	}
/* TODO: hardware registers are not supported everywhere. bp->hw = 0 for the rest! */

	ret = -1;

	switch(type) {
	case BP_HARD:
		ret = arch_set_bp_hw(&ps.bps[bp_free], addr);
		ps.bps[bp_free].hw = 1;
		break;
	case BP_SOFT:
	default:
		ret = arch_set_bp_soft(&ps.bps[bp_free], addr);
		ps.bps[bp_free].hw = 0;
		break;
	}

	if(ret < 0) {
		if((ret = arch_set_bp_hw(&ps.bps[bp_free], addr)) >= 0 )
			ps.bps[bp_free].hw = 1;
		else if((ret = arch_set_bp_soft(&ps.bps[bp_free], addr)) >= 0)
			ps.bps[bp_free].hw = 0;

		if(ret < 0) {
			ps.bps[bp_free].addr = 0;
			return ret;
		}
	}

	ps.bps[bp_free].addr = addr;
	if(bp)
		bp = &ps.bps[bp_free];

	ps.bps_n++;

	return bp_free;
	//return bp_free;
}

void debug_bp_reload_all()
{
        int bps;
        int i;

	// XXX must free the lists before
        INIT_LIST_HEAD(&(ps.th_list));       
        INIT_LIST_HEAD(&(ps.map_mem));
        INIT_LIST_HEAD(&(ps.map_reg));

        bps = 0;
        for(i = 0; bps != ps.bps_n; i++) {
                if(ps.bps[i].addr != 0) {
                        if(ps.bps[i].hw) {
                                if(arch_set_bp_hw(&ps.bps[i], ps.bps[i].addr) < 0)
                                	eprintf(":debug_reload_bps failed "
                                        	"set breakpoint HARD at 0x%x\n",
						 ps.bps[i].addr);
			} else {
				if(arch_set_bp_soft(&ps.bps[i], ps.bps[i].addr) != 0)
                                	eprintf(":debug_reload_bps failed "
                                        	"set breakpoint SOFT at 0x%x\n",
						 ps.bps[i].addr);
                        }
                        bps++;
                }
        }
}

void debug_bp_list()
{
	char str[512];
	int bps;
	int i;

	if (ps.bps_n) {
		eprintf("breakpoints:\n");
		bps = ps.bps_n;
		for(i = 0; i < MAX_BPS && bps > 0; i++) {
			if(ps.bps[i].addr > 0) { 
				string_flag_offset(str, ps.bps[i].addr);
				eprintf(" 0x%08llx %s %s (hits=%d)\n",
					ps.bps[i].addr, (ps.bps[i].hw)?"HARD":"SOFT",
					str, ps.bps[i].count); 
				bps--;	
			}
		}
	} else
		eprintf("breakpoints not set\n");
}

