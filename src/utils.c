/*
 * Copyright (C) 2006, 2007, 2008
 *       pancake <@youterm.com>
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
#include "flags.h"
#include "utils.h"
#include "plugin.h"
#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdarg.h>
#include <fcntl.h>
#include <time.h>
#if __Linux__
#include <sys/utsname.h>
#endif

#if __UNIX__
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/socket.h>
#endif

const char hex[16] = "0123456789ABCDEF";
const char *nullstr = "";
static const char * nullstr_c="(null)";

int iswhitespace(char ch)
{
   return (ch==' '||ch=='\t');
}

void eprintf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

char *estrdup(char *ptr, char *string)
{
        if (ptr)
                free(ptr);
        ptr = strdup(string);
        return ptr;
}

char *lstrchr(char *str, char chr)
{
	int len = strlen(str);
	for(;len>=0;len--)
		if (str[len]==chr)
			return str+len;
	return NULL;
}

int _strnstr(char *from, char *to, int size)
{
	int i;
	for(i=0;i<size;i++)
		if (from==NULL||to==NULL||from[i]!=to[i])
			break;
	return (size!=i);
}

char *slurp(const char *str)
{
	char *ret;	
	long sz;
	FILE *fd = fopen(str, "r");
	if (fd == NULL)
		return NULL;
	fseek(fd, 0,SEEK_END);
	sz = ftell(fd);
	fseek(fd, 0,SEEK_SET);
	ret = (char *)malloc(sz+1);
	fread(ret, sz, 1, fd);
	ret[sz]='\0';
	fclose(fd);
	return ret;
}

/*
 * Count the number of words in the given string
 *
 */
int word_count(const char *string)
{
	char *text = (char *)string;
	char *tmp  = (char *)string;
	int word   = 0;

	for(;(*text)&&(iswhitespace(*text));text=text+1);

	for(word = 0; *text; word++) {
		for(;*text && !iswhitespace(*text);text = text +1);
		tmp = text;
		for(;*text &&iswhitespace(*text);text = text +1);
		if (tmp == text)
			word-=1;
	}

	return word-1;
}

void memcpy_loop(u8 *dest, u8 *orig, int dsize, int osize)
{
	int i=0,j;
	while(i<dsize)
		for(j=0;j<osize && i<dsize;j++)
			dest[i++] = orig[j];
}

/* arch dependant */
void drop_endian(u8 *dest, u8 *orig, unsigned int size)
{
#if LIL_ENDIAN
	endian_memcpy_e(dest, orig, size, 0);
#else
	endian_memcpy_e(dest, orig, size, 1); // lilendian by default
#endif
}


void endian_memcpy(u8 *dest, u8 *orig, unsigned int size)
{
#if RADARE_CORE
	endian_memcpy_e(dest, orig, size, (int)config_get("cfg.bigendian"));
#else
	endian_memcpy_e(dest, orig, size, 0); // lilendian by default
#endif
}

void endian_memcpy_e(u8 *dest, u8 *orig, int size, int endian)
{
	if (endian) {
		memcpy(dest, orig, size);
	} else {
		unsigned char buffer[8];
		switch(size) {
		case 2:
			buffer[0] = orig[0];
			dest[0]   = orig[1];
			dest[1]   = buffer[0];
			break;
		case 4:
			memcpy(buffer, orig, 4);
			dest[0] = buffer[3];
			dest[1] = buffer[2];
			dest[2] = buffer[1];
			dest[3] = buffer[0];
			break;
		case 8:
			memcpy(buffer, orig, 8);
			dest[0] = buffer[7];
			dest[1] = buffer[6];
			dest[2] = buffer[5];
			dest[3] = buffer[4];
			dest[4] = buffer[3];
			dest[5] = buffer[2];
			dest[6] = buffer[1];
			dest[7] = buffer[0];
			break;
		default:
			printf("Invalid size: %d\n", size);
		}
	}
}

static char tmpdir[1024];
const char *get_tmp_dir()
{
	// TODO: Do not enter this function twice!
#if __WINDOWS__
	// http://msdn.microsoft.com/en-us/library/aa364992(VS.85).aspx
	GetTempPath(1023, &tmpdir);
#else
	const char *tmp;
#if RADARE_CORE
	tmp = config_get("dir.tmp");
#else
	tmp = getenv("TMP");
#endif
	if (tmp)
		strcpy(tmpdir, tmp);
	else
		strcpy(tmpdir, "/tmp/");
#endif
	return (const char *)&tmpdir;
}

