/*
 * Copyright (C) 2007
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

#include "main.h"
#include "utils.h"
#include "plugin.h"
#include "rdb/rdb.h"
#include "list.h"
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

float hash_entropy(char *data, size_t size);

int inc = 16;
int dec = 16;
format_info_t formats[] = {
	{ 'b', FMT_BIN,        MD_BLOCK,  "binary",                 "N bytes",    "entire block" },
	{ 'o', FMT_OCT,        MD_BLOCK,  "octal",                  "N bytes",    "entire block" },
	{ 'O', FMT_ZOOM,       MD_BLOCK,  "Whole file overview",    "entire file", "entire block" },
	{ 'c', FMT_CSTR,       MD_BLOCK,  "C format",               "N bytes",    "entire block" },
	{ 'C', FMT_CODE,       MD_BLOCK,  "Code Analysis",          "N bytes",    "entire block" },
	{ 'G', FMT_CODEGRAPH,  MD_BLOCK,  "Code Analysis Graph",    "N bytes",    "entire block" },
	{ 'a', FMT_ASHC,       MD_BLOCK,  "asm shellcode",          NULL,         "entire block" },
	{ 'A', FMT_ANAL,       MD_BLOCK,  "analyze data",           NULL,         "entire block" },
	{ 'r', FMT_RAW,        MD_BLOCK,  "raw ascii",              NULL,         "entire block" },
	{ 'u', FMT_URLE,       MD_BLOCK,  "URL encoding",           NULL,         "entire block" },
	{ 'p', FMT_PRINT,      MD_BLOCK,  ".%PRINTCMD",             NULL,         "entire block" },
	{ 'v', FMT_VISUAL,     MD_BLOCK,  ".%VISUALCMD",            NULL,         "entire block" },
	{ 's', FMT_ASC,        MD_BLOCK,  "ascii",                  NULL,         "entire block" },
	{ 'S', FMT_ASCP,       MD_BLOCK,  "ascii printable",        NULL,         "entire block" },
	{ 'z', FMT_ASC0,       MD_BLOCK,  "ascii null terminated",  NULL,         "until \\0" },
	{ 'Z', FMT_WASC0,      MD_BLOCK,  "wide ascii null end",    NULL,         "until \\0" },
	{ 'd', FMT_DISAS,      MD_BLOCK,  "(asm.objdump) disassembly",  "bsize bytes", "entire block" },
	{ 'D', FMT_UDIS,       MD_BLOCK,  "(asm.arch) disassembler",   "bsize bytes", "entire block" },
	{ 'X', FMT_HEXBS,      MD_BLOCK,  "hexadecimal string",	   "N byte", 	  "entire block" },
	{ 'x', FMT_HEXB,       MD_BLOCK,  "hexadecimal byte pairs","N byte", 	  "entire block" },
	{ 't', FMT_TIME_UNIX,  MD_BLOCK,  "unix timestamp",        "4 bytes",	  "(endian)"},
	{ 'T', FMT_TIME_DOS,   MD_BLOCK,  "dos timestamp",         "4 bytes",	  "(endian)"},
	{ 'f', FMT_FLOAT,      MD_BLOCK,  "float",                 "4 bytes",     "(endian)"},
	{ 'F', FMT_TIME_FTIME, MD_BLOCK,  "windows filetime",      "8 bytes",     "(endian)"},
	{ 'h', FMT_SHORT,      MD_BLOCK,  "short",                 "2 bytes",     "(endian)"},
	{ 'i', FMT_INT,        MD_BLOCK,  "integer",               "4 bytes",     "(endian)"},
	{ 'l', FMT_LONG,       MD_BLOCK,  "long",                  "4 bytes",     "(endian)"},
	{ 'L', FMT_LLONG,      MD_BLOCK,  "long long",             "8 bytes",     "(endian)"},
	{ 'q', FMT_HEXQ,       MD_BLOCK,  "hexadecimal quad-word", "8 bytes",	  "(endian)"},
	{ 'W', FMT_HEXD,       MD_BLOCK,  "hexadecimal dword",     "4 bytes",     "(endian)"},
	{ 'w', FMT_HEXW,       MD_BLOCK,  "hexadecimal word",      "2 bytes",	  "(endian)"},
	{ 0, 0, 0, NULL }
};

/** Get the specified format
 *
 * In case of an invalid format, returns FMT_ERR.
 * 
 * /fmt: the format specifier
 * /mode: the supported format modes
 *
 * /ret: the specified format
 */
