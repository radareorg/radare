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

#include "rasm.h"

int rasm_x86(u64 offset, const char *str, unsigned char *data)
{
	char op[128];
	char *arg;

	strncpy(op, str, 120);
 	arg = strchr(op, ' ');
	if (arg) {
		arg[0] = '\0';
		arg = arg + 1;
	}
	if (!strcmp(op, "ret")) {
		data[0]='\xc3';
		return 1;
	} else
	if (!strcmp(op, "ret0")) {
		memcpy(data, "\x31\xc0\xc3", 3);
		return 3;
	} else
	if (!strcmp(op, "trap")) {
		data[0]='\xcc';
		return 1;
	} else
	if (arg && !strcmp(op, "int")) {
		int a = (int)get_offset(arg);
		data[0]='\xcd';
		data[1]=(unsigned char)a;
		return 2;
	} else
	if (!strcmp(op, "hang")) {
		data[0]='\xeb';
		data[1]='\xfe';
		return 2;
	} else
	if (arg && !strcmp(op, "call")) {
		u64 dst = get_math(arg);
		unsigned long addr = dst;
		unsigned char *ptr = (uchar *)&addr;

		if (dst == 0) {
			data[0] = '\xff';
			if (!strcmp(arg, "eax")) data[1]='\xd0'; else
			if (!strcmp(arg, "ebx")) data[1]='\xd3'; else
			if (!strcmp(arg, "ecx")) data[1]='\xd1'; else
			if (!strcmp(arg, "edx")) data[1]='\xd2'; else
			if (!strcmp(arg, "esi")) data[1]='\xd6'; else
			if (!strcmp(arg, "edi")) data[1]='\xd7'; else
			if (!strcmp(arg, "ebp")) data[1]='\xd5'; else
			if (!strcmp(arg, "esp")) data[1]='\xd4';
			else {
				printf("Invalid argument for 'call'\n");
				return 0;
			}
			return 2;
		}
		addr = addr - offset - 5;

		data[0] = '\xe8';
		data[1] = ptr[0];
		data[2] = ptr[1];
		data[3] = ptr[2];
		data[4] = ptr[3];
		return 5;
	} else
	if (arg && !strcmp(op, "push")) {
		off_t dst = get_math(arg);
		unsigned long addr = dst;
		unsigned char *ptr = (uchar *)&addr;

		if (dst == 0) {
			if (!strcmp(arg, "eax")) data[0]='\x50'; else
			if (!strcmp(arg, "ebx")) data[0]='\x53'; else
			if (!strcmp(arg, "ecx")) data[0]='\x51'; else
			if (!strcmp(arg, "edx")) data[0]='\x52'; else
			if (!strcmp(arg, "esi")) data[0]='\x56'; else
			if (!strcmp(arg, "edi")) data[0]='\x57'; else
			if (!strcmp(arg, "ebp")) data[0]='\x55'; else
			if (!strcmp(arg, "esp")) data[0]='\x54';
			else
				return 0; // invalid register name to push
			
			return 1;
		}

		data[0] = '\x68';
		data[1] = ptr[0];
		data[2] = ptr[1];
		data[3] = ptr[2];
		data[4] = ptr[3];
		return 5;
	} else
	if (arg && !strcmp(op, "pop")) {
		off_t dst = get_math(arg);

		if (dst == 0) {
			if (!strcmp(arg, "eax")) data[0]='\x58'; else
			if (!strcmp(arg, "ebx")) data[0]='\x5b'; else
			if (!strcmp(arg, "ecx")) data[0]='\x59'; else
			if (!strcmp(arg, "edx")) data[0]='\x5a'; else
			if (!strcmp(arg, "esi")) data[0]='\x5e'; else
			if (!strcmp(arg, "edi")) data[0]='\x5f'; else
			if (!strcmp(arg, "ebp")) data[0]='\x5d'; else
			if (!strcmp(arg, "esp")) data[0]='\x5c'; else
				return 0; // invalid register name to push
			
			return 1;
		}
		eprintf("Invalid pop syntax\n");
		return 0;
	} else
	if (arg && !strcmp(op, "mov")) {
		off_t dst;
		unsigned long addr;
		unsigned char *ptr = (uchar *)&addr;
		char *arg2 = strchr(arg, ',');
		if (arg2 == NULL) {
			eprintf("Invalid syntax\n");
			return 0;
		}
		arg2[0]='\0';
		dst = get_math(arg2+1);
		addr = dst;

		data[0]='\xb8';
		if (!strcmp(arg, "eax")) data[0] = 0xb8; else
		if (!strcmp(arg, "ebx")) data[0] = 0xbb; else
		if (!strcmp(arg, "ecx")) data[0] = 0xb9; else
		if (!strcmp(arg, "edx")) data[0] = 0xba; else
		if (!strcmp(arg, "esp")) data[0] = 0xbc; else
		if (!strcmp(arg, "ebp")) data[0] = 0xbd; else
		if (!strcmp(arg, "esi")) data[0] = 0xbe; else
		if (!strcmp(arg, "edi")) data[0] = 0xbf;

		if (dst==0 && arg2[1]!='0') {
			int src = data[0];
			data[0]=0x89;
			if (strstr(arg2+1, "eax")) {
				switch(src) {
				case 0xb8: data[1]=0xc0; break;
				case 0xbb: data[1]=0xd8; break;
				case 0xb9: data[1]=0xc8; break;
				case 0xba: data[1]=0xd0; break;
				case 0xbc: data[1]=0xe0; break;
				case 0xbd: data[1]=0xe8; break;
				case 0xbe: data[1]=0xf0; break;
				case 0xbf: data[1]=0xf8; break;
				default:
					fprintf(stderr, "OOPS: unknown register\n");
					return 0;
				}
				return 2;
			} else
			if (!strcmp(arg2, "ebx")) data[0]='\x5b'; else
			if (!strcmp(arg2, "ecx")) data[0]='\x59'; else
			if (!strcmp(arg2, "edx")) data[0]='\x5a'; else
			if (!strcmp(arg2, "esi")) data[0]='\x5e'; else
			if (!strcmp(arg2, "edi")) data[0]='\x5f'; else
			if (!strcmp(arg2, "ebp")) data[0]='\x5d'; else
			if (!strcmp(arg2, "esp")) data[0]='\x5c'; else
				return 0; // invalid register name to push
		} else {
			data[1] =ptr[0];
			data[2] =ptr[1];
			data[3] =ptr[2];
			data[4] =ptr[3];
		}
		return 5;
	} else
	if (arg && !strcmp(op, "jmp")) {
		//off_t dst = get_math(arg); // XXX: get_math breaks ebp+33 to be 33 instead of 0
		off_t dst = get_offset(arg) - offset;
		unsigned long addr = dst;
		unsigned char *ptr = (uchar *)&addr;

		if (dst+offset == 0) {
			data[0] = '\xff';
			if (!strcmp(arg, "eax")) data[1]='\xe0'; else
			if (!strcmp(arg, "ebx")) data[1]='\xe3'; else
			if (!strcmp(arg, "ecx")) data[1]='\xe1'; else
			if (!strcmp(arg, "edx")) data[1]='\xe2'; else
			if (!strcmp(arg, "esi")) data[1]='\xe6'; else
			if (!strcmp(arg, "edi")) data[1]='\xe7'; else
			if (!strcmp(arg, "ebp")) data[1]='\xe5'; else
			if (!strcmp(arg, "esp")) data[1]='\xe4'; 
			else {
				if (!strcmp(arg, "[eax]")) data[1]='\x20'; else
				if (!strcmp(arg, "[ebx]")) data[1]='\x23'; else
				if (!strcmp(arg, "[ecx]")) data[1]='\x21'; else
				if (!strcmp(arg, "[edx]")) data[1]='\x22'; else
				if (!strcmp(arg, "[esi]")) data[1]='\x26'; else
				if (!strcmp(arg, "[edi]")) data[1]='\x27'; 
				else {
					if (!strcmp(arg, "esp")) { data[1]='\x24'; data[2]='\x24'; } else
					if (!strcmp(arg, "ebp")) { data[1]='\x24'; data[2]='\x24'; } else
					if (strstr(arg, "[eax")) { data[1]='\x60'; data[2]=(char)get_math(arg+4); } else
					if (strstr(arg, "[ebx")) { data[1]='\x63'; data[2]=(char)get_math(arg+4); } else
					if (strstr(arg, "[ecx")) { data[1]='\x61'; data[2]=(char)get_math(arg+4); } else
					if (strstr(arg, "[edx")) { data[1]='\x62'; data[2]=(char)get_math(arg+4); } else
					if (strstr(arg, "[esi")) { data[1]='\x66'; data[2]=(char)get_math(arg+4); } else
					if (strstr(arg, "[edi")) { data[1]='\x67'; data[2]=(char)get_math(arg+4); } else
					if (strstr(arg, "[esi")) { data[1]='\x67'; data[2]=(char)get_math(arg+4); } else
					if (strstr(arg, "[ebp")) { data[1]='\x65'; data[2]=(char)get_math(arg+4); } 
					else {
						if (!strcmp(arg, "[esp")) { data[1]='\x64'; data[2]='\x24'; data[3]=(char)get_math(arg+4); }
							else return 0;
						return 4;
					}
					return 3;
				}
				return 2; // invalid register name to push
			}
			return 2;
		}

		dst-=offset;

// 7C90EAF5   .- E9 42158783   JMP     0018003C
// RELATIVE LONG JUMP (nice coz is 4 bytes, not 5) 

		if (dst>-0x80 && dst<0x7f) {
			/* relative address */
			addr-=2;
			addr-=offset;
			data[0]='\xeb';
			data[1]=(char)dst;
			return 2;
		} else {
			/* absolute address */
			addr-=5;
			data[0]='\xe9';
			data[1] =ptr[0];
			data[2] =ptr[1];
			data[3] =ptr[2];
			data[4] =ptr[3];
			return 5;
		}
	} else
	if (!strcmp(op, "jnz")) {
		off_t dst = get_math(arg) - offset;

		if (dst>-0x80 && dst<0x7f) {
			dst-=2;
			data[0]='\x75';
			data[1]=(char)dst;
			return 2;
		} else {
			data[0]=0x0f;
			data[1]=0x85;
			dst-=6;
			memcpy(data+2,&dst,4);
			return 6;
		}
	} else
	if (!strcmp(op, "jz")) {
		off_t dst = get_math(arg) - offset;

		if (dst>-0x80 && dst<0x7f) {
			dst-=2;
			data[0]='\x74';
			data[1]=(char)dst;
			return 2;
		} else {
			data[0]=0x0f;
			data[1]=0x84;
			dst-=6;
			memcpy(data+2,&dst,4);
			return 6;
		}
	} else
	if (!strcmp(op, "pusha")) {
		data[0]='\x60';
		return 1;
	} else
	if (!strcmp(op, "popa")) {
		data[0]='\x61';
		return 1;
	} else
	if (!strcmp(op, "nop")) {
		data[0]='\x90';
		return 1;
	} else {
		if (op[0]&& op[0]==';')
			return 0;
		fprintf(stderr, "unknown opcode (%s)\n",op);
	}

	return 0;
}
