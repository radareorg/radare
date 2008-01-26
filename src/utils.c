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

#if __UNIX__
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/socket.h>
#endif

const char hex[16] = "0123456789ABCDEF";

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

void endian_memcpy(unsigned char *dest, unsigned char *orig, unsigned int size)
{
#if RADARE_CORE
#if LIL_ENDIAN /* little endian : x86 */
	int endian = !config.endian;
#else /* big endian */
	int endian = config.endian;
#endif
	endian_memcpy_e(dest, orig, size, endian);
#else
	endian_memcpy_e(dest, orig, size, 0); // lilendian by default
#endif
}

void endian_memcpy_e(unsigned char *dest, unsigned char *orig, unsigned int size, int endian)
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

int make_tmp_file(char *str)
{
	int fd;
#if RADARE_CORE
	const char *tmp = config_get("dir.tmp");
#else
	char *tmp = getenv("TMP");
#endif
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
        char *columns = getenv("COLUMNS");
        int tmp, cols = 80;

        (pc<0)?pc=0:(pc>100)?pc=100:0;
        printf("\e[K  %3d%% [", pc);
        if (columns)
                cols = atoi(columns);
        cols-=15;
        for(tmp=cols*pc/100;tmp;tmp--) printf("#");
        for(tmp=cols-(cols*pc/100);tmp;tmp--) printf("-");
        printf("]\r");
	fflush(stdout);
}

#if RADARE_CORE
// TODO get [ prefix: (size of ptr), use cfg.endian
//  4[0x300] = dword[0x300]
//  d[0x300] ...
//  b[0x300]
// eval cfg.endian
//unsigned char *ptr = &newa;
unsigned long get_pointer(off_t addr)
{
	unsigned long newa;
	off_t sk = config.seek;
	radare_seek(addr, SEEK_SET);
	io_read(config.fd, &newa, 4);
	radare_seek(sk, SEEK_SET);
	return newa;
}
#endif

/* Converts a string to off_t type. off_t jmp = get_offset("0x123456"); */
off_t get_offset(char *orig)
{
	char arga[1024];
	char *arg = (char *)&arga;
	off_t ret = 0;
	int i, j;
#ifdef RADARE_CORE
	rad_flag_t *flag;
	char *ptr = 0;
#endif
	if (orig==NULL||orig[0]=='\0')
		return 0;

	strncpy(arg, orig, 1023);

	for(;*arg==' ';arg=arg+1);
	for(i=0;arg[i]==' ';i++);
	for(;arg[i]=='\\';i++); i++;

#ifdef RADARE_CORE
	ptr = strchr(arg, '[');
	if (ptr)
		return get_pointer(get_offset(ptr+1));

	ret = config_get_i(arg);
	if (((int)ret) != 0)
		return ret;

	flag = flag_get(arg);
	if (flag)
		return flag->offset; // - config.baddr;
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

off_t get_math(const char* text)
{
	off_t t, new_off = 0;
	int  sign     = 1;
	char op       = 0;
	char oop      = 0;
	char *txt, *txt2, *tmp;
	char *ptr     = NULL;
	char *end;

	if (text==NULL||text[0]=='\0')
		return 0;

#if RADARE_CORE
	txt2=strdup(text);
#endif
	for(txt = strdup(text); txt[0]==' ' && txt[0]; strcpy(txt, txt+1));
	sign = (*txt=='+')?1:(*txt=='-')?-1:0;

	for(ptr = txt; ptr && ptr[0]; ptr = ptr + strlen(ptr)+1)
	{
		tmp = mytok(ptr, "+-*/[", &op);
		switch(oop) {
#ifdef RADARE_CORE
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
		oop=op;
	}
	free(txt);
#if RADARE_CORE
	free(txt2);
#endif

	return new_off;
}

/* int byte = hexpair2bin("A0"); */
int get_cmp(const char *str0, const char *str1)
{
	off_t a,b;
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
int hexstr2binstr(const char *in, unsigned char *out) // 0A 3B 4E A0
{
	const char *ptr;
	unsigned char  c = '\0';
	unsigned char  d = '\0';
	unsigned int len = 0, j = 0;

	for (ptr = in; ;ptr = ptr + 1) {
		if (ptr[0]==':' || ptr[0]==0x52 || ptr[0]=='\n' || ptr[0]=='\t' || ptr[0]=='\r' || ptr[0]== ' ')
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

		if (ptr[0] == '\0') break;

		d = c;
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

int strhash(char *str)
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

#if __WINDOWS__
/* code ripped from OpenBSD */
#if defined(LIBC_SCCS) && !defined(lint)
/* static char sccsid[] = "from: @(#)getopt.c	8.2 (Berkeley) 4/2/94"; */
static char *rcsid = "$Id: getopt.c,v 1.2 1998/01/21 22:27:05 billm Exp $";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int	opterr = 1,		/* if error message should be printed */
	optind = 1,		/* index into parent argv vector */
	optopt,			/* character checked for validity */
	optreset;		/* reset getopt */
char	*optarg;		/* argument associated with option */

#define	BADCH	(int)'?'
#define	BADARG	(int)':'
#define	EMSG	""

/*
 * getopt --
 *	Parse argc/argv argument vector.
 */
int
getopt(nargc, nargv, ostr)
	int nargc;
	char * const *nargv;
	const char *ostr;
{
	static char *place = EMSG;		/* option letter processing */
	char *oli;				/* option letter list index */

	if (optreset || !*place) {		/* update scanning pointer */
		optreset = 0;
		if (optind >= nargc || *(place = nargv[optind]) != '-') {
			place = EMSG;
			return (-1);
		}
		if (place[1] && *++place == '-') {	/* found "--" */
			++optind;
			place = EMSG;
			return (-1);
		}
	}					/* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' ||
	    !(oli = strchr(ostr, optopt))) {
		/*
		 * if the user didn't specify '-' as an option,
		 * assume it means -1.
		 */
		if (optopt == (int)'-')
			return (-1);
		if (!*place)
			++optind;
		if (opterr && *ostr != ':')
			(void)fprintf(stderr,
			    "%s: illegal option -- %c\n", nargv[0], optopt);
		return (BADCH);
	}
	if (*++oli != ':') {			/* don't need argument */
		optarg = NULL;
		if (!*place)
			++optind;
	}
	else {					/* need an argument */
		if (*place)			/* no white space */
			optarg = place;
		else if (nargc <= ++optind) {	/* no arg */
			place = EMSG;
			if (*ostr == ':')
				return (BADARG);
			if (opterr)
				(void)fprintf(stderr,
				    "%s: option requires an argument -- %c\n",
				    nargv[0], optopt);
			return (BADCH);
		}
	 	else				/* white space */
			optarg = nargv[optind];
		place = EMSG;
		++optind;
	}
	return (optopt);			/* dump back option letter */
}
#endif
