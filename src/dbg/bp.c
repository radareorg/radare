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
#include "arch/arch.h"
#include "../radare.h"
#include "../config.h"

int debug_bp(const char *str)
{
	u64 addr = 0;
	const char *ptr = str;
	const char *type;
	int bptype = BP_NONE;

	switch(str[0]) {
	case 's':
		bptype = BP_SOFT;
		break;
	case 'h':
		bptype = BP_HARD;
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

	if (bptype == BP_NONE) {
		type = config_get("dbg.bptype");
		if (type) {
			if (type[0]=='s')
				bptype = BP_SOFT;
			else
			if (type[0]=='h')
				bptype = BP_HARD;
		}
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
		debug_bp_set(NULL, addr, bptype);
		eprintf("new breakpoint at 0x%lx\n", addr);
		break;
	case '*':
		eprintf("%i breakpoint(s) removed\n", debug_bp_rms());
		break;
	default:
		addr = get_offset(ptr);
		if (ptr[0]==0 || addr == 0)
			debug_print_bps();
		else {
			flag_set("breakpoint", addr, 3);
			debug_bp_set(NULL, addr, bptype);
			eprintf("new breakpoint at 0x%lx\n", addr);
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
/* XXX: called after the step */
int debug_bp_restore_before(int pos)
{

}

/* called before the step */
int debug_bp_restore(int pos)
{
	struct bp_t *bp;
	u64 addr = arch_pc()-1; // x86
#if 0
#if ARCH_I386
	u64 addr = arch_pc()-1; // x86
#else
	u64 addr = arch_pc()-4; // arm, mips, ppc, ...
#endif
#endif
	int ret = 0;

	if (pos==-1)
		bp = debug_bp_get(addr);
	else	bp = debug_bp_get_num(pos);

eprintf("bp-restore\n");

	if (bp == NULL) {
	//	eprintf("CaNnot restore no bp found here :/ %08llx\n", addr);
		return 0;
	}else
{
	printf("Breakpoint ata\n");
}
	//printf("go forward with bp found here !! %08llx and bp = %08x\n", addr, bp);
#if 0
	if (!bp->hw)
#if ARCH_I386
	  arch_jmp(arch_pc()-1);
#else
	  arch_jmp(arch_pc()-4);
#endif
#else
        if(WS(event) == BP_EVENT) {
		eprintf("restore: restoring bp at %llx\n", addr);
		arch_restore_bp(WS(bp));
		return 1;
        }
#endif

	return ret;
}

int debug_bp_rm(u64 addr, int type)
{
	struct bp_t *bp;
	int ret;

	bp = debug_bp_get(addr);	
	if (bp == NULL) {
		eprintf("No breakpoint found at this address\n");
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
	return debug_bp_rm(num, 1);
}

int debug_bp_rm_addr(u64 addr)
{
	return debug_bp_rm(addr, 0);
}

int debug_bp_set(struct bp_t *bp, u64 addr, int type)
{
	int i, ret, bp_free = -1;

	if (addr == 0)
		addr = arch_pc(); // WS_PC();

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

		if(ps.bps[i].addr == 0)
			bp_free = i;
	}

	ret = -1;
	if(type == BP_SOFT) {
		ret = arch_set_bp_soft(&ps.bps[bp_free], addr);
		
		ps.bps[bp_free].hw = 0;
	} else if(type == BP_HARD) {
		ret = arch_set_bp_hw(&ps.bps[bp_free], addr);
		ps.bps[bp_free].hw = 1;
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
}

void debug_reload_bps()
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