print_fmt_t format_get (char fmt, print_mode_t mode)
{
	format_info_t *fi;

	for (fi = formats; fi->id != 0; fi++)
		if ((fi->mode & mode) && fi->id == fmt)
			return fi->print_fmt;

	return FMT_ERR;
}

// TODO: implement getenv("RCOLORS");
void print_addr(off_t off)
{
	C	pprintf(COLOR_AD""OFF_FMT""C_RESET" ", off);
	else	pprintf(OFF_FMT" ", off);
}

char *get_color_for(int c)
{
	if (c==0)    return COLOR_00; else
	if (c==0xff) return COLOR_FF; else
	if (c==0x7f) return COLOR_7F; else
	if (is_printable(c)) return COLOR_PR;
	return "";
}

void print_color_byte(char *str, int c)
{
	char cash[128];

	C {	strcpy(cash, get_color_for(c));
		strcat(cash, str);
		strcat(cash, C_RESET);
		pprintf(cash, (unsigned char)c);
	} else
		pprintf(str, c);
}

void cursor_precolor(int i)
{
	if (config.cursor_mode) {
		if (config.ocursor != -1) {
			if ((i >= config.ocursor && i <= config.cursor)
			||  (i <= config.ocursor && i >= config.cursor)) {
				pprintf("\e[7m");
				return;
			}
		} else {
			if (i == config.cursor) {
				pprintf("\e[7m");
				return;
			}
		}
	}
	return;
}

void print_color_byte_i(int i, char *str, int c)
{
	if (config.cursor_mode) {
		if (config.ocursor != -1) {
			if ((i >= config.ocursor && i <= config.cursor)
			||  (i <= config.ocursor && i >= config.cursor)) {
				pprintf("\e[7m");
				print_color_byte(str, c);
				pprintf("\e[0m");
				return;
			}
		} else {
			if (i == config.cursor) {
				pprintf("\e[7m");
				print_color_byte(str, c);
				pprintf("\e[0m");
				return;
			}
		}
	}
	print_color_byte(str, c);
	return;
}

void print_color_byte_is(int i, char *str, char *c)
{
	char cash[128];

	if (config.cursor_mode) {
		if (i == config.cursor) {
			strcpy(cash, "\e[7m");
			strcat(cash, str);
			strcat(cash, C_RESET);
			pprintf(cash, c);
		} else
			print_color_byte_i(i, str, 0);
	} else
		print_color_byte_i(i, str, 0);
}

void format_show_help (print_mode_t mode)
{
	format_info_t *fi;
	
	pprintf("Available formats:\n");
	for (fi = formats; fi->id != 0; fi++)
	if (fi->mode & mode) {
		if (mode == MD_ONCE)
			pprintf(" %c : %-23s %s\n", fi->id, fi->name, fi->sizeo);
		else	pprintf(" %c : %-23s %s\n", fi->id, fi->name, fi->sizeb);
	}
}

void radare_dump_and_process(int type, int size)
{
	char cmd[1024];
	char file[TMPFILE_MAX];
	const char *objdump = config_get("asm.objdump");
	const char *syntax  = config_get("asm.syntax");
	int ret;
	int x,y;

	if (objdump == NULL) {
		eprintf("OBJDUMP not defined. Use 'objdump -m i386 --target=binary -D f.ex.'.\n");
		return;
	}

	make_tmp_file (file);
	CHECK_TMP_FILE(file)
	radare_dump   (file, size);

	switch(type) {
	case DUMP_MAGIC:
		sprintf(cmd, "rfile '%s'", file);
		break;
	case DUMP_DISASM:
		D radare_command("fd", 0);
		x = config_get_i("scr.x");
		y = config_get_i("scr.y");
		if (x&&y)
		sprintf(cmd,"%s %s %s | head -n %d | Y=%d W=%d awk 'BEGIN{W=ENVIRON[\"W\"];"
			"Y=ENVIRON[\"Y\"];E=\"\x1B\";ORS=\"\";print \"\" E \"[\" Y \";"
			"%dH\";Y++}{ORS=E \"[\" Y \";%dH\";"
			"if(/.:\\t/){print substr($_,0,W);Y++};}'",
			objdump, (syntax&&syntax[0]=='i')?"-M intel":"",file,
			config.visual?config.height:200, y, config.width, x,x);
		else
		sprintf(cmd, "%s %s %s | head -n %d | awk '{if (/.:\\t/)print;}'", 
			objdump, (syntax&&syntax[0]=='i')?"-M intel":"",file, 
			config.visual?config.height:200);
		break;
	}
	ret = io_system(cmd);
	if (ret != 0)
		eprintf("Error executing command ('%s')\n", cmd);

	unlink(file);
}

