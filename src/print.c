/*
 * Copyright (C) 2007, 2008, 2009
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
#include "rahash/hash.h"
#include "plugin.h"
#include "rdb.h"
#include "list.h"
#if __UNIX__
#include <termios.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

void udis_arch(int arch, int len, int rows);
int inc = 16;
int dec = 16;

format_info_t formats[] = {
//	{ '8', FMT_87BIT,       "print 8bit block in raw 7bit",         NULL,   "entire block" },
	{ '%', FMT_PERCENT,    "print scrollbar of seek",NULL,          "entire file" },
	{ '=', FMT_BARS,       "print line bars for each byte", NULL,   "entire file" },
	{ 'a', FMT_ASC,        "ascii",                  NULL,          "entire block" },
	{ 'A', FMT_ASCP,       "ascii printable",        NULL,          "entire block" },
	{ 'b', FMT_BIN,        "binary",                 "N bytes",     "entire block" },
	{ 'B', FMT_LSB,        "LSB Stego analysis",     "N bytes",     "entire block" },
	{ 'c', FMT_CSTR,       "C format",               "N bytes",     "entire block" },
	//{ 'h', FMT_SHORT,      "half word (short)",      "2 bytes",     "(endian)"},
	{ 'd', FMT_DISAS,      "disassembly N opcodes",  "bsize bytes", "entire block" },
	{ 'D', FMT_UDIS,       "asm.arch disassembler",  "bsize bytes", "entire block" },
	{ 'e', FMT_DOUBLE,     "double",                 "8 bytes",     "(endian)"},
	{ 'F', FMT_TIME_FTIME, "windows filetime",       "8 bytes",     "(endian)"},
	{ 'f', FMT_FLOAT,      "float",                  "4 bytes",     "(endian)"},
	{ 'i', FMT_INT,        "integer",                "4 bytes",     "(endian)"},
	{ 'l', FMT_LONG,       "long",                   "4 bytes",     "(endian)"},
	{ 'L', FMT_LLONG,      "long (ll for long long)","4/8 bytes",   "(endian)"},
	{ 'm', FMT_MEMORY,     "print memory structure", "0xHHHH",      "fun args"},
	{ 'C', FMT_COMMENT,    "comment information",    "string",      "range"},
	{ 'o', FMT_OCT,        "octal dump",             "N bytes",     "entire block" },
	{ 'O', FMT_ZOOM,       "Overview (zoom.type)",   "entire file", "entire block" },
	{ 'p', FMT_PRINT,      "cmd.prompt",             NULL,          "entire block" },
	{ 'r', FMT_RAW,        "raw ascii",              NULL,          "entire block" },
	{ 'R', FMT_REF,        "reference",              NULL,          "entire block" },
	{ 's', FMT_ASHC,       "asm shellcode",          NULL,          "entire block" },
	{ 't', FMT_TIME_UNIX,  "unix timestamp",         "4 bytes",     "(endian)"},
	{ 'T', FMT_TIME_DOS,   "dos timestamp",          "4 bytes",     "(endian)"},
	{ 'u', FMT_URLE,       "URL encoding",           NULL,          "entire block" },
	{ 'U', FMT_USER,       "executes cmd.user",      NULL,          "entire block" },
	{ 'v', FMT_VISUAL,     "executes cmd.vprompt",   NULL,          "entire block" },
	{ '1', FMT_HEXBS,      "p1: 1byte,  8 bit hex pair",	 "1 byte",      "entire block" },
	{ '2', FMT_HEXW,       "p2: 2bytes, 16 bit hex word",   "2 bytes",     "(endian)"},
	{ '4', FMT_HEXD,       "p4: 4bytes, 32 bit hex dword",  "4 bytes",     "(endian)"},
	{ '6', FMT_BASE64,     "p6: base64 encode (p9 to decode)",  "entire block",     "(endian)"},
	{ '7', FMT_7BIT,       "7bit encoding (sms)",    NULL,   "entire block" },
	{ '8', FMT_HEXQ,       "p8: 8bytes, 64 bit quad-word",  "8 bytes",     "(endian)"},
	{ '9', FMT_EBASE64,    "p9: base64 decode (p6 to encode)",  "entire block",     "(endian)"},
	{ 'x', FMT_HEXB,       "hexadecimal dump", "N byte",      "entire block" },
	{ 'X', FMT_HEXPAIRS,   "hexpairs", "N byte",      "entire block" },
	{ 'z', FMT_ASC0,       "ascii null terminated",  NULL,          "until \\0" },
	{ 'Z', FMT_WASC0,      "wide ascii null end",    NULL,          "until \\0" },
	{ 0, 0, NULL, NULL, NULL }
};

void getHTTPDate(char *DATE)
{
	DATE[0]=0;
#if __UNIX__
	struct tm curt; /* current time */
	time_t l;
	char week_day[4], month[4];
	char *week_str[7]= { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	char *month_str[12]= { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
		"Aug", "Sep", "Oct", "Nov", "Dec" };

	l = time(0);
	localtime_r(&l, &curt);

	if ((curt.tm_wday <0 || curt.tm_wday > 6)
	||  (curt.tm_mon < 0 || curt.tm_mon > 11))
		return;

	sprintf(DATE, "%s, %02d %s %d %02d:%02d:%02d GMT", 
		week_str[curt.tm_wday],
		curt.tm_mday,
		month_str[curt.tm_mon],
		curt.tm_year + 1900, curt.tm_hour, 
		curt.tm_min, curt.tm_sec);
#else
#warning getHTTPdate now implemented for this platform
#endif
}