int make_tmp_file(char *str)
{
	int fd;
	const char *tmp = get_tmp_dir();
	sprintf(str, "%s/.radare.tmp.XXXXXX", tmp?tmp:"./");
	fd = mkstemp(str);
	if ( fd == -1 ) {
		perror("mkstemp");
		*str = '\0';
	}
	return fd;
}

void progressbar(int pc)
{
#if RADARE_CORE
	int tmp, cols = config_get_i("scr.width");
#else
	int tmp, cols = 78;
#endif

	(pc<0)?pc=0:(pc>100)?pc=100:0;
	fprintf(stderr, "\x1b[K  %3d%% [", pc);
	cols-=15;
	for(tmp=cols*pc/100;tmp;tmp--) fprintf(stderr,"#");
	for(tmp=cols-(cols*pc/100);tmp;tmp--) fprintf(stderr,"-");
	fprintf(stderr, "]\r");
	fflush(stderr);
}

#if RADARE_CORE
// TODO get [ prefix: (size of ptr), use cfg.bigendian
//  4[0x300] = dword[0x300]
//  d[0x300] ...
//  b[0x300]
// eval cfg.bigendian
//unsigned char *ptr = &newa;
unsigned long get_pointer(u64 addr)
{
	unsigned long newa;
	u64 sk = config.seek;
	radare_seek(addr, SEEK_SET);
	io_read(config.fd, &newa, 4);
	radare_seek(sk, SEEK_SET);
	return newa;
}
#endif

/* Converts a string to u64 type. u64 jmp = get_offset("0x123456"); */
u64 get_offset(const char *orig)
{
	char arga[1024];
	char *arg = (char *)&arga;
	u64 ret = 0;
	int i, j;
#if RADARE_CORE
	flag_t *flag;
	char *ptr = 0;
#endif
	if (orig==NULL||orig[0]=='\0')
		return 0;

	strncpy(arg, orig, 1023);

	for(;*arg==' ';arg=arg+1);
	for(i=0;arg[i]==' ';i++);
	for(;arg[i]=='\\';i++); i++;

#if RADARE_CORE
	if (!strcmp(orig, "here"))
		return config.seek;
	ptr = strchr(arg, '[');
	if (ptr)
		return get_pointer(get_offset(ptr+1));

	ret = config_get_i(arg);
	if (((int)ret) != 0)
		return ret;

	if (arg[i]=='$') {
		if (arg[i+1]=='$') {
			struct aop_t aop;
			arch_aop(config.seek, config.block,&aop);
			ret = aop.length;
		} else {
			ret = config.seek;
		}
	} else {
		flag = flag_get(arg);
		if (flag)
			return flag->offset; // - config.baddr;
	}
#endif
	if (arg[i] == 'x' && i>0 && arg[i-1]=='0') {
		sscanf(arg, "0x"OFF_FMTx, &ret);
	} else {
		sscanf(arg, OFF_FMTd, &ret);

		switch(arg[strlen(arg)-1]) {
		case 'o': // octal
			sscanf(arg, "%llo", &ret);
			break;
		case 'b': // binary
			ret = 0;
			for(j=0,i=strlen(arg)-2;i>=0;i--,j++) {
				if (arg[i]=='1') ret|=1<<j; else
				if (arg[i]!='0') break;
			}
			break;
		case 'K': case 'k':
			ret*=1024;
			break;
		case 'M': case 'm':
			ret*=1024*1024;
			break;
		case 'G': case 'g':
			ret*=1024*1024*1024;
			break;
		}
	}

	return ret;
}

u32 get_offset32(u64 foo)
{
	return (u32) get_offset (foo);
}

char *mytok(char *ptr, char *delim, char *backup)
{
	char *tmp,*tmp2;

	if (ptr[0]=='\0')
		return NULL;

	for(tmp = ptr;tmp[0];tmp= tmp+1) {
		for(tmp2 = delim;tmp2[0];tmp2=tmp2+1) {
			if (tmp[0]==tmp2[0]) {
				if (backup)
					*backup = tmp[0];
				tmp[0]='\0';
				return ptr;
			}
		}
	}

	if (tmp[0]=='\0')
		return NULL;

	return ptr;
}

