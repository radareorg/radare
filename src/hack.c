/*
 * Copyright (C) 2008
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

#include "main.h"
#include "code.h"
#include "utils.h"
#include "print.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if 0
TODO:
	printf(" 7 - negate zero flag (TODO)\n");
#endif

struct hack_t {
	const char *name;
	const char *desc;
	int (*callback)();
	struct list_head list;
};

struct list_head hacks;

int radare_hack_help()
{
	int i=1;
	struct list_head *pos;

	list_for_each(pos, &hacks) {
		struct hack_t *h= list_entry(pos, struct comment_t, list);
		cons_printf("%02d %s\t%s\n", i++, h->name, h->desc);
	}
	return 0;
}

static int hack_nop()
{
	struct aop_t aop;
	char buf[1024];
	int i, len;
	int delta = (config.cursor_mode)?config.cursor:0;
	char *arch = config_get("asm.arch");

	if (!config_get("cfg.write"))
		return 0;
	//debug_getregs(ps.tid, &reg);
	//debug_read_at(ps.tid, buf, 16, R_EIP(reg));
	radare_read_at(config.seek+delta, buf, 16);
	len = arch_aop(config.seek+delta, buf, &aop);
	if (!strcmp(arch, "x86")) {
		for(i=0;i<len;i++)
			buf[i]=0x90;
	} else {
		// TODO real multiarch
		for(i=0;i<len;i++)
			buf[i]=0x00;
	}
	//debug_write_at(ps.tid, buf, 16, R_EIP(reg));
	radare_write_at( config.seek+delta, buf, 4);
	return 0;
}

static int hack_negjmp()
{
	int delta = (config.cursor_mode)?config.cursor:0;
	char *buf = config.block+delta;
	if (!config_get("cfg.write"))
		return 0;

// TODO: multiarch!
	eprintf("warning: x86 only atm.\n");
	switch(buf[0]) {
	case 0x0f:
		switch(buf[1]) {
		case 0x84: // jz
			buf[1] = 0x85;
			break;
		case 0x85: // jnz
			buf[1] = 0x84;
			break;
		case 0x8e: // jle
		case 0x86: // jbe
			buf[1] = 0x8f; // jg
			break;
		case 0x87: // ja
		case 0x8f: // jg
			buf[1] = 0x86; // jbe
			break;
		case 0x88: // js
			buf[1] = 0x89; // jns
			break;
		case 0x89: // jns
			buf[1] = 0x88; // js
			break;
		case 0x8a: // jp
			buf[1] = 0x8b; // jnp
			break;
		case 0x8b: // jnp
			buf[1] = 0x8a; // jp
			break;
		case 0x8c: // jl
			buf[1] = 0x8d;
			break;
		case 0x8d: // jge
			buf[1] = 0x8c;
			break;
		}
		break;
	case 0x70: buf[0] = 0x71; break; // jo->jno
	case 0x71: buf[0] = 0x70; break; // jno->jo
	case 0x72: buf[0] = 0x73; break; // jb->jae
	case 0x73: buf[0] = 0x72; break; // jae->jb
	case 0x75: buf[0] = 0x74; break; // jne->je
	case 0x74: buf[0] = 0x75; break; // je->jne
	case 0x76: buf[0] = 0x77; break; // jbe->ja
	case 0x77: buf[0] = 0x76; break; // jbe->ja
	case 0x78: buf[0] = 0x79; break; // js->jns
	case 0x79: buf[0] = 0x78; break; // jns->js
	case 0x7a: buf[0] = 0x7b; break; // jp->jnp
	case 0x7b: buf[0] = 0x7a; break; // jnp->jp
	case 0x7c: buf[0] = 0x7d; break; // jl->jge
	case 0x7d: buf[0] = 0x7c; break; // jge->jl
	case 0x7e: buf[0] = 0x7f; break; // jg->jle
	case 0x7f: buf[0] = 0x7e; break; // jle->jg
		break;
	}
	
	radare_write_at( config.seek+delta, buf, 4);
	//debug_write_at(ps.tid, buf, 4, R_EIP(reg));
	return 0;
}

static int hack_forcejmp()
{
//	debug_getregs(ps.tid, &reg);
//	debug_read_at(ps.tid, buf, 5, R_EIP(reg));
	char buf[128];
	int delta = (config.cursor_mode)?config.cursor:0;
	if (!config_get("cfg.write"))
		return 0;

	eprintf("warning: x86 only atm.\n");

	radare_read_at(config.seek+delta, buf, 5);
	if (buf[0]==0x0f)
	if (buf[1]>=0x80 && buf[1]< 0x8f) {
		buf[0] = 0x90; // nop
		buf[1] = 0xe9; // jmp
	}
	radare_write_at(config.seek+delta, buf, 2);
//	debug_write_at(ps.tid, buf, 5, R_EIP(reg));
	return 0;
}

/* logic */

struct hack_t *radare_hack_new(char *name, char *desc, int (*callback)())
{
	struct hack_t *hack;
	hack = (struct hack_t *)malloc(sizeof(struct hack_t));
	hack->name = strdup(name);
	hack->desc = strdup(desc);
	hack->callback = callback;
	return hack;
}

int radare_hack_init()
{
	struct hack_t *hack;
	INIT_LIST_HEAD(&hacks);
	hack = radare_hack_new("no", "nop one opcode", &hack_nop);
	list_add_tail(&(hack->list), &(hacks));
	hack = radare_hack_new("nj", "negate jump", &hack_negjmp);
	list_add_tail(&(hack->list), &(hacks));
	hack = radare_hack_new("fj", "force jump", &hack_forcejmp);
	list_add_tail(&(hack->list), &(hacks));
}

int radare_hack(const char *cmd)
{
	int i=1;
	struct list_head *pos;
	int num = 0;
	char *ptr = strchr(cmd, ' ');

	if (ptr) {
		ptr = ptr + 1;
		num = atoi(ptr);
	}

	if ((!num && !ptr) || (strnull(cmd)))
		return radare_hack_help();

	eprintf("warning: does not yet works inside debugger\n");
	list_for_each(pos, &hacks) {
		struct hack_t *h= list_entry(pos, struct comment_t, list);
		if (num) {
			 if  (i==num) {
				h->callback();
				return 0;
			}
		} else {
			if (!strcmp(ptr, h->name)) {
				h->callback();
				return 0;
			}
		}
	}
	eprintf("Unknown hack\n");
	return 1;
}