/* msdos date format */
// From freebsd kernel msdosfs/direntry.h
void print_msdos_date(unsigned char _time[2], unsigned char _date[2])
{
        unsigned int t       = _time[1]<<8 | _time[0];
        unsigned int d       = _date[1]<<8 | _date[0];
        unsigned int year    = ((d&0xfe00)>>9)+1980;
        unsigned int month   = (d&0x01e0)>>5;
        unsigned int day     = (d&0x001f)>>0;
        unsigned int hour    = (t&0xf800)>>11;
        unsigned int minutes = (t&0x07e0)>>5;
        unsigned int seconds = (t&0x001f)<<1;

        /* la data de modificacio del fitxer, no de creacio del zip */
        printf("%d-%02d-%02d %d:%d:%d",
                year, month, day, hour, minutes, seconds);
}

/** Get the specified format
 *
 * In case of an invalid format, returns FMT_ERR.
 * 
 * /fmt: the format specifier
 * /mode: the supported format modes
 *
 * /ret: the specified format
 */
print_fmt_t format_get (char fmt)
{
	format_info_t *fi;

	for (fi = formats; fi->id != 0; fi++)
		if (fi->id == fmt)
			return fi->print_fmt;

	return FMT_ERR;
}

void format_show_help (print_mode_t mode)
{
	format_info_t *fi;
	cons_printf("Available formats:\n");
	for (fi = formats; fi->id != 0; fi++)
		cons_printf(" p%c : %-23s %s\n", fi->id, fi->name, fi->sizeo); // sizeb?
	cons_flush();
}

/* helpers */

void print_addr(u64 off)
{
	int mod = config_get_i("cfg.addrmod");
	char ch = (0==(off%(mod?mod:1)))?',':' ';
	
	C {
		if (off==config.cursor_ptr+config.vaddr)
			cons_invert();
		cons_printf("%s0x%08llx"C_RESET"%c ", 
			(off==config.cursor_ptr+config.vaddr)?C_WHITE:
			cons_palette[PAL_ADDRESS], off, ch);
	} else	cons_printf("0x%08llx%c ", off, ch);
}

// TODO: move to console
const char *get_color_for(int c)
{
	if (c==0)    return cons_palette[PAL_00];
	if (c==0xff) return cons_palette[PAL_FF];
	if (c==0x7f) return cons_palette[PAL_7F];
	if (is_printable(c)) return cons_palette[PAL_PRINTABLE];
	return "";
}

void print_color_byte(char *str, int c)
{
	char cash[128];

	C {	strcpy(cash, get_color_for(c));
		strcat(cash, str);
		strcat(cash, C_RESET);
		cons_printf(cash, (unsigned char)c);
	} else cons_printf(str, c);
}

int is_cursor(int from, int len)
{
	int end = from+len;
	if (config.cursor_mode == 0)
		return 0;

	//for(;from<end;from++) {
		if (config.ocursor != -1) {
			if ((from >= config.ocursor && from <= config.cursor)
			||  (from <= config.ocursor && from >= config.cursor))
				return 2;
		} else {
			if (len > 1) {
				if (from < config.cursor && config.cursor < end)
					return 2;
			}
			if  (from == config.cursor)
				return 1;
		}
	//}
	return 0;
}

void print_color_byte_i(int i, char *str, int c)
{
	C {
		flag_t *f = flag_by_offset(config.seek+config.vaddr+i);
		if (f) cons_strcat("\x1b[44m");
		else cons_strcat("\x1b[0m");
	}
	if (is_cursor(i, 1)) {
		cons_invert(1);
		print_color_byte(str, c);
		cons_invert(0);
	} else print_color_byte(str, c);
}

