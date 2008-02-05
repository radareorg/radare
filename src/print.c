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

#include "main.h"
#include "utils.h"
#include "plugin.h"
#include "rdb/rdb.h"
#include "list.h"
#if __UNIX__
#include <termios.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

float hash_entropy(char *data, size_t size);

int inc = 16;
int dec = 16;
format_info_t formats[] = {
	{ 'a', FMT_ASHC,       MD_BLOCK,  "asm shellcode",          NULL,         "entire block" },
	{ 'A', FMT_ANAL,       MD_BLOCK,  "analyze data",           NULL,         "entire block" },
	{ 'b', FMT_BIN,        MD_BLOCK,  "binary",                 "N bytes",    "entire block" },
	{ 'B', FMT_LSB,        MD_BLOCK,  "LSB Stego analysis",     "N bytes",    "entire block" },
	{ 'c', FMT_CSTR,       MD_BLOCK,  "C format",               "N bytes",    "entire block" },
	{ 'C', FMT_CODE,       MD_BLOCK,  "Code Analysis",          "N bytes",    "entire block" },
	{ 'h', FMT_SHORT,      MD_BLOCK,  "short",                 "2 bytes",     "(endian)"},
	{ 'd', FMT_DISAS,      MD_BLOCK,  "asm.objdump disassembly",  "bsize bytes", "entire block" },
	{ 'D', FMT_UDIS,       MD_BLOCK,  "asm.arch disassembler",   "bsize bytes", "entire block" },
	{ 'f', FMT_FLOAT,      MD_BLOCK,  "float",                 "4 bytes",     "(endian)"},
	{ 'F', FMT_TIME_FTIME, MD_BLOCK,  "windows filetime",      "8 bytes",     "(endian)"},
	{ 'G', FMT_CODEGRAPH,  MD_BLOCK,  "Code Analysis Graph",    "N bytes",    "entire block" },
	{ 'i', FMT_INT,        MD_BLOCK,  "integer",               "4 bytes",     "(endian)"},
	{ 'l', FMT_LONG,       MD_BLOCK,  "long",                  "4 bytes",     "(endian)"},
	{ 'L', FMT_LLONG,      MD_BLOCK,  "long long",             "8 bytes",     "(endian)"},
	{ 'm', FMT_MEMORY,     MD_BLOCK,  "print memory structure", "0xHHHH",        "fun args"},
	{ 'o', FMT_OCT,        MD_BLOCK,  "octal",                  "N bytes",    "entire block" },
	{ 'O', FMT_ZOOM,       MD_BLOCK,  "Zoom out view",          "entire file", "entire block" },
	{ 'p', FMT_PRINT,      MD_BLOCK,  "cmd.prompt",              NULL,         "entire block" },
	{ 'q', FMT_HEXQ,       MD_BLOCK,  "hexadecimal quad-word", "8 bytes",	  "(endian)"},
	{ 'r', FMT_RAW,        MD_BLOCK,  "raw ascii",              NULL,         "entire block" },
	{ 's', FMT_ASC,        MD_BLOCK,  "ascii",                  NULL,         "entire block" },
	{ 'S', FMT_ASCP,       MD_BLOCK,  "ascii printable",        NULL,         "entire block" },
	{ 't', FMT_TIME_UNIX,  MD_BLOCK,  "unix timestamp",        "4 bytes",	  "(endian)"},
	{ 'T', FMT_TIME_DOS,   MD_BLOCK,  "dos timestamp",         "4 bytes",	  "(endian)"},
	{ 'u', FMT_URLE,       MD_BLOCK,  "URL encoding",           NULL,         "entire block" },
	{ 'U', FMT_USER,       MD_BLOCK,  "executes cmd.user",      NULL,         "entire block" },
	{ 'v', FMT_VISUAL,     MD_BLOCK,  "executes cmd.vprompt",            NULL,         "entire block" },
	{ 'w', FMT_HEXW,       MD_BLOCK,  "hexadecimal word",      "2 bytes",	  "(endian)"},
	{ 'W', FMT_HEXD,       MD_BLOCK,  "hexadecimal dword",     "4 bytes",     "(endian)"},
	{ 'x', FMT_HEXB,       MD_BLOCK,  "hexadecimal byte pairs","N byte", 	  "entire block" },
	{ 'X', FMT_HEXBS,      MD_BLOCK,  "hexadecimal string",	   "N byte", 	  "entire block" },
	{ 'z', FMT_ASC0,       MD_BLOCK,  "ascii null terminated",  NULL,         "until \\0" },
	{ 'Z', FMT_WASC0,      MD_BLOCK,  "wide ascii null end",    NULL,         "until \\0" },
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
void print_addr(u64 off)
{
	C	cons_printf(COLOR_AD""OFF_FMT""C_RESET" ", off);
	else	cons_printf(OFF_FMT" ", off);
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
		cons_printf(cash, (unsigned char)c);
	} else
		cons_printf(str, c);
}

