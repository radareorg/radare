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

int _print_fd = 1;

int pprint_fd(int fd)
{
	if (_print_fd == 0)
		return fd;
	return _print_fd = fd;
}

static int pprintf_buffer_len = 0;
static char *pprintf_buffer = NULL;

// TODO : rename pprintf to console or so
char *pprintf_get()
{
	return pprintf_buffer;
}

void palloc(int moar)
{
	if (pprintf_buffer == NULL) {
		pprintf_buffer_len = moar+1024;
		pprintf_buffer = (char *)malloc(pprintf_buffer_len);
		pprintf_buffer[0]='\0';
	} else
	if (moar + strlen(pprintf_buffer)>pprintf_buffer_len) {
		pprintf_buffer = (char *)realloc(pprintf_buffer, pprintf_buffer_len+moar+strlen(pprintf_buffer)+1);
	}
}


void pprintf_flush()
{
	if (pprintf_buffer && pprintf_buffer[0]) {
		write(_print_fd, pprintf_buffer, strlen(pprintf_buffer));
		pprintf_buffer[0] = '\0';
	}
}

void pprintf_newline()
{
	pprintf("\n");
#if RADARE_CORE
	if (!config.buf)
		pprintf_flush();
#endif
}

// XXX cannot print longer than 4K
// no buffering
void pprintf(const char *format, ...)
{
	char buf[4096];
	va_list ap;

	va_start(ap, format);

	buf[0]='\0';
	if (vsnprintf(buf, 4095, format, ap)<0)
		buf[0]='\0';

	palloc(strlen(buf)+1000);
	strcat(pprintf_buffer, buf);
//eprintf("STR(%s)\n", buf);

#if 0
#if RADARE_CORE
	if (!config.buf)
		pprintf_flush();
#endif
#endif

	va_end(ap);
}

void pstrcat(const char *str)
{
	palloc(strlen(str));
	strcat(pprintf_buffer, str);
}

void eprintf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
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

int terminal_get_columns()
{
	int columns_i = terminal_get_real_columns();
	char buf[64];

	sprintf(buf, "%d", columns_i);
	setenv("COLUMNS", buf, 0);

	return columns_i;
}

int terminal_get_real_columns()
{
#if __UNIX__
        struct winsize win;

        if (ioctl(1, TIOCGWINSZ, &win)) {
		/* default values */
		win.ws_col = 80;
		win.ws_row = 23;
	}
#ifdef RADARE_CORE
	config.height = win.ws_row;
#endif

        return win.ws_col;
#else
	return 80;
#endif
}

#ifdef RADARE_CORE
int yesno(int def, const char *fmt, ...)
{
	va_list ap;
	int key = def;

	if (config.visual)
		key='y';
	else D {
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
		fflush(stderr);
		terminal_set_raw(1);
		read(0, &key, 1); write(2, "\n", 1);
		terminal_set_raw(0);
	} else
		key = 'y';

	return key=='y';
}
#endif

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

/**
 *
 * void terminal_set_raw( [0,1] )
 *
 *   Change canonicality of the terminal
 *
 * For optimization reasons, there's no initialization flag, so you need to
 * ensure that the make the first call to terminal_set_raw() with '1' and
 * the next calls ^=1, so: 1, 0, 1, 0, 1, ...
 *
 * If you doesn't use this order you'll probably loss your terminal properties.
 *
 */
#if __UNIX__
static struct termios tio_old, tio_new;
#endif
static int termios_init = 0;

void terminal_set_raw(int b)
{
#if __UNIX__
	if (b) {
		if (termios_init == 0) {
			tcgetattr(0, &tio_old);
			memcpy (&tio_new,&tio_old,sizeof(struct termios));
			tio_new.c_iflag &= ~(BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
			tio_new.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
			tio_new.c_cflag &= ~(CSIZE|PARENB);
			tio_new.c_cflag |= CS8;
			tio_new.c_cc[VMIN]=1; // Solaris stuff hehe
			termios_init = 1;
		}
		tcsetattr(0, TCSANOW, &tio_new);
	} else
		tcsetattr(0, TCSANOW, &tio_old);
#else
	/* TODO : W32 */
#endif
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