void radare_dump_and_process(int type, int size)
{
	char cmd[1024];
	char file[TMPFILE_MAX];
	const char *objdump = config_get("asm.objdump");
	const char *syntax  = config_get("asm.syntax");
	int ret;

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

/* 7/8bit encoding for SMS from libgsmd */
// FROM GSMD
int packing_7bit_character(char *src, char *dest)
{
        int i,j = 0;
        unsigned char ch1, ch2;
        char tmp[2];
        int shift = 0;

        *dest = '\0';

        for ( i=0; i<strlen(src); i++ ) {

                ch1 = src[i] & 0x7F;
                ch1 = ch1 >> shift;
                ch2 = src[(i+1)] & 0x7F;
                ch2 = ch2 << (7-shift);

                ch1 = ch1 | ch2;

                j = strlen(dest);
                sprintf(tmp, "%x", (ch1 >> 4));
                dest[j++] = tmp[0];
                sprintf(tmp, "%x", (ch1 & 0x0F));
                dest[j++] = tmp[0];
                dest[j++] = '\0';

                shift++;

                if ( 7 == shift ) {
                        shift = 0;
                        i++;
                }
        }

        return 0;
}

int unpacking_7bit_character(char *src, char *dest)
{
        unsigned char ch1, ch2 = '\0';
        int i, j;
        char buf[8];
        int shift = 0;

        *dest = '\0';

        for ( i=0; i<strlen(src); i+=2 ) {
                sprintf(buf, "%c%c", src[i], src[i+1]);
                ch1 = strtol(buf, NULL, 16);

                j = strlen(dest);
                dest[j++] = ((ch1 & (0x7F >> shift)) << shift) | ch2;
                dest[j++] = '\0';

                ch2 = ch1 >> (7-shift);

                shift++;
        }

        return 0;
}

void print_mem_help()
{
	eprintf(
	"Usage: pm [times][format] [arg0 arg1]\n"
	"Example: pm 10xdz pointer length string\n"
	"Example: pm {array_size}b @ array_base\n"
	"Example: pm x[foo]b @ esp\n"
	" e - little endian\n"
	" E - big endian\n"
	//" D - double (8 bytes)\n"
	" f - float value\n"
	" b - one byte \n"
	" B - show 10 first bytes of buffer\n"
	" d - %%d integer value (4 bytes)\n"
	" D - double value (4 bytes)\n"
	" q - quadword (8 bytes)\n"
	//" p - pointer reference\n"
	" x - 0x%%08x hexadecimal value\n"
	" X - 0x%%08x hexadecimal value and flag (fd @ addr)\n"
	" z - \\0 terminated string\n"
	" Z - \\0 terminated wide string\n"
	" s - pointer to string\n"
	" t - unix timestamp string\n"
	" * - next char is pointer\n"
	" . - skip 1 byte\n"
	" : - skip 4 bytes\n"
	" {}- used to eval math expressions to repeat next fmt char\n"
	" []- used to nest format structures registered with 'am'\n"
	" %1,%2,%4,%8 - type size (default is asm.bits/8)\n"
	"NOTE: Use 'am' command to register inner structs\n");
	/* TODO: add 1,2,4,8 pointer sizes support */
}

struct list_head print_mems;

void print_mem_add(char *name, char *fmt)
{
	print_mem_t *pm = MALLOC_STRUCT(print_mem_t);
	while(name[0]==' ')name=name+1;
	while(fmt[0]==' ')fmt=fmt+1;
	if (!*name || !*fmt) {
		eprintf("Usage: am foo xxd\n");
		return;
	}
	pm->name = strdup(name);
	pm->fmt = strdup(fmt);
	list_add_tail(&(pm->list), &(print_mems));
}

void print_mem_del(char *name)
{
	struct list_head *pos;
	list_for_each_prev(pos, &print_mems) {
		print_mem_t *im = list_entry(pos, print_mem_t, list);
		if (!strcmp(name, im->name)) {
			free(im->name);
			free(im->fmt);
			list_del(&(im->list));
			free(im);
			return;
		}
	}
}

void print_mem_list(char *name, char *fmt)
{
	struct list_head *pos;
	list_for_each_prev(pos, &print_mems) {
		print_mem_t *im = list_entry(pos, print_mem_t, list);
		cons_printf("%s %s\n", im->name, im->fmt);
	}
}

const char *print_mem_get(char *name)
{
	struct list_head *pos;
	list_for_each_prev(pos, &print_mems) {
		print_mem_t *im = list_entry(pos, print_mem_t, list);
		if (!strcmp(name, im->name))
			return im->fmt;
	}
	return NULL;
}

/* TODO: add support for 64bit pointers */
/* following asm.bits ??? */
void print_mem(u64 addr, const u8 *buf, u64 len, const char *fmt, int endian)
{
	unsigned char buffer[256];
	int i,j,idx;
	int times, otimes;
	char tmp, last;
	char *args, *bracket;
	int nargs;
	u64 paddr = 0LL;
	const char *arg = fmt;
	char hexfmt[32];
	int ptrsize = (int)(config_get_i("asm.bits")/8);
	int optrsize = ptrsize;
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
			switch (ptrsize) {
			case 1:
				addr =  (u64)(*(buf+i));
				strcpy(hexfmt, "0x%02llx");
				break;
			case 2:
				if (endian) 
					addr =  (u64)(*(buf+i))<<8 | (u64)(*(buf+i+1));
				else
					addr =  (u64)(*(buf+i+1))<<8 | (u64)(*(buf+i));
				strcpy(hexfmt, "0x%04llx");
				break;
			case 4:
				if (endian) 
					addr =  (u64)(*(buf+i))<<24 | (u64)(*(buf+i+1))<<16 | 
							(u64)(*(buf+i+2))<<8 | (u64)(*(buf+i+3));
				else 
					addr =  (u64)(*(buf+i+3))<<24 | (u64)(*(buf+i+2))<<16 | 
							(u64)(*(buf+i+1))<<8 | (u64)(*(buf+i));
				strcpy(hexfmt, "0x%08llx");
				break;
			case 8:
				if (endian) 
					addr =  (u64)(*(buf+i))<<56 | (u64)(*(buf+i+1))<<48 | 
							(u64)(*(buf+i+2))<<40 | (u64)(*(buf+i+3))<<32 | 
							(u64)(*(buf+i+4))<<24 | (u64)(*(buf+i+5))<<16 | 
							(u64)(*(buf+i+6))<<8 | (u64)(*(buf+i+7));
				else 
					addr =  (u64)(*(buf+i+7))<<56 | (u64)(*(buf+i+6))<<48 | 
							(u64)(*(buf+i+5))<<40 | (u64)(*(buf+i+4))<<32 | 
							(u64)(*(buf+i+3))<<24 | (u64)(*(buf+i+2))<<16 | 
							(u64)(*(buf+i+1))<<8 | (u64)(*(buf+i));
				strcpy(hexfmt, "0x%016llx");
			}
			tmp = *arg;
		feed_me_again:
			if (tmp == 0 && last != '*')
				break;
			/* skip chars */
			switch(tmp) {
			case '%':
				idx--;
				arg = arg + 1;
				switch(*arg) {
				case '1':
					ptrsize = 1;
					continue;
				case '2':
					ptrsize = 2;
					continue;
				case '4':
					ptrsize = 4;
					continue;
				case '8':
					ptrsize = 8;
					continue;
				}
			case ' ':
				config.interrupted =1;
				//i = len; // exit
				continue;
			case '*':
				if (i<=0) break;
				tmp = last;
				arg = arg - 1;
				idx--;
				goto feed_me_again;
			case 'e': // little endian
				idx--;
				endian =0;
				continue;
			case 'E': // big endian
				idx--;
				endian =1;
				continue;
			case '.': // skip char
				i++;
				idx--;
				continue;
			case ':': // skip 4 bytes
				i+=4;
				idx--;
				continue;
			case '?': // help
				print_mem_help();
				idx--;
				i=len; // exit
				continue;
			case '[':
				{
					char *fmt, *end = strchr(arg,']');
					if (!end) {
						eprintf("Missing '['\n");
						return;
					}
					*end='\0';
					fmt = print_mem_get(arg+1);
					if (fmt) {
#if 0 
						D cons_printf("0x%08llx = ", config.seek+i);
						paddr = buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3];
						cons_printf("0x%08llx -> pm %s {\n", addr, fmt);
#endif 
						cons_printf("pm %s {\n", fmt);
						/* XXX: this is 32bit pointer only FUCK! */
						print_mem(addr, buf+i, config.block_size, fmt, endian); //config.endian);
						cons_printf("}\n");
					} else {
						eprintf("Unknown named print format '%s' (Use 'am name format')\n", arg+1);
					}
					arg = end;
					idx--;
					i+=ptrsize;
				}
				continue;
			}
			if (idx<nargs)
				cons_printf("%10s : ", get0word(args, idx));
			/* cmt chars */
			switch(tmp) {
			case 't':
				/* unix timestamp */
				D cons_printf("0x%08llx = ", config.seek+i);
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
			case 'D': {
				double doub;
				memcpy(&doub, buf+i, sizeof(double));
				D cons_printf("%e = ", doub);
				cons_printf("(double)");
				}
				break;
			case 'b':
				D cons_printf("0x%08llx = ", config.seek+i);
				cons_printf("%d ; 0x%02x ; '%c' ", 
					buf[i], buf[i], is_printable(buf[i])?buf[i]:0);
				i++;
				break;
			case 'B':
				memset(buffer, '\0', 255);
				radare_read_at((u64)addr, buffer, 248);
				D cons_printf("0x%08llx = ", config.seek+i);
				for(j=0;j<10;j++) cons_printf("%02x ", buf[j]);
				cons_strcat(" ... (");
				for(j=0;j<10;j++)
					if (is_printable(buf[j]))
						cons_printf("%c", buf[j]);
				cons_strcat(")");
				i+=ptrsize;
				ptrsize = optrsize;
				break;
			case 'd':
				D cons_printf("0x%08llx = ", config.seek+i);
				cons_printf("%lld", addr);
				ptrsize = optrsize;
				i+=ptrsize;
				break;
			case 'x':
				D cons_printf("0x%08llx = ", config.seek+i);
				cons_printf(hexfmt, addr);
				i+=ptrsize;
				ptrsize = optrsize;
				break;
			case 'X': {
				char buf[128];
				D cons_printf("0x%08llx = ", config.seek+i);
				cons_printf(hexfmt, addr);
				if (string_flag_offset(buf, addr, -1))
					cons_printf(" ; %s", buf);
				i+=ptrsize;
				ptrsize = optrsize;
				} 
				break;
			case 'z': // zero terminated string
				D cons_printf("0x%08llx = ", config.seek+i);
				for(;buf[i]&&i<len;i++) {
					if (is_printable(buf[i]))
						cons_printf("%c", buf[i]);
					else cons_strcat(".");
				}
				break;
			case 'Z': // zero terminated wide string
				D cons_printf("0x%08llx = ", config.seek+i);
				for(;buf[i]&&i<len;i+=2) {
					if (is_printable(buf[i]))
						cons_printf("%c", buf[i]);
					else cons_strcat(".");
				}
				cons_strcat(" ");
				break;
			case 's':
				D cons_printf("0x%08llx = ", config.seek+i);
				memset(buffer, '\0', 255);
				radare_read_at((u64)addr, buffer, 248);
				cons_printf("0x%08llx -> ", (u64)config.seek+i);
				cons_printf(hexfmt, addr);
				cons_printf(" %s ", buffer);
				i+=ptrsize;
				ptrsize = optrsize;
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
	//D {} else cons_newline();
}

static int zoom = 0;

void print_zoom(u64 from, u64 to, char *byte, int enable)
{
	zoom = enable;
	config_set_i("zoom.from", from);
	config_set_i("zoom.to", to);
	config_set("zoom.byte", byte);
	eprintf("Zoom is %s\n", (zoom)?"enabled":"disabled");
}

/** Print some data.
 *
 * seek: position of the stream
 * buf:  buffer to print
 * len:       buffer length (only used in block mode)
 * fmt:  print format
 * mode: print mode
 *
 */
void print_data(u64 seek, char *arg, u8 *buf, int olen, print_fmt_t fmt)
{
	int tmp, i, j, k;
	int lines   = 0;
	int endian  = (int)config_get("cfg.bigendian");
	int inverse = (int)config_get("cfg.inverse");
	char *str;
	// code anal
	struct program_t *prg;
	unsigned char buffer[256];
	unsigned char *bufi = NULL, *bufz = NULL; // inverted buffer
	unsigned long addr = seek;
	u64 len = olen;

	last_print_format = fmt;
	if (buf == NULL)
		return;

	if (inverse) {
		bufi = (unsigned char *)malloc(len);
		for(i=0;i<len;i++)
			bufi[i] = buf[len-i-1];
		buf = bufi;
	}

	if (zoom) {
		u64 sz;
		u8 *bufz2;
		const char *mode = config_get("zoom.byte");
		u64 ptr = config_get_i("zoom.from");

		if (!mode)
			return;

		bufz = (u8 *)malloc(len);
		bufz2 = (u8 *)malloc(config.zoom.piece);
		sz = (u64)config.zoom.piece;

		for(i=0;!config.interrupted && i<len;i++) {
			io_lseek(config.fd, ptr, SEEK_SET);
			bufz2[0]='\xff';
			io_read(config.fd, bufz2, sz);

			switch(mode[0]) {
			case 'F': // 0xFF
				bufz[i] = 0;
				for(j=0;j<sz;j++)
					if (bufz2[j]==0xff)
						bufz[i]++;
				break;
			case 'c': // code
				{
				struct data_t *data = data_get_between(ptr, ptr+config.zoom.piece);
				bufz[i] = (unsigned char) 0;
				if (data->type == DATA_CODE)
					bufz[i] = (unsigned char) data->times;
				}
				break;
			case 's': // strings
				{
				struct data_t *data = data_get_between(ptr, ptr+config.zoom.piece);
				bufz[i] = (unsigned char) 0;
				if (data->type == DATA_STR)
					bufz[i] = (unsigned char) data->times;
				}
				break;
			case 't': // traces
				bufz[i] = (unsigned char)trace_get_between(ptr, ptr+config.zoom.piece);
				break;
			case 'f': // flags
				bufz[i] = (unsigned char)flags_between(ptr, ptr+config.zoom.piece);
				break;
			case 'p': // % printable chars
				bufz[i] = (unsigned char)2.55*hash_pcprint(bufz2, sz);
				break;
			case 'e': // entropy
				bufz[i] = (unsigned char)hash_entropy(bufz2, sz);
				break;
			//case 'h':
			default:
				bufz[i] = bufz2[0];
				break;
			}
			ptr += config.zoom.piece;
		}
		buf = bufz;
		free(bufz2);
	}

	if (len <= 0) len = config.block_size;
	radare_controlc();

	if (config.visual) {
		config.height = config_get_i("scr.height");
		config.height -= (config.scrdelta+3);
	}
	switch(fmt) {
	case FMT_7BIT:
		// TODO : use inverse for decoding
		for(i=0;!config.interrupted && i<len; i++) {
			packing_7bit_character(config.block+i, buffer);
			cons_printf("%c", buffer[0]);
		}
		cons_newline();
		break;
#if 0
	case FMT_87BIT:
		for(i=0;!config.interrupted && i<len; i++) {
			unpacking_7bit_character(config.block+i, buffer);
			cons_printf("%c", buffer[0]);
		}
		cons_newline();
		break;
#endif
	case FMT_COMMENT:
		//data_comment_list();
		data_xrefs_print(config.seek,-1);
			//print_datad(0);
		break;
	case FMT_BARS:
		{
			int i,j,pc,pce;
			for(i=0;i<len;i++) {
				if (zoom) print_addr(seek+(config.zoom.piece*i));
				else print_addr(seek);
				cons_printf(" %02x |", buf[i]);
				pc = (buf[i]*100)/255;
				//pce = 100-pc;
				pc = pc*config.width/116;
				pce = (100*config.width/116) - pc;
				// TODO: adatp pc/pce to screen width
				for(j=0;j<pc;j++) cons_strcat("#");
				for(j=0;j<pce;j++) cons_strcat(".");
				cons_strcat("|\n");
			}
		}
		break;
	case FMT_PERCENT: {
			int w = config.width-4;
			u64 s = config.size;
			u64 piece = 0;
			if (s==-1)
				s = 0x100000000LL; // XXX WTF
			piece = s/w;
			cons_printf("[");
			for(i=0;i<w;i++) {
				u64 from = (piece*i);
				C { struct data_t *d = data_get_between(from, from+piece);
//printf("%lld, %lld (piece =%lld\n", from , from+piece, piece);
				if (d != NULL)
					switch(d->type) {
					case DATA_STR: cons_strcat(C_RED); break;
					case DATA_HEX: cons_strcat(C_GREEN); break;
					case DATA_CODE: cons_strcat(C_YELLOW); break;
					case DATA_FUN: cons_strcat(C_MAGENTA); break;
				}	}
				
				if (config.seek >= piece*i && config.seek < (piece*(i+1)))
					cons_strcat("#");
				else
				if (flags_between(piece*i, piece*(i+1)))
					cons_strcat(".");
				else cons_strcat("_");
				C { cons_strcat(C_RESET); }
			}
			cons_strcat("]\n");
		}
		break;
	case FMT_PRINT:
		i = last_print_format;
		radare_cmd( config_get("cmd.print"), 0);
		last_print_format = i;
		break;
	case FMT_REF:
		{
		char buf[128];
		char *str = NULL;
		buf[0]='\0';
		sprintf(buf, "!!rsc dwarf-addrs '$FILE' 0x%llx", config.seek);
		str = pipe_command_to_string(buf);
		if (str) {
			cons_printf(str);
			free(str);
		}
		} break;

#if 0
		str = config_get("cmd.asm");
		str = pipe_command_to_string(str);
		if (str) {
			cons_printf(str);
			free(str);
		}
		break;
#endif
	case FMT_VISUAL:
		i = last_print_format;
		radare_cmd( config_get("cmd.visual"),0);
		last_print_format = i;
		break;
	case FMT_MEMORY:
		print_mem((u64)addr, (const u8*)buf, (u64)len, (char *)arg, (int)endian);
		break;
	case FMT_DISAS:
		radis(config.block_size, len);
		break;
	case FMT_CODEGRAPH:
		eprintf("THIS COMMAND IS GOING TO BE DEPRECATED. PLEASE USE 'ag'\n");
#if HAVE_GUI
		prg = code_analyze(config.vaddr + config.seek, config_get_i("graph.depth"));
		list_add_tail(&prg->list, &config.rdbs);
		grava_program_graph(prg, NULL);
#else
		eprintf("Compiled without valac/gtk/cairo\n");
#endif
		break;
	case FMT_UDIS:
		radis(len, 0);
		break;
	case FMT_SHORT: {
		short *s;
		for(i=0;!config.interrupted && i<len;i+=sizeof(short)) {
			endian_memcpy(buffer, buf+i, sizeof(short));
			s = (short *)buffer;
			print_color_byte("%hd", s[0]);
			cons_newline();
		} } break;
	case FMT_DOUBLE: {
		double f;
		for(i=0;!config.interrupted && i<len;i+=sizeof(float)) {
			endian_memcpy((u8*)&f, buf+i, sizeof(float));
			cons_printf("%e\n", f);
		} } break;
	case FMT_FLOAT: {
		float f;
		for(i=0;!config.interrupted && i<len;i+=sizeof(float)) {
			endian_memcpy((u8*)&f, buf+i, sizeof(float));
			cons_printf("%f\n", f);
		} } break;
	case FMT_INT:
		{
			int *iv;
			for(i=0;!config.interrupted && i<len;i+=sizeof(int)) {
				endian_memcpy(buffer, buf+i, sizeof(int));
				iv = (int *)buffer;
				print_color_byte("%d", iv[0]);
				cons_newline();
			}
		} break;
	case FMT_LONG:
		{
			int i;
			long *l;
			for(i=0;!config.interrupted && i<len;i+=sizeof(long)) {
				endian_memcpy(buffer, config.block+i, sizeof(long));
				l = (long *)buffer;
				print_color_byte("%ld", *l);
				//printf("%ld", *l);
				D { cons_newline(); } else cons_printf(" ");
			}
		} break;
	case FMT_LLONG:
	       {
			long long *ll;
			for(i=0;!config.interrupted && i<len;i+=sizeof(long long)) {
				endian_memcpy(buffer, config.block+i, sizeof(long long));
				ll = (long long *)buffer;
				cons_printf("%lld", *ll);
				D { cons_newline(); } else cons_strcat(" ");
			}
		} break;
	case FMT_LSB: {
		int length = len;
		int bit, byte = 0;
		char dbyte;
		int lsb = 0;
		/* original code from lsbstego.c of Roman Medina */
		for ( byte = 0 ; byte < length ; ) {
			dbyte = 0;
			for (bit = 0; bit <= 7; bit++, byte++) {
				// TODO handle inverse (backward)
				/* Obtain Least Significant Bit */
				lsb = config.block[byte] & 1;
				dbyte = dbyte | lsb << bit ;
			}

			if (is_printable(dbyte))
				cons_printf ("%c", dbyte);
		}
		cons_newline();
		} break;
	case FMT_USER: {
		const char *ptr = config_get("cmd.user");
		if (ptr && ptr[0])
			radare_cmd(ptr, 0);
		} break;
	/*   DATES   */
	case FMT_TIME_DOS: {
		unsigned char _time[2];
		unsigned char _date[2];
		int delta = config_get_i("cfg.tzdelta");
		for(i=0;!config.interrupted && i<len;i+=4) {
			endian_memcpy(_time, config.block+i, 2);
			endian_memcpy(_date, config.block+i+2, 2);
			print_msdos_date(_time, _date);
			// TODO: add tzdelta support
			cons_newline();
		} } break;
	case FMT_TIME_UNIX: {
		int delta = config_get_i("cfg.tzdelta");
		time_t t;
		char datestr[256];
		const char *datefmt;
		for(i=0;!config.interrupted && i<len;i+=sizeof(time_t)) {
			endian_memcpy((unsigned char*)&t, config.block+i, sizeof(time_t));
			//printf("%s", (char *)ctime((const time_t*)&t));
			t += (delta*3600); // time zone delta
			datefmt = config_get("cfg.datefmt");
			if (datefmt&&datefmt[0])
				tmp = strftime(datestr,256,datefmt,
					(const struct tm*)gmtime((const time_t*)&t));
			else 	tmp = strftime(datestr,256,"%d:%m:%Y %H:%M:%S %z",
					(const struct tm*)gmtime((const time_t*)&t));
			// TODO colorize depending on the distance between dates
			if (tmp) cons_printf("%s",datestr); else printf("*failed*");
			cons_newline();
		} } break;
	case FMT_TIME_FTIME: {
		int delta = config_get_i("cfg.tzdelta");
		unsigned long long l, L = 0x2b6109100LL;
		time_t t;
		char datestr[256];
		const char *datefmt;
		for(i=0;!config.interrupted && i<len;i+=8) {
			endian_memcpy((unsigned char*)&l, config.block+i, sizeof(unsigned long long));
			l /= 10000000; // 100ns to s
			l = (l > L ? l-L : 0); // isValidUnixTime?
			t = (time_t) l; // TODO limit above!
			t += (delta*3600); // time zone delta
			datefmt = config_get("cfg.datefmt");
			if (datefmt&&datefmt[0])
				tmp = strftime(datestr, 256, datefmt,
					(const struct tm*)gmtime((const time_t*)&t));
			else 	tmp = strftime(datestr, 256, "%d:%m:%Y %H:%M:%S %z",
					(const struct tm*)gmtime((const time_t*)&t));
			if (tmp) cons_printf("%s", datestr); else cons_printf("*failed*");
			cons_newline();
		} } break;
	case FMT_RAW:
		// XXX TODO: measure the string length and make it fit properly
		V i = config.width*config.height; else i=len;
		write(1, buf, (len>i)?i:len);
		break;
	case FMT_URLE:
		for(i = 0; i < len; i++) {
			if (config.verbose&&is_printable(config.block[i]))
				print_color_byte_i(i, "%c", config.block[i]);
			else 	print_color_byte_i(i, "%%%02x", config.block[i]);
		}
		cons_newline();
		break;
	case FMT_ASHC:
		str = flag_name_by_offset(seek);
		if (!*str) str = "shellcode";
                cons_printf("%s:", str);
		inc = config.width/7;
		if (inc<1)inc = 1;
		lines = 0;
                for(i = 0; !config.interrupted && i < len; i++) {
			V if (lines>config.height-4)
					break;
                        if (!(i%inc)) {
				if (++lines>config.height-4) 
					break;
				cons_newline(); cons_printf(".byte ");
			}
			print_color_byte_i(i, "0x%02x", config.block[i]);
                        if (((i+1)%inc) && i+1<len) cons_printf(", ");
                }
		cons_newline();
                cons_printf(".equ %s_len, %d", str, len); cons_newline();
                break;
	case FMT_CSTR:
		inc = config_get_i("scr.bytewidth");
		if (!inc) inc = config.width/6;
		if (inc<1)inc = 1;
		cons_printf("#define _BUFFER_SIZE %d", len); cons_newline();
		cons_printf("unsigned char buffer[_BUFFER_SIZE] = {"); cons_newline();
		for(j = i = 0; !config.interrupted && i < len;) {
			print_color_byte_i(i, "0x%02x", config.block[i]);
			if (++i<len)  cons_printf(", ");
			if (!(i%inc)) {
				cons_newline(); 
				V if (++j+5>config.height)
					D if ((i/inc)+5 > config.height )
						break;
			}
		}
		cons_printf(" };\n");
		break;
	case FMT_BIN:
		inc = config_get_i("scr.bytewidth");
		if (!inc) inc = (int)((config.width-17)/11);
		if (inc<1)inc = 1;
		D {
			C cons_strcat(cons_palette[PAL_HEADER]);
			cons_printf("   offset ");
			for(i=0;i<inc;i++)
				cons_printf("     +0x%02x",i);
			cons_newline();
			C cons_strcat(C_RESET);
		}
		for(i=0; !config.interrupted && i<len; i++) {
			V if ((i/inc)+5>config.height) break;
			D print_addr(seek+i+config.vaddr);
			for(j = i+inc; i<j && i<len; i++) {
				C cons_printf(get_color_for(buf[i]));
				if (is_cursor(i,1))
					cons_strcat("\x1b[7m");
				PRINT_BIN(buf[i]);
				C cons_printf(C_RESET);
			}
			i--;
			D { cons_newline(); }
		}
		break;
	case FMT_OCT:
		inc = config_get_i("scr.bytewidth");
		if (!inc) inc = (int)((config.width)/6);
		if (inc<1)inc = 1;
		D {
			C cons_strcat(cons_palette[PAL_HEADER]);
			cons_printf("   offset   ");
			for(i=0;i<inc;i++)
				cons_printf("+%02x ",i);
			for(i=0;i<inc;i++)
				cons_printf("%c",hex[i%16]);
			cons_newline();
		}
		for(i=0;!config.interrupted && i<len;i++) {
			V if ((i/inc)+6>config.height) break;
			D print_addr(seek+i+config.vaddr);
			tmp = i;
			for(j=i+inc;i<j && i<len;i++) {
				print_color_byte_i(i, "%03o", (int)buf[i]);
				cons_printf(" ");
			}

			D { 
				if (i==len) for(;i<j;i++) cons_printf("    ");
				i = tmp;
				for(j=i+inc;i<j && i<len;i++) {
					if ( is_printable(buf[i]) )
						print_color_byte_i(i, "%c", buf[i]);
					else	print_color_byte_i(i, ".", buf[i]);
				}
			}
			cons_newline();
		}
		break;
	case FMT_ASCP:
		for(i=0;!config.interrupted && i<len;i++)
			if ( is_printable(buf[i]) )
				print_color_byte_i(i, "%c", buf[i]);
		cons_newline();
		break;
	case FMT_WASC0:
		for(i=0;!config.interrupted && i<len && (buf[i]&&!buf[i+1]);i+=2)
			print_color_byte_i(i, "%c", buf[i]);
		cons_newline();
		break;
	case FMT_ASC0:
		cons_printf("%s\n", buf);
		break;
	case FMT_ASC:
		for(i=0;!config.interrupted && i<len;i++)
			if ( !is_printable(buf[i]) )
				print_color_byte_i(i, "\\x%02x", buf[i]);
			else	cons_printf("%c", buf[i]);
		cons_newline();
		break;
	case FMT_HEXQ: {
		long long int *sh;
		for(i=0;!config.interrupted && i<len;i+=8) {
			endian_memcpy(buffer, buf+i, 8);
			sh = (long long int*)&buffer;
			cons_printf("0x%016llx ", (long long int)sh[0]);
			D {cons_newline();}
		} D{}else cons_newline();
		} break;
	case FMT_HEXD: {
		unsigned int *sh;
		for(i=0;!config.interrupted && i<len;i+=4) {
			endian_memcpy(buffer, buf+i, 4);
			sh = (unsigned int *)buffer;
			cons_printf("0x%02x%02x%02x%02x ", buffer[0],buffer[1],buffer[2],buffer[3]);
			D { cons_newline(); }
		} D{}else cons_newline();
		} break;
	case FMT_HEXW: {
		unsigned short *sh;
		for(i=0;!config.interrupted&&i<len;i+=2) {
			endian_memcpy(buffer, buf+i, 2); //sizeof(short));
			sh = (unsigned short *)&buffer;
			cons_printf("0x%02x%02x ", buffer[0],buffer[1]);
			D { cons_newline(); }
		}  D{}else cons_newline();
		} break;
	case FMT_ZOOM: {
		if (!zoom)
			config_set("zoom.enable", "true");
		print_data(seek, arg, buf,len, FMT_HEXB);
		break;
		}
	case FMT_HEXBS:
	case FMT_HEXB:
		inc = config_get_i("scr.bytewidth");
		if (!inc) {
			D inc = 2+(int)((config.width-14)/4);
			else inc = 2+(int)((config.width)/4);
		}
		if (inc%2) inc++;
		tmp = config.cursor_mode;
		D if ( fmt == FMT_HEXB ) {
			C cons_printf(cons_palette[PAL_HEADER]);
			cons_strcat("   offset   ");
			k = 0; // TODO: ??? SURE??? config.seek & 0xF;
			for (i=0; i<inc; i++) {
				cons_printf(" %c", hex[(i+k)%16]);
				if (i&1) cons_strcat(" ");
			}
			for (i=0; i<inc; i++)
				cons_printf("%c", hex[(i+k)%16]);
			cons_newline();
		}
		if (inc<1) inc = 1;
		for(i=0; !config.interrupted && i<len; i+=inc) {
			V if (inc==0 && (i/inc)+4>config.height) break;
			D { if ( fmt == FMT_HEXB ) {
				if (zoom) print_addr(seek+(config.zoom.piece*i));
				else print_addr(seek+i+config.vaddr);
			} }

			if (config.insert_mode==1) config.cursor_mode = 1;
			else if (config.insert_mode==2) config.cursor_mode = 0;

			for(j=i;j<i+inc;j++) {
				if (fmt==FMT_HEXB) {
					if (j>=len) {
						cons_printf("  ");
						if (j%2) cons_printf(" ");
						continue;
					}
				} else if (j>=len) break;
				print_color_byte_i(j, "%02x", (unsigned char)buf[j]);

				if (fmt == FMT_HEXBS || j%2) cons_printf(" ");
			}

			if (config.insert_mode==1)
				config.cursor_mode = 0;
			else 
			if (config.insert_mode==2)
				config.cursor_mode = 1;
			if (fmt == FMT_HEXB) {
				for(j=i; j<i+inc; j++) {
					if (j >= len)
						cons_printf(" ");
					else
					if ( is_printable(buf[j]) )
						print_color_byte_i(j, "%c", buf[j]);
					else	print_color_byte_i(j, ".", buf[j]);
				}
				cons_newline();
			}
		}
		config.cursor_mode = tmp;
		if (fmt == FMT_HEXBS)
			cons_newline();
		break;
	case FMT_HEXPAIRS:
		for(i=0;i<len;i++)
			cons_printf("%02x", buf[i]);
		cons_newline();
		break;
	case FMT_BASE64:
		{
		/* decode base64 */
		int i,o;
		char *out = malloc(len*2);
		memset(out,0,len*2);
		for(i=o=0;i<len;i+=4,o+=3) {
			/* XXX: properly control block boundaries */
			if (base64_decodeblock(buf+i, out+o)<0)
				break;
		}
		cons_strcat(out);
		}
		break;
	case FMT_EBASE64:
		{
		/* encode base64 */
		int i,o;
		char *out = malloc(len*2);
		memset(out,0,len*2);
		for(i=o=0;i<len;i+=3,o+=4) {
			/* XXX: properly control block boundaries */
			if (base64_encodeblock(buf+i, out+o, len-i)<0)
				break;
		}
		cons_strcat(out);
		}
		break;
	default:
		eprintf("Don't know how to print %d\n", fmt);
	}

	if (inverse)
		free(bufi);
	if (zoom)
		free(bufz);
	radare_controlc_end();
}

/** Read the device and print a block
 *
 * arg: optional length specifier (otherwise block_size; only MD_BLOCK)
 * fmt: the format for the print
 * mode: the output mode
 */
void radare_print(char *arg, print_fmt_t fmt)
{
	int obs, bs;
	if (radare_read(0) < 0)
		return;
	obs = 0;
	if ( fmt!=FMT_MEMORY && arg[0] != '\0' ) {
		bs = get_math(arg);
		if (bs > config.block_size) {
			/* Resize block */
			obs = config.block_size;
			radare_set_block_size_i (bs);
		}
	} else bs = config.block_size;

	if (config.limit && bs > config.limit - config.seek)
		bs = config.limit - config.seek;

	if (config.limit && config.seek >= config.limit) {
		//D eprintf("End of file reached.\n");
		return;
	}

	print_data(config.seek, arg, config.block, bs, fmt);

	if (obs != 0)
		radare_set_block_size_i (obs);
}