void cursor_precolor(int i)
{
	if (config.cursor_mode) {
		if (config.ocursor != -1) {
			if ((i >= config.ocursor && i <= config.cursor)
			||  (i <= config.ocursor && i >= config.cursor)) {
				cons_strcat("\e[7m");
				return;
			}
		} else {
			if (i == config.cursor) {
				cons_strcat("\e[7m");
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
				cons_strcat("\e[7m");
				print_color_byte(str, c);
				cons_strcat("\e[0m");
				return;
			}
		} else {
			if (i == config.cursor) {
				cons_strcat("\e[7m");
				print_color_byte(str, c);
				cons_strcat("\e[0m");
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
			cons_printf(cash, c);
		} else
			print_color_byte_i(i, str, 0);
	} else
		print_color_byte_i(i, str, 0);
}

void format_show_help (print_mode_t mode)
{
	format_info_t *fi;
	
	cons_printf("Available formats:\n");
	for (fi = formats; fi->id != 0; fi++)
	if (fi->mode & mode) {
		if (mode == MD_ONCE)
			cons_printf(" %c : %-23s %s\n", fi->id, fi->name, fi->sizeo);
		else	cons_printf(" %c : %-23s %s\n", fi->id, fi->name, fi->sizeb);
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
		D radare_cmd("fd", 0);
#if 0
		//x = config_get_i("scr.x");
		//y = config_get_i("scr.y");
		if (x&&y)
		sprintf(cmd,"%s %s %s | head -n %d | Y=%d W=%d awk 'BEGIN{W=ENVIRON[\"W\"];"
			"Y=ENVIRON[\"Y\"];E=\"\x1B\";ORS=\"\";print \"\" E \"[\" Y \";"
			"%dH\";Y++}{ORS=E \"[\" Y \";%dH\";"
			"if(/.:\\t/){print substr($_,0,W);Y++};}'",
			objdump, (syntax&&syntax[0]=='i')?"-M intel":"",file,
			config.visual?config.height:200, y, config.width, x,x);
		else
#endif
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
void data_print(u64 seek, char *arg, unsigned char *buf, int len, print_fmt_t print_fmt, print_mode_t mode)
{
	int tmp, i, j;
	int zoom = 0;
	unsigned char buffer[256];
	unsigned char *bufi; // inverted buffer
	unsigned long addr;
	int endian = config_get("cfg.endian");
	int lines = 0;
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
	radare_controlc();

	switch(print_fmt) {
	case FMT_ANAL:
		radare_analyze(seek, len);
		break;
	case FMT_PRINT:
		INILINE;
		i = last_print_format;
		radare_cmd( config_get("cmd.print"),0);
		last_print_format = i;
		break;
	case FMT_VISUAL:
		i = last_print_format;
		radare_cmd( config_get("cmd.visual"),0);
		last_print_format = i;
		break;
	case FMT_MEMORY:
		i = j = 0;
		// TODO: support skip n char  f.ex: pm i(3)s
		// TODO: automatic add comment C `pmxzx ??
		if (arg)
		for(;!config.interrupted && i<len&&*arg;arg=arg+1) {
			if (endian)
				 addr = (*(buf+i))<<24   | (*(buf+i+1))<<16 | *(buf+i+2)<<8 | *(buf+i+3);
			else     addr = (*(buf+i+3))<<24 | (*(buf+i+2))<<16 | *(buf+i+1)<<8 | *(buf+i);

			if (*arg == '*') {
				radare_read_at((u64)addr, buffer, 4);
				memcpy(&addr, buffer, 4);
				continue;
			}

			switch(*arg) {
			case 'e': // tmp swap endian
				endian ^=1;
				continue;
			case 'n': // enable newline
				j ^= 1;
				continue;
			case '.': // skip char
				i++;
				continue;
			case '?': // help
				cons_reset();
				cons_printf(
				"Usage: pm [format]\n"
				" e - temporally swap endian\n"
				" n - perform \\n after format\n"
				" b - one byte \n"
				" i - %%d integer value (4 byets)\n"
				" w - word (16 bit hexa)\n"
				" q - quadword (8 bytes)\n"
				" x - 0x%%08x hexadecimal value\n"
				" z - \\0 terminated string\n"
				" Z - \\0 terminated wide string\n"
				" s - pointer to string\n"
				" * - next char is pointer\n"
				" . - skip 1 byte\n");
				i=len; // exit
				continue;
			case 'q':
				D cons_printf("0x%08x  ", config.seek+i);
				cons_printf("(qword)");
				i+=8;
				break;
			case 'b':
				D cons_printf("0x%08x ", config.seek+i);
				cons_printf("%d ; 0x%02x ; '%c' ", buf[i], buf[i], is_printable(buf[i])?buf[i]:0);
				i++;
				break;
			case 'i':
				D cons_printf("0x%08x ", config.seek+i);
				cons_printf("%d", addr);
				i+=4;
				break;
			case 'x':
				D cons_printf("0x%08x ", config.seek+i);
				cons_printf("0x%08x ", addr);
				i+=4;
				break;
			case 'w': // word (16 bits)
				D cons_printf("0x%08x ", config.seek+i);
				if (endian)
					 addr = (*(buf+i))<<8  | (*(buf+i+1));
				else     addr = (*(buf+i+1))<<8 | (*(buf+i));
				cons_printf("0x%04x ", addr);
				break;
			case 'z': // zero terminated string
				D cons_printf("0x%08x  ", config.seek+i);
				for(;buf[i]&&i<len;i++) {
					if (is_printable(buf[i]))
						cons_printf("%c", buf[i]);
					else cons_strcat(".");
				}
				cons_strcat(" ");
				break;
			case 'Z': // zero terminated wide string
				D cons_printf("0x%08x  ", config.seek+i);
				for(;buf[i]&&i<len;i+=2) {
					if (is_printable(buf[i]))
						cons_printf("%c", buf[i]);
					else cons_strcat(".");
				}
				cons_strcat(" ");
				break;
			case 's':
				D cons_printf("0x%08x  ", config.seek+i);
				memset(buffer, '\0', 255);
				radare_read_at((u64)addr, buffer, 248);
				D cons_printf("0x%08x -> 0x%08x ", config.seek+i, addr);
				cons_printf("%s ", buffer);
				i+=4;
				break;
			default:
				continue;
			}
			D cons_printf("\n");
		}
		D {} else cons_printf("\n");
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
				cons_printf("0x%08x (%d) -> ", b0->addr, b0->n_bytes);
				if (b0->tnext)
					cons_printf("0x%08x", b0->tnext);
				if (b0->fnext)
					cons_printf(", 0x%08x", b0->fnext);
				cons_printf("\n");
				sprintf(cmd, "pD %d @ 0x%08x", b0->n_bytes+1, (unsigned int)b0->addr);
				radare_cmd(cmd, 0);
				cons_printf("\n\n");
			} else {
				cons_printf("b %d\n", b0->n_bytes);
				cons_printf("f blk_%08X @ 0x%08x\n", b0->addr, b0->addr);
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
		for(i=0;!config.interrupted && i<len;i+=sizeof(short)) {
			endian_memcpy(buffer, buf+i, sizeof(short));
			s = (short *)buffer;
			print_color_byte("%hd", s[0]);
			NEWLINE;
		} } break;
	case FMT_FLOAT: {
		float *f;
		for(i=0;!config.interrupted && i<len;i+=sizeof(float)) {
			endian_memcpy(buffer, buf+i, sizeof(float));
			f = (float *)buffer;
			print_color_byte("%f", f[0]);
			NEWLINE;
		} } break;
	case FMT_INT: {
		int *iv;
		for(i=0;!config.interrupted && i<len;i+=sizeof(int)) {
			endian_memcpy(buffer, buf+i, sizeof(int));
			iv = (int *)buffer;
			print_color_byte("%d", iv[0]);
			NEWLINE;
		} } break;
	case FMT_LONG: {
		int i;
		long *l;
		INILINE;
		for(i=0;!config.interrupted && i<len;i+=sizeof(long)) {
			endian_memcpy(buffer, config.block+i, sizeof(long));
			l = (long *)buffer;
			print_color_byte("%ld", *l);
			//printf("%ld", *l);
			D { NEWLINE; } else cons_printf(" ");
		} }
		break;
	case FMT_LSB: {
		int length = len;
		int bit, byte = 0;
		char dbyte;
		int lsb = 0;
		/* original code from lsbstego.c of Roman Medina */
		for ( byte = 0 ; byte < length ; )
		{
			dbyte = 0;

			for (bit = 0; bit <= 7; bit++, byte++)
			{
				// TODO handle inverse (backward)

				/* Obtain Least Significant Bit */
				lsb = config.block[byte] & 1;

				/* Add lsb to decrypted message */
				#if 0
				if (downward)
					dbyte = dbyte | lsb << (7-bit) ;
				else
				#endif
					dbyte = dbyte | lsb << bit ;
			}

			if (is_printable(dbyte))
				cons_printf ("%c", dbyte);
		}
		NEWLINE;
		} break;
	case FMT_USER: {
		const char *ptr = config_get("cmd.user");
		if (ptr && ptr[0])
			radare_cmd(ptr, 0);
		} break;
	case FMT_LLONG: {
		long long *ll;
		INILINE;
		for(i=0;!config.interrupted && i<len;i+=sizeof(long long)) {
			endian_memcpy(buffer, config.block+i, sizeof(long long));
 			ll = (long long *)buffer;
			cons_printf("%lld", *ll);
			D { NEWLINE; } else cons_printf(" ");
		} } break;
	/*   DATES   */
	case FMT_TIME_DOS: {
		unsigned char _time[2];
		unsigned char _date[2];
		for(i=0;!config.interrupted && i<len;i+=4) {
			endian_memcpy(_time, config.block+i, 2);
			endian_memcpy(_date, config.block+i+2, 2);
			print_msdos_date(_time, _date);
			NEWLINE;
		} } break;
	case FMT_TIME_UNIX: {
		time_t t;
		char datestr[256];
		const char *datefmt;
		INILINE;
		for(i=0;!config.interrupted && i<len;i+=4) {
			endian_memcpy((unsigned char*)&t, config.block+i, sizeof(time_t));
			//printf("%s", (char *)ctime((const time_t*)&t));
			datefmt = config_get("cfg.datefmt");

			if (datefmt&&datefmt[0])
				tmp = strftime(datestr,256,datefmt,
					(const struct tm*)gmtime((const time_t*)&t));
			else 	tmp = strftime(datestr,256,"%d:%m:%Y %H:%M:%S %z",
					(const struct tm*)gmtime((const time_t*)&t));
			// TODO colorize depending on the distance between dates
			if (tmp) cons_printf("%s",datestr); else printf("*failed*");
			NEWLINE;
		} } break;
	case FMT_TIME_FTIME: {
		unsigned long long l, L = 0x2b6109100LL;
		time_t t;
		char datestr[256];
		const char *datefmt;
		INILINE;
		for(i=0;!config.interrupted && i<len;i+=8) {
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
			if (tmp) cons_printf("%s", datestr); else cons_printf("*failed*");
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
                cons_printf("shellcode:");
		inc = config.width/7;
		lines = 0;
                for(i = 0; !config.interrupted && i < len; i++) {
			V if (lines>config.height-4)
					break;
                        if (!(i%inc)) {
				if (++lines>config.height-4) 
					break;
				NEWLINE; cons_printf(".byte ");
			}
			print_color_byte_i(i, "0x%02x", config.block[i]);
                        if (((i+1)%inc) && i+1<len) cons_printf(", ");
                }
		NEWLINE;
                cons_printf("shellcode_len = . - shellcode"); NEWLINE;
                break;
	case FMT_CSTR:
		D { INILINE; }
		inc = config.width/6;
		cons_printf("#define _BUFFER_SIZE %d", len); NEWLINE;
		cons_printf("unsigned char buffer[_BUFFER_SIZE] = {"); NEWLINE;
		for(j = i = 0; !config.interrupted && i < len;) {
			print_color_byte_i(i, "0x%02x", config.block[i]);
			if (++i<len)  cons_printf(", ");
			if (!(i%inc)) {
				NEWLINE; 
				V if (++j+5>config.height)
					D if ((i/inc)+5 > config.height )
						break;
			}
		}
		cons_printf(" };\n");
		break;
	case FMT_BIN:
		if (config.width<30)
			break;
		inc = (int)((config.width-17)/11);
		D {
			INILINE;
			C cons_printf(COLOR_HD);
			cons_printf("   offset");
			for(i=0;i<inc;i++)
				cons_printf("       +0x%x",i);
#if 0
			NEWLINE;
			cons_printf("----------+");
			for(i=0;i<inc;i++)
				cons_printf("-----------");
#endif
			NEWLINE;
			C cons_printf(C_RESET);
		}
		for(i=0; !config.interrupted && i<len; i++) {
			V if ((i/inc)+5>config.height) break;
			D print_addr(seek+i+config.baddr);
			for(j = i+inc; i<j && i<len; i++) {
				C cons_printf(get_color_for(buf[i]));
				cursor_precolor(i);
				PRINT_BIN(buf[i]);
				C cons_printf(C_RESET);
			}
			i--;
			D { NEWLINE; }
		}
		break;
	case FMT_OCT:
		inc = (int)((config.width)/6);
		D {
			C cons_printf(COLOR_HD);
			cons_printf("  offset   ");
			for(i=0;i<inc;i++)
				cons_printf("+%02x ",i);
			for(i=0;i<inc;i++)
				cons_printf("%c",hex[i%16]);
#if 0
			NEWLINE; cons_printf("----------+");
			for(i=0;i<inc;i++)
				cons_printf("----");
			for(i=0;i<inc;i++)
				cons_printf("-");
			C cons_printf(C_RESET);
#endif
			NEWLINE;
		}
		for(i=0;!config.interrupted && i<len;i++) {
			V if ((i/inc)+6>config.height) break;
			D print_addr(seek+i+config.baddr);
			tmp = i;
			for(j=i+inc;i<j && i<len;i++) {
				print_color_byte_i(i, "%03o", (int)buf[i]);
				cons_printf(" ");
			}
			i = tmp;
			for(j=i+inc;i<j && i<len;i++)
				if (j >= len)
					cons_printf("  ");
				else
				if ( is_printable(buf[i]) )
					print_color_byte_i(i, "%c", buf[i]);
				else	print_color_byte_i(i, ".", buf[i]);
			i--;
			D { NEWLINE; }
		}
		break;
	case FMT_ASCP:
		for(i=0;!config.interrupted && i<len;i++)
			if ( is_printable(buf[i]) )
				print_color_byte_i(i, "%c", buf[i]);
		NEWLINE;
		break;
	case FMT_WASC0:
		for(i=0;!config.interrupted && i<len && (buf[i]&&!buf[i+1]);i+=2)
			print_color_byte_i(i, "%c", buf[i]);
		NEWLINE;
		break;
	case FMT_ASC0:
		cons_printf("%s", buf);
		NEWLINE;
		break;
	case FMT_ASC:
		for(i=0;!config.interrupted && i<len;i++)
			if ( !is_printable(buf[i]) )
				print_color_byte_i(i, "\\x%02x", buf[i]);
			else	cons_printf("%c", buf[i]);
		NEWLINE;
		break;
	case FMT_HEXQ: {
		long long int *sh;
		for(i=0;!config.interrupted && i<len;i+=8) {
			endian_memcpy(buffer, buf+i, 8);
			sh = (long long int*)&buffer;
			print_color_byte_i(i, "%016llx ", (long long int)sh[0]);
			cons_printf("0x%016llx ", (long long int)sh[0]);
			D {NEWLINE;}
		} D{}else NEWLINE;
		} break;
	case FMT_HEXD: {
		unsigned int *sh;
		for(i=0;!config.interrupted && i<len;i+=4) {
			endian_memcpy(buffer, buf+i, 4);
			sh = (unsigned int *)buffer;
			//print_color_byte_i(i, "%08x ", sh[0]);
			cons_printf("0x%08x ", sh[0]);
			D { NEWLINE; }
		} D{}else NEWLINE;
		} break;
	case FMT_HEXW: {
		unsigned short *sh;
		for(i=0;!config.interrupted&&i<len;i+=2) {
			endian_memcpy(buffer, buf+i, 2); //sizeof(short));
			sh = (unsigned short *)&buffer;
			//print_color_byte_i(i, "%04x ", sh[0]);
			cons_printf("0x%04x", sh[0]);
			D { NEWLINE; }
		}  D{}else NEWLINE;
		} break;
	case FMT_ZOOM: {
		char *buf = NULL;
		unsigned long sz = 4;
		const char *mode = config_get("zoom.byte");
		u64 ptr = config.zoom.from;
	
		if (!mode)
			break;
		zoom = 1;

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
		for(i=0;!config.interrupted && i<len;i++) {
			io_lseek(config.fd, ptr, SEEK_SET);
			io_read(config.fd, buf, sz);
			switch(mode[0]) {
			case 'f': // flags
				config.block[i] = (unsigned char)flags_between(ptr, ptr+config.zoom.piece);
				break;
			case 'p': // % printable chars
				config.block[i] = (unsigned char)2.55*hash_pcprint(buf, sz);
				break;
			case 'e': // entropy
				config.block[i] = (unsigned char)hash_entropy(buf, sz);
				break;
			//case 'h':
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
			C cons_printf(COLOR_HD);
			cons_printf("   offset  ");
			for (i=0; i<inc; i++) {
				for(j=i; j>15; j-=15) j--;
				cons_printf(" %c", hex[j]);
				if (j%2) cons_printf(" ");
			}
			for (i=0; i<inc; i++) {
				for(j=i; j>15; j-=15) j--;
				cons_printf("%c", hex[j]);
			}
			NEWLINE;
		}
		for(i=0; !config.interrupted && i<len; i+=inc) {
			V if ((i/inc)+5>config.height) break;
			D { if ( print_fmt == FMT_HEXB )
				if (zoom) print_addr(seek+(config.zoom.piece*i));
				else print_addr(seek+i+config.baddr);
				
			} else { INILINE; }

			for(j=i;j<i+inc;j++) {
				if (print_fmt==FMT_HEXB) {
					if (j>=len) {
						cons_printf("  ");
						if (j%2) cons_printf(" ");
						continue;
					}
				} else if (j>=len) break;
				print_color_byte_i(j, "%02x", (unsigned char)buf[j]);

				if (print_fmt == FMT_HEXBS || j%2) cons_printf(" ");
			}

			if (print_fmt == FMT_HEXB) {
				for(j=i; j<i+inc; j++) {
					if (j >= len)
						cons_printf(" ");
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
	radare_controlc_end();
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

	data_print(config.seek, arg, config.block, bs, print_fmt, mode);
}
