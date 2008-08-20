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
#include "../mem.h"
#include "i386.h"
#include "../debug.h"

int arch_hack_help()
{
	printf("Usage: !hack <cmd>\n");
	printf(" 0 - nop one opcode (N bytes)\n");
	printf(" 1 - negate jump (jz->jnz , ja->jbe, ..)\n");
	printf(" 2 - force jmp (only for { 0f, 0x80-0x8f })\n");
	printf(" 3 - insert jmp (TODO)\n");
	printf(" 4 - insert call (TODO)\n");
	printf(" 5 - add ret (1 byte)\n");
	printf(" 6 - add ret with eax=0 (3 bytes)\n");
	printf(" 7 - negate zero flag (TODO)\n");
	printf(" 8 - jmp $$ (infinite loop) (2 bytes)\n");
	return 0;
}

int arch_hack(const char *cmd)
{
	int i, len;
	unsigned char buf[16];
	regs_t reg;

	// XXX: Make it work if config.seek instead of R_EIP(reg) !!
	if (!cmd)
		return arch_hack_help();
	if (cmd[0]==' ')
		cmd = cmd + 1;

	switch(cmd[0]) {
	case '0': // nop
		debug_getregs(ps.tid, &reg);
		debug_read_at(ps.tid, buf, 16, R_EIP(reg));
		len = dislen(buf, 16);
		for(i=0;i<len;i++)
			buf[i]=0x90;
		debug_write_at(ps.tid, buf, 16, R_EIP(reg));
		break;
	case '1':
		debug_getregs(ps.tid, &reg);
		debug_read_at(ps.tid, buf, 4, R_EIP(reg));
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
		debug_write_at(ps.tid, buf, 4, R_EIP(reg));
		break;
	case '2': // force jump
		debug_getregs(ps.tid, &reg);
		debug_read_at(ps.tid, buf, 5, R_EIP(reg));
		if (buf[0]==0x0f)
		if (buf[1]>=0x80 && buf[1]< 0x8f) {
			buf[0] = 0x90; // nop
			buf[1] = 0xe9; // jmp
		}
		debug_write_at(ps.tid, buf, 5, R_EIP(reg));
		break;
	case '5':
		debug_getregs(ps.tid, &reg);
		debug_read_at(ps.tid, buf, 4, R_EIP(reg));
		buf[0]=0xc3; // RET
		debug_write_at(ps.tid, buf, 4, R_EIP(reg));
		break;
	case '6': // ret 0
		debug_getregs(ps.tid, &reg);
		debug_read_at(ps.tid, buf, 4, R_EIP(reg));
		buf[0]=0x31; // XOR
		buf[1]=0xc0; // EAX, EAX
		buf[2]=0xc3; // RET
		debug_write_at(ps.tid, buf, 4, R_EIP(reg));
		break;
	case '7':
		debug_getregs(ps.tid, &reg);
		i = R_EFLAGS(reg) & (1 << 6);
		if (i) R_EFLAGS(reg) &= 0xFFFFFFbf;
		else R_EFLAGS(reg) |= (1 << 6);
		debug_setregs(ps.tid, &reg);
		break;
	case '8':
		debug_getregs(ps.tid, &reg);
		debug_write_at(ps.tid,(unsigned char *)"\xeb\xfe", 2, R_EIP(reg));
		break;
	case '?':
		arch_hack_help();
		break;
	default:
		break;
	}
	return 0;
}
