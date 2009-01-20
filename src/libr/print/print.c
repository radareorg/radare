/*
 * Copyright (C) 2007, 2008
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

#include "r_cons.h"
#include "r_print.h"

static int flags =
	R_PRINT_FLAGS_COLOR |
	R_PRINT_FLAGS_ADDRMOD;

void r_print_set_flags(int _flags)
{
	flags = _flags;
}

void r_print_addr(u64 addr)
{
	//config_get_i("cfg.addrmod");
	int mod = flags & R_PRINT_FLAGS_ADDRMOD;
	char ch = (0==(addr%(mod?mod:1)))?',':' ';

	if (flags & R_PRINT_FLAGS_COLOR) {
		r_cons_printf("%s0x%08llx"C_RESET"%c ",
			r_cons_palette[PAL_ADDRESS], addr, ch);
	} else r_cons_printf("0x%08llx%c ", addr, ch);
}

static const char hex[16] = "0123456789ABCDEF";
void r_print_hexdump(u64 addr, u8 *buf, int len, int step, int columns, int header)
{
	int i,j,k,inc;

	inc = 2+(int)((columns-14)/4);
	if (inc%2) inc++;
	inc = 16;

	if (header) {
		// only for color..too many options .. brbr
		r_cons_printf(r_cons_palette[PAL_HEADER]);
		r_cons_strcat("   offset   ");
		k = 0; // TODO: ??? SURE??? config.seek & 0xF;
		for (i=0; i<inc; i++) {
			r_cons_printf(" %c", hex[(i+k)%16]);
			if (i&1) r_cons_strcat(" ");
		}
		for (i=0; i<inc; i++)
			r_cons_printf("%c", hex[(i+k)%16]);
		r_cons_newline();
	}

	for(i=0; i<len; i+=inc) {
		r_print_addr(addr+(i*step));

		for(j=i;j<i+inc;j++) {
			if (j>=len) {
				r_cons_printf("  ");
				if (j%2) r_cons_printf(" ");
				continue;
			}
			r_cons_printf("%02x", (u8)buf[j]);
			//print_color_byte_i(j, "%02x", (unsigned char)buf[j]);
			if (j%2) r_cons_strcat(" ");
		}

		for(j=i; j<i+inc; j++) {
			if (j >= len)
				r_cons_strcat(" ");
			else r_cons_printf("%c",
				(IS_PRINTABLE(buf[j]))?
					buf[j] : '.');
		}
		r_cons_newline();
		addr+=inc;
	}
}

void r_print_bytes(const u8* buf, int len, const char *fmt)
{
	int i;
	for(i=0;i<len;i++)
		r_cons_printf(fmt, buf[i]);
	r_cons_newline();
}

void r_print_raw(const u8* buf, int len)
{
	r_cons_memcat(buf, len);
}

void r_print_format(const u8* buf, int len, const char *fmt)
{
	/* TODO: needs refactoring */