/** Print some data.
 *
 * /seek: position of the stream
 * /buf: buffer to print
 * /len: buffer length (only used in block mode)
 * /print_fmt: print format
 * /mode: print mode
 *
 * /TODO: don't use config.<...> (this should be completely parametrized)
 * /TODO: normalize output command
 */
void data_print(off_t seek, unsigned char *buf, int len, print_fmt_t print_fmt, print_mode_t mode)
{
	int tmp, i, j;
	unsigned char buffer[4];
	unsigned char *bufi; // inverted buffer
	int lines = 0;
	int x,y;
	// code anal
	struct program_t *prg;
	struct block_t *b0;
	struct list_head *head;

	last_print_format = print_fmt;
	if (buf == NULL)
		return;

	if (mode & FMT_INV) {
		bufi = (unsigned char *)malloc(len);
		for(i=0;i<len;i++)
			bufi[i] = buf[len-i-1];
		buf = bufi;
	}

	if (len <= 0) len = config.block_size;

	switch(print_fmt) {
	case FMT_ANAL:
		radare_analyze(seek, len);
		break;
	case FMT_PRINT:
		INILINE;
		i = last_print_format;
		radare_command( config_get("cmd.print"),0);
		last_print_format = i;
		break;
	case FMT_VISUAL:
		i = last_print_format;
		radare_command( config_get("cmd.visual"),0);
		last_print_format = i;
		break;
	case FMT_DISAS:
		radare_dump_and_process( DUMP_DISASM, len);
		break;
	case FMT_CODE: {
		char cmd[1024];
		prg = code_analyze(config.seek+config.baddr, config_get_i("graph.depth"));
		list_add_tail(&prg->list, &config.rdbs);
		list_for_each(head, &(prg->blocks)) {
			b0 = list_entry(head, struct block_t, list);
			D {
				pprintf("0x%08x (%d) -> ", b0->addr, b0->n_bytes);
				if (b0->tnext)
					pprintf("0x%08x", b0->tnext);
				if (b0->fnext)
					pprintf(", 0x%08x", b0->fnext);
				pprintf("\n");
				sprintf(cmd, "pD %d @ 0x%08x", b0->n_bytes+1, (unsigned int)b0->addr);
				radare_command(cmd, 0);
				pprintf("\n\n");
			} else {
				pprintf("b %d\n", b0->n_bytes);
				pprintf("f blk_%08X @ 0x%08x\n", b0->addr, b0->addr);
			}
			i++;
		}
		} break;
	case FMT_CODEGRAPH:
#if VALA
		prg = code_analyze(config.baddr + config.seek, config_get_i("graph.depth"));
		list_add_tail(&prg->list, &config.rdbs);
		grava_program_graph(prg);
#else
		eprintf("Compiled without valac/gtk/cairo\n");
#endif
		break;
	case FMT_UDIS:
		disassemble(len, 0);
		break;
	case FMT_SHORT: {
		short *s;
		for(i=0;i<len;i+=sizeof(short)) {
			endian_memcpy(buffer, buf+i, sizeof(short));
			s = (short *)buffer;
			print_color_byte("%hd", s[0]);
			NEWLINE;
		} } break;
	case FMT_FLOAT: {
		float *f;
		for(i=0;i<len;i+=sizeof(float)) {
			endian_memcpy(buffer, buf+i, sizeof(float));
			f = (float *)buffer;
			print_color_byte("%f", f[0]);
			NEWLINE;
		} } break;
	case FMT_INT: {
		int *iv;
		for(i=0;i<len;i+=sizeof(int)) {
			endian_memcpy(buffer, buf+i, sizeof(int));
			iv = (int *)buffer;
			print_color_byte("%d", iv[0]);
			NEWLINE;
		} } break;
	case FMT_LONG: {
		int i;
		long *l;
		INILINE;
		for(i=0;i<len;i+=sizeof(long)) {
			endian_memcpy(buffer, config.block+i, sizeof(long));
			l = (long *)buffer;
			print_color_byte("%ld", *l);
			//printf("%ld", *l);
			D { NEWLINE; } else pprintf(" ");
		} }
		break;
	case FMT_LLONG: {
		long long *ll;
		INILINE;
		for(i=0;i<len;i+=sizeof(long long)) {
			endian_memcpy(buffer, config.block+i, sizeof(long long));
 			ll = (long long *)buffer;
			pprintf("%lld", *ll);
			D { NEWLINE; } else pprintf(" ");
		} } break;
	/*   DATES   */
	case FMT_TIME_DOS: {
		unsigned char _time[2];
		unsigned char _date[2];
		for(i=0;i<len;i+=4) {
			endian_memcpy(_time, config.block+i, 2);
			endian_memcpy(_date, config.block+i+2, 2);
			print_msdos_date(_time, _date);
			NEWLINE;
		} } break;
	case FMT_TIME_UNIX: {
		time_t t;
		char datestr[256];
		char *datefmt;
		INILINE;
		for(i=0;i<len;i+=4) {
			endian_memcpy((unsigned char*)&t, config.block+i, sizeof(time_t));
			//printf("%s", (char *)ctime((const time_t*)&t));
			datefmt = config_get("cfg.datefmt");
			
			if (datefmt&&datefmt[0])
				tmp = strftime(datestr,256,datefmt,
					(const struct tm*)gmtime((const time_t*)&t));
			else 	tmp = strftime(datestr,256,"%d:%m:%Y %H:%M:%S %z",
					(const struct tm*)gmtime((const time_t*)&t));
			// TODO colorize depending on the distance between dates
			if (tmp) pprintf("%s",datestr); else printf("*failed*");
			NEWLINE;
		} } break;
	case FMT_TIME_FTIME: {
		unsigned long long l, L = 0x2b6109100LL;
		time_t t;
		char datestr[256];
		const char *datefmt;
		INILINE;
		for(i=0;i<len;i+=8) {
			endian_memcpy((unsigned char*)&l, config.block+i, sizeof(unsigned long long));
			l /= 10000000; // 100ns to s
			l = (l > L ? l-L : 0); // isValidUnixTime?
			t = (time_t) l; // TODO limit above!
			datefmt = config_get("cfg.datefmt");
			if (datefmt&&datefmt[0])
				tmp = strftime(datestr, 256, datefmt,
					(const struct tm*)gmtime((const time_t*)&t));
			else 	tmp = strftime(datestr, 256, "%d:%m:%Y %H:%M:%S %z",
					(const struct tm*)gmtime((const time_t*)&t));
			if (tmp) pprintf("%s", datestr); else pprintf("*failed*");
			NEWLINE;
		} } break;
	case FMT_RAW:
		// XXX TODO: measure the string length and make it fit properly
		V i = config.width*config.height; else i=len;
		write(1, buf, (len>i)?i:len);
		break;
	case FMT_URLE:
		INILINE;
		for(i = 0; i < len; i++) {
			if (config.verbose&&is_printable(config.block[i]))
				print_color_byte_i(i, "%c", config.block[i]);
			else 	print_color_byte_i(i, "%%%02x", config.block[i]);
		}
		NEWLINE;
		break;
	case FMT_ASHC:
		INILINE;
                pprintf("shellcode:");
		inc = config.width/7;
		lines = 0;
                for(i = 0; i < len; i++) {
			V if (lines>config.height-4)
					break;
                        if (!(i%inc)) {
				if (++lines>config.height-4) 
					break;
				NEWLINE; pprintf(".byte ");
			}
			print_color_byte_i(i, "0x%02x", config.block[i]);
                        if (((i+1)%inc) && i+1<len) pprintf(", ");
                }
		NEWLINE;
                pprintf("shellcode_len = . - shellcode"); NEWLINE;
                break;
	case FMT_CSTR:
		D { INILINE; }
		inc = config.width/6;
		pprintf("#define _BUFFER_SIZE %d", len); NEWLINE;
		pprintf("unsigned char buffer[_BUFFER_SIZE] = {"); NEWLINE;
		for(j = i = 0; i < len;) {
			print_color_byte_i(i, "0x%02x", config.block[i]);
			if (++i<len)  pprintf(", ");
			if (!(i%inc)) {
				NEWLINE; 
				V if (++j+5>config.height)
					D if ((i/inc)+5 > config.height )
						break;
			}
		}
		pprintf(" };\n");
		break;
	case FMT_BIN:
		if (config.width<30)
			break;
		inc = (int)((config.width-17)/11);
		D {
			INILINE;
			C pprintf(COLOR_HD);
			pprintf("   offset");
			for(i=0;i<inc;i++)
				pprintf("       +0x%x",i);
			NEWLINE;
			pprintf("----------+");
			for(i=0;i<inc;i++)
				pprintf("-----------");
			NEWLINE;
			C pprintf(C_RESET);
		}
		for(i=0; i<len; i++) {
			V if ((i/inc)+5>config.height) break;
			D print_addr(seek+i+config.baddr);
			for(j = i+inc; i<j && i<len; i++) {
				C pprintf(get_color_for(buf[i]));
				cursor_precolor(i);
				PRINT_BIN(buf[i]);
				C pprintf(C_RESET);
			}
			i--;
			D { NEWLINE; }
		}
		break;
	case FMT_OCT:
		inc = (int)((config.width)/6);
		D {
			C pprintf(COLOR_HD);
			pprintf("  offset   ");
			for(i=0;i<inc;i++)
				pprintf("+%02x ",i);
			for(i=0;i<inc;i++)
				pprintf("%c",hex[i%16]);
			NEWLINE; pprintf("----------+");
			for(i=0;i<inc;i++)
				pprintf("----");
			for(i=0;i<inc;i++)
				pprintf("-");
			C pprintf(C_RESET);
			NEWLINE;
		}
		for(i=0;i<len;i++) {
			V if ((i/inc)+6>config.height) break;
			D print_addr(seek+i+config.baddr);
			tmp = i;
			for(j=i+inc;i<j && i<len;i++) {
				print_color_byte_i(i, "%03o", (int)buf[i]);
				pprintf(" ");
			}
			i = tmp;
			for(j=i+inc;i<j && i<len;i++)
				if (j >= len)
					pprintf("  ");
				else
				if ( is_printable(buf[i]) )
					print_color_byte_i(i, "%c", buf[i]);
				else	print_color_byte_i(i, ".", buf[i]);
			i--;
			D { NEWLINE; }
		}
		break;
	case FMT_ASCP:
		for(i=0;i<len;i++)
			if ( is_printable(buf[i]) )
				print_color_byte_i(i, "%c", buf[i]);
		NEWLINE;
		break;
	case FMT_WASC0:
		for(i=0;i<len && (buf[i]&&!buf[i+1]);i+=2)
			print_color_byte_i(i, "%c", buf[i]);
		NEWLINE;
		break;
	case FMT_ASC0:
		printf("%s", buf);
		NEWLINE;
		break;
	case FMT_ASC:
		for(i=0;i<len;i++)
			if ( !is_printable(buf[i]) )
				print_color_byte_i(i, "\\x%02x", buf[i]);
			else	pprintf("%c", buf[i]);
		NEWLINE;
		break;
	case FMT_HEXQ: {
		long long int *sh;
		for(i=0;i<len;i+=8) {
			endian_memcpy(buffer, buf+i, 8);
			sh = (long long int*)&buffer;
			print_color_byte_i(i, "%016llx ", (long long int)sh[0]);
			pprintf("0x%016llx ", (long long int)sh[0]);
			D {NEWLINE;}
		} D{}else NEWLINE;
		} break;
	case FMT_HEXD: {
		unsigned int *sh;
		for(i=0;i<len;i+=4) {
			endian_memcpy(buffer, buf+i, 4);
			sh = (unsigned int *)buffer;
			//print_color_byte_i(i, "%08x ", sh[0]);
			pprintf("0x%08x ", sh[0]);
			D { NEWLINE; }
		} D{}else NEWLINE;
		} break;
	case FMT_HEXW: {
		unsigned short *sh;
		for(i=0;i<len;i+=2) {
			endian_memcpy(buffer, buf+i, 2); //sizeof(short));
			sh = (unsigned short *)&buffer;
			//print_color_byte_i(i, "%04x ", sh[0]);
			pprintf("0x%04x", sh[0]);
			D { NEWLINE; }
		}  D{}else NEWLINE;
		} break;
	case FMT_ZOOM: {
		char *buf = NULL;
		unsigned long sz = 4;
		const char *mode = config_get("zoom.byte");
		off_t ptr = config.zoom.from;
	
		if (!mode)
			break;

		config.zoom.piece = config.size / config.block_size ;
		print_fmt = FMT_HEXB;
		buf = (char *)malloc(config.zoom.piece+10);

		switch(mode[0]) {
		case 'e':
			buf = (char *)malloc(config.zoom.piece);
			sz = (unsigned long)config.zoom.piece;
			break;
		default:
			buf = (char *)malloc(10);
			break;
		}
		for(i=0;i<config.block_size;i++) {
			io_lseek(config.fd, ptr, SEEK_SET);
			io_read(config.fd, buf, sz);
			switch(mode[0]) {
			case 'p': // % printable chars
				config.block[i] = (unsigned char)2.55*hash_pcprint(buf, sz);
				break;
			case 'e': // entropy
				config.block[i] = (unsigned char)hash_entropy(buf, sz);
				break;
			//case 'f': // first
			default:
				config.block[i] = buf[0];
				break;
			}
			ptr += config.zoom.piece;
		}
		free(buf);
		}
	case FMT_HEXBS:
	case FMT_HEXB:
		D inc = 2+(int)((config.width-14)/4);
		else inc = 2+(int)((config.width)/4);
		if (inc%2) inc++;
		D if ( print_fmt == FMT_HEXB ) {
			C pprintf(COLOR_HD);
			INILINE;
			pprintf("   offset  ");
			for (i=0; i<inc; i++) {
				for(j=i; j>15; j-=15) j--;
				pprintf(" %c", hex[j]);
				if (j%2) pprintf(" ");
			}
			for (i=0; i<inc; i++) {
				for(j=i; j>15; j-=15) j--;
				pprintf("%c", hex[j]);
			}
			NEWLINE;
			pprintf(".--------+");
			for (i=0; i<inc; i++)
				pprintf((i%2)?"---":"--");

			pprintf("+");
			for (i=0; i<inc; i++)
				pprintf("-");
			pprintf("-");
			NEWLINE;
	//		C pprintf("\e[0m");
		}
		for(i=0; i<len; i+=inc) {
			V if ((i/inc)+5>config.height) return;
			D { if ( print_fmt == FMT_HEXB )
				print_addr(seek+i+config.baddr);
			} else {
				INILINE; 
			}

			for(j=i;j<i+inc;j++) {
				if (print_fmt==FMT_HEXB) {
					if (j>=len) {
						pprintf("  ");
						if (j%2) pprintf(" ");
						continue;
					}
				} else if (j>=len) break;
				print_color_byte_i(j, "%02x", (unsigned char)buf[j]);

				if (print_fmt == FMT_HEXBS || j%2) pprintf(" ");
			}

			if (print_fmt == FMT_HEXB) {
				for(j=i; j<i+inc; j++) {
					if (j >= len)
						pprintf(" ");
					else
					if ( is_printable(buf[j]) )
						print_color_byte_i(j, "%c", buf[j]);
					else	print_color_byte_i(j, ".", buf[j]);
				}
				NEWLINE;
			}
		}
		if (print_fmt == FMT_HEXBS) { NEWLINE; }
		INILINE;
		break;
	default:
		eprintf("Don't know how to print %d\n", print_fmt);
	}

	if (mode & FMT_INV)
		free(bufi);

	fflush(stdout);
}

/** Read the device and print a block
 *
 * /arg: optional length specifier (otherwise block_size; only MD_BLOCK)
 * /print_fmt: the format for the print
 * /mode: the output mode
 */
void radare_print(char *arg, print_fmt_t print_fmt, print_mode_t mode)
{
	int bs;

	if (radare_read(0) < 0) {
		eprintf("Error reading: %s\n", strerror(errno));
		return;
	}

	if ( arg[0] != '\0' ) {
		bs = get_math(arg);
		if (bs > config.block_size)
			bs = config.block_size;
	} else bs = config.block_size;

	if (config.limit && bs > config.limit - config.seek)
		bs = config.limit - config.seek;

	if (config.limit && config.seek >= config.limit) {
		D eprintf("End of file reached.\n");
		return;
	}

	data_print(config.seek, config.block, bs, print_fmt, mode);
}