static int level = 0; // recursivity counter
u64 get_math(const char* text)
{
	u64 t;
	u64 new_off = 0;
	u64 cmp_off = 0;
	int is_cmp  = 0;
	int  sign   = 1;
	char op     = 0;
	char oop    = 0;
	char *txt, *tmp;
	char *ptr   = NULL;
#if RADARE_CORE
	char *end = NULL, *txt2 = NULL;
#endif
	/* AVOID STACK OVERFLOW */
	if (level++>5)
		return 0;

	if (text==NULL||text[0]=='\0') {
		level--;
		return 0;
	}

	/* remove whitespaces and dupped '=' */
	for(tmp = txt = strdup(text); txt && *txt; txt = txt+1) {
#if 1
		if ((txt[0]=='!' && txt[1]=='='))
			strcpy(txt+1, txt+2);
		else
#endif
		if ((txt[0]=='=' && txt[1]=='=') || (txt[0]==' '))
			strcpy(txt, txt+1);
	}
	txt = tmp;

#if RADARE_CORE
	txt2 = strdup(txt);
#endif
	sign = (*txt=='+')?1:(*txt=='-')?-1:0;
	for(ptr = txt; ptr && ptr[0]; ptr = ptr + strlen(ptr)+1)
	{
		tmp = mytok(ptr, "=!+-<>%*/[\\", &op);
		switch(oop) {
		case '!':
			if (is_cmp == 0) {
				is_cmp  = -1;
				cmp_off = new_off;
				new_off = get_offset(ptr);
			}
			break;
		case '=':
			if (is_cmp == 0) {
				is_cmp = 1;
				cmp_off = new_off;
				new_off = get_offset(ptr);
			}
			break;
#if RADARE_CORE
		case '[': end = strchr(txt2+(ptr-txt+1),']');
			// todo. support nested lol
			if (end) {
				end[0]='\0';
				new_off += get_pointer(get_math(txt2+(ptr-txt)));
				end[0]=']';
				ptr = ptr + (end-txt2);
			} else {
				eprintf("Unbalanced ']' (%s)\n", ptr);
			}
			break;
#endif
		case '+': new_off += get_offset(ptr); break;
		case '\\':  /* scaped chars */
			switch(ptr[0]) {
			case '|': new_off |= get_offset(ptr+1); break;
			case '&': new_off &= get_offset(ptr+1); break; }
			ptr = ptr + 1;
			break;
		case '%': { t = get_offset(ptr);
			if (t == 0) {
				printf("Mod by zero?\n");
				break;
			} else new_off %= t;
			} break;
		case '<': new_off <<= get_offset(ptr); break;
		case '>': new_off >>= get_offset(ptr); break;
		case '-': new_off -= get_offset(ptr); break;
		case '/': t = get_offset(ptr);
			if (t == 0) {
				printf("Division by zero?\n");
				break;
			} else new_off /= t; break;
		case '*': new_off *= get_offset(ptr); break;
		default : new_off  = get_offset(ptr); break;
		}
		if (tmp == NULL) break;
		ptr = tmp;
		oop = op;
	}
	free(txt);
#if RADARE_CORE
	free(txt2);
#endif
	level--;
//eprintf("CMP(%d)(%llx-%llx)\n", is_cmp,new_off,cmp_off);
	if (is_cmp>0)
		return (new_off - cmp_off);
	else
	if (is_cmp<0)
		return ((new_off - cmp_off)==0)?1:0;

	return new_off;
}