#if 0
	unsigned char buffer[256];
	int i,j,idx;
	int times, otimes;
	char tmp, last;
	char *args, *bracket;
	int nargs;
	const char *arg = fmt;
	i = j = 0;

	while(*arg && *arg==' ') arg = arg +1;
	/* get times */
	otimes = times = atoi(arg);
	if (times > 0)
		while((*arg>='0'&&*arg<='9')) arg = arg +1;
	bracket = strchr(arg,'{');
	if (bracket) {
		char *end = strchr(arg,'}');
		if (end == NULL) {
			eprintf("No end bracket. Try pm {ecx}b @ esi\n");
			return;
		}
		*end='\0';
		times = get_math(bracket+1);
		arg = end+1;
	}

	if (arg[0]=='\0') {
		print_mem_help();
		return;
	}
	/* get args */
	args = strchr(arg, ' ');
	if (args) {
		args = strdup(args+1);
		nargs = set0word(args);
		if (nargs == 0)
			efree((void **)&args);
	}

	/* go format */
	i = 0;
	if (times==0) otimes=times=1;
	for(;times;times--) {// repeat N times
		const char * orig = arg;
		if (otimes>1)
			cons_printf("0x%08llx [%d] {\n", config.seek+i, otimes-times);
		config.interrupted = 0;
		for(idx=0;!config.interrupted && idx<len;idx++, arg=arg+1) {
			addr = 0LL;
			if (endian)
				 addr = (*(buf+i))<<24   | (*(buf+i+1))<<16 | *(buf+i+2)<<8 | *(buf+i+3);
			else     addr = (*(buf+i+3))<<24 | (*(buf+i+2))<<16 | *(buf+i+1)<<8 | *(buf+i);

			tmp = *arg;
		feed_me_again:
			if (tmp == 0 && last != '*')
				break;
			/* skip chars */
			switch(tmp) {
			case ' ':
//config.interrupted =1;
				//i = len; // exit
				continue;
			case '*':
				if (i<=0) break;
				tmp = last;
				arg = arg - 1;
				idx--;
				goto feed_me_again;
			case 'e': // tmp swap endian
				idx--;
				endian ^=1;
				continue;
			case '.': // skip char
				i++;
				idx--;
				continue;
			case '?': // help
				print_mem_help();
				idx--;
				i=len; // exit
				continue;
			}
			if (idx<nargs)
				cons_printf("%10s : ", get0word(args, idx));
			/* cmt chars */
			switch(tmp) {
	#if 0
			case 'n': // enable newline
				j ^= 1;
				continue;
	#endif
			case 't':
				/* unix timestamp */
				D cons_printf("0x%08x = ", config.seek+i);
				{
				/* dirty hack */
				int oldfmt = last_print_format;
				u64 old = config.seek;
				radare_seek(config.seek+i, SEEK_SET);
				radare_read(0);
				print_data(config.seek+i, "8", buf+i, 4, FMT_TIME_UNIX);
				last_print_format=oldfmt;
				radare_seek(old, SEEK_SET);
				}
				break;
			case 'e': {
				double doub;
				memcpy(&doub, buf+i, sizeof(double));
				D cons_printf("%e = ", doub);
				cons_printf("(double)");
				i+=8;
				}
				break;
			case 'q':
				D cons_printf("0x%08x = ", config.seek+i);
				cons_printf("(qword)");
				i+=8;
				break;
			case 'b':
				D cons_printf("0x%08x = ", config.seek+i);
				cons_printf("%d ; 0x%02x ; '%c' ", 
					buf[i], buf[i], is_printable(buf[i])?buf[i]:0);
				i++;
				break;
			case 'B':
				memset(buffer, '\0', 255);
				radare_read_at((u64)addr, buffer, 248);
				D cons_printf("0x%08x = ", config.seek+i);
				for(j=0;j<10;j++) cons_printf("%02x ", buf[j]);
				cons_strcat(" ... (");
				for(j=0;j<10;j++)
					if (is_printable(buf[j]))
						cons_printf("%c", buf[j]);
				cons_strcat(")");
				i+=4;
				break;
			case 'd':
				D cons_printf("0x%08x = ", config.seek+i);
				cons_printf("%d", addr);
				i+=4;
				break;
			case 'x':
				D cons_printf("0x%08x = ", config.seek+i);
				cons_printf("0x%08x ", addr);
				i+=4;
				break;
			case 'X': {
				u32 addr32 = (u32)addr;
				char buf[128];
				D cons_printf("0x%08x = ", config.seek+i);
				cons_printf("0x%08llx ", addr32);
				if (string_flag_offset(buf, (u64)addr32, -1))
					cons_printf("; %s", buf);
				i+=4;
				} break;
			case 'w':
			case '1': // word (16 bits)
				D cons_printf("0x%08x = ", config.seek+i);
				if (endian)
					 addr = (*(buf+i))<<8  | (*(buf+i+1));
				else     addr = (*(buf+i+1))<<8 | (*(buf+i));
				cons_printf("0x%04x ", addr);
				break;
			case 'z': // zero terminated string
				D cons_printf("0x%08x = ", config.seek+i);
				for(;buf[i]&&i<len;i++) {
					if (is_printable(buf[i]))
						cons_printf("%c", buf[i]);
					else cons_strcat(".");
				}
				break;
			case 'Z': // zero terminated wide string
				D cons_printf("0x%08x = ", config.seek+i);
				for(;buf[i]&&i<len;i+=2) {
					if (is_printable(buf[i]))
						cons_printf("%c", buf[i]);
					else cons_strcat(".");
				}
				cons_strcat(" ");
				break;
			case 's':
				D cons_printf("0x%08x = ", config.seek+i);
				memset(buffer, '\0', 255);
				radare_read_at((u64)addr, buffer, 248);
				D cons_printf("0x%08x -> 0x%08x ", config.seek+i, addr);
				cons_printf("%s ", buffer);
				i+=4;
				break;
			default:
				/* ignore unknown chars */
				continue;
			}
		D cons_newline();
		last = tmp;
		}
		if (otimes>1)
			cons_printf("}\n");
		arg = orig;
		idx=0;
	}
	efree((void *)&args);
#endif
}