/* int byte = hexpair2bin("A0"); */
int get_cmp(const char *str0, const char *str1)
{
	u64 a,b;
	a = get_math(str0);
	b = get_math(str1);
	return (int)(a-b);
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

void getHTTPDate(char *DATE)
{
#if __UNIX__
	struct tm curt; /* current time */
	time_t l;
	char week_day[4], month[4];

	l=time(0);
	localtime_r(&l,&curt);

	DATE[0]=0;
	switch(curt.tm_wday)
	{
		case 0: strcpy(week_day, "Sun");
			break;
		case 1: strcpy(week_day, "Mon");
			break;
		case 2:	strcpy(week_day, "Tue");
			break;
		case 3:	strcpy(week_day, "Wed");
			break;
		case 4:	strcpy(week_day, "Thu");
			break;
		case 5:	strcpy(week_day, "Fri");
			break;
		case 6:	strcpy(week_day, "Sat");
			break;
		default: return;
	}
	
	switch(curt.tm_mon)
	{
		case 0: strcpy(month, "Jan");
			break;
		case 1: strcpy(month, "Feb");
			break;
		case 2: strcpy(month, "Mar");
			break;
		case 3: strcpy(month, "Apr");
			break;
		case 4: strcpy(month, "May");
			break;
		case 5: strcpy(month, "Jun");
			break;
		case 6: strcpy(month, "Jul");
			break;
		case 7: strcpy(month, "Aug");
			break;
		case 8: strcpy(month, "Sep");
			break;
		case 9: strcpy(month, "Oct");
			break;
		case 10: strcpy(month, "Nov");
			break;
		case 11: strcpy(month, "Dec");
			break;
		default: return;
	}
	
	sprintf(DATE, "%s, %02d %s %d %02d:%02d:%02d GMT", 
			week_day, curt.tm_mday, month, 
			curt.tm_year + 1900, curt.tm_hour, 
			curt.tm_min, curt.tm_sec);
#else
	DATE[0]='\0';

#endif
}


/* Returns 0 or 1 depending if the given character is safety printable */
int is_printable (int c)
{
	if (c<' '||c>'~') return 0;
	return 1;
}

/* int c; ret = hex2int(&c, 'c'); */
int hex2int (unsigned char *val, unsigned char c)
{
	if ('0' <= c && c <= '9')      *val = (unsigned char)(*val) * 16 + ( c - '0');
	else if (c >= 'A' && c <= 'F') *val = (unsigned char)(*val) * 16 + ( c - 'A' + 10);
	else if (c >= 'a' && c <= 'f') *val = (unsigned char)(*val) * 16 + ( c - 'a' + 10);
	else return 1;
	return 0;
}

/* int byte = hexpair2bin("A0"); */
int hexpair2bin(const char *arg) // (0A) => 10 || -1 (on error)
{
	unsigned char *ptr;
	unsigned char c = '\0';
	unsigned char d = '\0';
	unsigned int  j = 0;

	for (ptr = (unsigned char *)arg; ;ptr = ptr + 1) {
		if (ptr[0]=='\0'||ptr[0]==' ' || j==2)
			break;
		d = c;
		if (hex2int(&c, ptr[0])) {
			eprintf("Invalid hexa string at char '%c'.\n", ptr[0]);
			return -1;
		}
		c |= d;
		if (j++ == 0) c <<= 4;
	}

	return (int)c;
}

/* char buf[1024]; int len = hexstr2binstr("0a 33 45", buf); */
// XXX control out bytes
int hexstr2binstr(const char *in, unsigned char *out) // 0A 3B 4E A0
{
	const char *ptr;
	unsigned char  c = '\0';
	unsigned char  d = '\0';
	unsigned int len = 0, j = 0;

	for (ptr = in; ;ptr = ptr + 1) {
		/* ignored chars */
		if (ptr[0]==':' || ptr[0]=='\n' || ptr[0]=='\t' || ptr[0]=='\r' || ptr[0]==' ')
			continue;

		if (j==2) {
			if (j>0) {
				out[len] = c;
				len++;
				c = j = 0;
			}
			if (ptr[0]==' ')
				continue;
		}

		/* break after len++ */
		if (ptr[0] == '\0') break;

		d = c;
		if (ptr[0]=='0' && ptr[1]=='x' ){ //&& c==0) {
			u64 addr   = get_math(ptr);
			unsigned int addr32 = (u32) addr;
			if (addr & ~0xFFFFFFFF) {
				// 64 bit fun
			} else {
				// 32 bit fun
				u8 *addrp = (u8*) &addr32;
				// XXX always copy in native endian?
				out[len++] = addrp[0];
				out[len++] = addrp[1];
				out[len++] = addrp[2];
				out[len++] = addrp[3];
				while(ptr[0]&&ptr[0]!=' '&&ptr[0]!='\t')
					ptr = ptr + 1;
				j = 0;
			}
			continue;
		}
		if (hex2int(&c, ptr[0])) {
			eprintf("binstr: Invalid hexa string at %d ('0x%02x') (%s).\n", (int)(ptr-in), ptr[0], in);
			return 0;
		}
		c |= d;
		if (j++ == 0) c <<= 4;
	}

	return (int)len;
}

// f.ex: PK\x01\x02
int escape_buffer(char *buf)
{
	unsigned char ch = 0, ch2 = 0;
	int err = 0;
	int i;

	for(i=0;buf[i];i++) {
		if (buf[i]=='\\') {
			if (buf[i+1]=='x') {
				err = ch2 = ch = 0;
				if (!buf[i+2] || !buf[i+3]) {
					printf("Unexpected end of string.\n");
					return 0;
				}
				err |= hex2int(&ch,  buf[i+2]);
				err |= hex2int(&ch2, buf[i+3]);
				if (err) {
					printf("Incorrect hexadecimal characters for conversion.\n");
					return 0;
				}
				buf[i] = (ch<<4)+ch2;
				strcpy(buf+i+1, buf+i+4);
			} else {
				printf("'\\x' expected.\n");
				return 0;
			}
		}
	}

	return i;
}

int iswhitechar(char c)
{
	switch(c) {
	case ' ':
	case '\t':
	case '\n':
	case '\r':
		return 1;
	}
	return 0;
}

char *strclean(char *str)
{
	int len;
	char *ptr;

	if (str == NULL)
		return NULL;

	while(str[0]&&iswhitechar(str[0]))
		str = str + 1;

	len = strlen(str);

	if (len>0)
	for(ptr = str+len-1;ptr!=str;ptr = ptr - 1) {
		if (iswhitechar(ptr[0]))
			ptr[0]='\0';
		else
			break;
	}
	return str;
}

int strnull(const char *str)
{
	return (!str || !str[0]);
}

int strhash(const char *str)
{
	int i = 1;
	int a = 0x31;
	int b = 0x337;
	int h;
	for(h = str[0] ; str[i]; i++) {
		h+=str[i]*i*a;
		a*=b;
	}
	return h&0x7ffffff;
}

// FROM bash::stringlib
#define RESIZE_MALLOCED_BUFFER(str,cind,room,csize,sincr) \
	if ((cind) + (room) >= csize) { \
		while ((cind) + (room) >= csize) \
		csize += (sincr); \
		str = realloc (str, csize); \
	}

/* Replace occurrences of PAT with REP in STRING.  If GLOBAL is non-zero,
   replace all occurrences, otherwise replace only the first.
   This returns a new string; the caller should free it. */

char *strsub (char *string, char *pat, char *rep, int global)
{
	int patlen, templen, tempsize, repl, i;
	char *temp, *r;

	patlen = strlen (pat);
	for (temp = (char *)NULL, i = templen = tempsize = 0, repl = 1; string[i]; )
	{
		if (repl && !memcmp(string + i, pat, patlen)) {
			RESIZE_MALLOCED_BUFFER (temp, templen, patlen, tempsize, 4096); //UGLY HACK (patlen * 2));

			for (r = rep; *r; )
				temp[templen++] = *r++;

			i += patlen;
			repl = global != 0;
		} else {
			RESIZE_MALLOCED_BUFFER (temp, templen, 1, tempsize, 4096); // UGLY HACK 16);
			temp[templen++] = string[i++];
		}
	}
	temp[templen] = 0;
	return (temp);
}

char *str_first_word(const char *string)
{
	char *text  = (char *)string;
	char *start = NULL;
	char *ret   = NULL;
	int len     = 0;

	for(;*text &&iswhitespace(*text);text = text + 1);
	start = text;
	for(;*text &&!iswhitespace(*text);text = text + 1) len++;

	/* strdup */
	ret = (char *)malloc(len+1);
	if (ret == 0) {
		fprintf(stderr, "Cannot allocate %d bytes.\n", len+1);
		exit(1);
	}
	strncpy(ret, start, len);
	ret[len]='\0';

	return ret;
}

/* memccmp("foo.bar", "foo.cow, '.') == 0 */
int strccmp(char *dst, char *orig, int ch)
{
	int i;
	for(i=0;orig[i] && orig[i] != ch; i++)
		if (dst[i] != orig[i])
			return 1;
	return 0;
}

int strccpy(char *dst, char *orig, int ch)
{
	int i;
	for(i=0;orig[i] && orig[i] != ch; i++)
		dst[i] = orig[i];
	dst[i] = '\0';
	return i;
}

#define __htonq(x) (\
	(((x) & 0xff00000000000000LL) >> 56)  | \
	(((x) & 0x00ff000000000000LL) >> 40)  | \
	(((x) & 0x0000ff0000000000LL) >> 24)  | \
	(((x) & 0x000000ff00000000LL) >> 8)   | \
	(((x) & 0x00000000ff000000LL) << 8)   | \
	(((x) & 0x0000000000ff0000LL) << 24)  | \
	(((x) & 0x000000000000ff00LL) << 40)  | \
	(((x) & 0x00000000000000ffLL) << 56))

u64 htonq(u64 value) {
	u64 ret  = value;
#if LIL_ENDIAN
	endian_memcpy_e(&ret, &value, 8, 0);
#endif
	return ret;
}

const char *strget(const char *str)
{
	if (str == NULL)
		return nullstr_c;
	return str;
}
