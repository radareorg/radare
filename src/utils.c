/*
 * Copyright (C) 2006, 2007, 2008, 2009
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

int util_mkdir(const char *dir)
{
#if __WINDOWS__
	return mkdir(dir);
#else
	return mkdir(dir, 0755);
#endif
}

void eprintf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

void efree(void **ptr)
{
	free (*ptr);
	*ptr = NULL;
}

char *estrdup(char *ptr, const char *string)
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

ut64 file_size(const char *str)
{
	ut64 sz;
	FILE *fd = fopen(str, "r");
	if (fd == NULL)
		return 0;
	fseek(fd, 0,SEEK_END);
	sz = ftell(fd);
	fclose(fd);
	return sz;
}

char *slurp(const char *str, int *len)
{
	char *ret;
	long sz;
	FILE *fd = fopen(str, "r");
	if (fd == NULL)
		return NULL;
	fseek(fd, 0,SEEK_END);
	sz = ftell(fd);
	if (len) *len = sz;
	fseek(fd, 0,SEEK_SET);
	ret = (char *)malloc(sz+1);
	if (ret == NULL) {
		if (len) *len = 0;
		return NULL;
	}
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

int file_dump(const char *file, const u8 *buf, int len)
{
	FILE *fd;
	fd = fopen(file, "w");
	if (fd) {
		fwrite(buf, len, 1, fd);
		fclose(fd);
		return 1;
	}
	return 0;
}

void endian_memcpy_e(u8 *dest, const u8 *orig, int size, int endian)
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
			eprintf("Invalid size: %d\n", size);
		}
	}
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


void endian_memcpy(u8 *dest, const u8 *orig, unsigned int size)
{
#if RADARE_CORE
	endian_memcpy_e(dest, orig, size, PTRCAST (config_get("cfg.bigendian")));
#else
	endian_memcpy_e(dest, orig, size, 0); // lilendian by default
#endif
}

static char tmpdir[1024];
const char *get_tmp_dir()
{
	// TODO: Do not enter this function twice!
#if __WINDOWS__
	// http://msdn.microsoft.com/en-us/library/aa364992(VS.85).aspx
	GetTempPath(1023, tmpdir);
#else
	const char *tmp;
#if RADARE_CORE
	tmp = config_get("dir.tmp");
#else
	tmp = getenv("TMP");
#endif
	if (tmp) strcpy(tmpdir, tmp);
	else strcpy(tmpdir, "/tmp/");
#endif
	return (const char *)&tmpdir;
}

int r_file_exist(const char *str)
{
	struct stat buf;
	return (stat(str, &buf) == 0);

#if __WRONG_WAY__
	int fd = open(str, O_RDONLY);
	if (fd == -1)
		return 0;
	close(fd);
	return 1;
#endif
}

int str_cpy(char *dst, const char *org)
{
	int i = 0;
	if (org&&*org)
	do { *dst++=*org++;
	} while (*org && ++i);
	*dst=0;
	return i;
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

void progressbar(int nhit, ut64 addr, ut64 size)
{
#if RADARE_CORE
	int tmp, cols = config_get_i("scr.width")-20;
#else
	int tmp, cols = 58;
#endif
	int pc = (int)((100*addr)/size);

	(pc<0)?pc=0:(pc>100)?pc=100:0;
	fprintf(stderr, "\x1b[K 0x%08llx %2d%% [", addr, pc);
	cols-=15;
	for(tmp=cols*pc/100;tmp;tmp--) fprintf(stderr,"#");
	for(tmp=cols-(cols*pc/100);tmp;tmp--) fprintf(stderr,"-");
	fprintf(stderr, "] %d\r", nhit);
	fflush(stderr);
}

#if RADARE_CORE
ut64 get_pointer(ut64 addr, int size)
{
	int endian = !config_get_i("cfg.bigendian"); // XXX -?
	ut64 ret = 0;
	u8 newa8 = 0;
	ut16 newa16 = 0;
	ut32 newa32 = 0;
	ut64 newa64 = 0;
	ut64 sk = config.seek;

	radare_seek(addr, SEEK_SET);
	switch(size) {
	case 1:
		io_read(config.fd, &newa8, 1);
		ret = newa64 = newa8;
		break;
	case 2:
		io_read(config.fd, &newa16, 2);
		ret = newa64 = newa16;
		endian_memcpy_e((u8*)&ret, (u8*)&newa16,2,endian);
		break;
	case 4:
		io_read(config.fd, &newa32, 4);
		newa64 = newa32;
		endian_memcpy_e((u8*)&ret, (u8*)&newa32,4,endian);
		break;
	case 8:
		io_read(config.fd, &newa64, 8);
		endian_memcpy_e((u8*)&ret, (u8*)&newa64,8,endian);
		break;
	}
	radare_seek(sk, SEEK_SET);
	return ret;
}
#endif

/* Converts a string to ut64 type. ut64 jmp = get_offset("0x123456"); */
ut64 get_offset(const char *orig)
{
	char *arg;
	ut64 ret = 0;
	int i, j;
#if RADARE_CORE
	flag_t *flag;
	char *ptr = 0;
#endif
	if (orig==NULL||orig[0]=='\0')
		return 0;

	while(*orig==' '&&*orig) orig++;
	arg = alloca(strlen(orig)+32);
	str_cpy(arg, orig);

	/* single char 'A' */
	if (arg[0]=='\'' && arg[0+2]=='\'')
		return arg[0+1];

	for (;*arg==' ';arg=arg+1) {}
	for (i=0;arg[i]==' ';i++) {}
	for (;arg[i]=='\\';i++) {}
	i++;

#if RADARE_CORE
#if 0
/* DEPRECATED WITH $$ special variable */
	if (!strcmp(orig, "here"))
		return config.seek;
#endif

	if (arg[0]=='.') {
		ut64 real = 0;
		ut64 mask = 0;
		for(ptr = arg; ptr[0];ptr = ptr +1) {
			if (ptr[0]!='.') {
				mask<<= 4;
				mask |= 0xf;
				if (real==0)
					sscanf(ptr,"%llx", &real);
			}
		}
		mask = -mask -1;
		return (config.seek&mask)+ real;
	}

	ptr = strchr(arg, '[');
	if (ptr) return get_pointer(get_offset(ptr+1),4);

	ret = config_get_i(arg);
	if (((int)ret) != 0)
		return ret;

	if (arg[i]=='$') {
		struct aop_t aop;
		switch(arg[i+1]) {
		case 'w':
			ret = io_write_last;
			break;
		case '$':
			arch_aop (config.seek, config.block, &aop);
			ret = aop.length;
			break;
		case '?':
			ret = config.last_cmp;
			break;
		case 'e':
			arch_aop(config.seek, config.block, &aop);
			ret = aop.eob;
			break;
		case 'b':
			ret = config.block_size;
			break;
		case 'l':
			ret = undo_get_last_seek();
			break;
		case 'F':
			ret = data_prev(config.seek, DATA_FUN);
			break;
		case 'S':
			ret = data_prev_size(config.seek, DATA_FUN);
			break;
		case 'j':
			arch_aop(config.seek, config.block,&aop);
			ret = aop.jump;
			break;
		case 's':
			ret = config.size;
			break;
		case 'f':
			arch_aop(config.seek, config.block,&aop);
			ret = aop.fail;
			break;
		case 'r':
			arch_aop(config.seek, config.block, &aop);
			ret = aop.ref;
			break;
		case '{':
			ptr = strchr(arg+i+2, '}');
			if (ptr != NULL) {
				ptr[0]='\0';
				ret = config_get_i(arg+i+2);
				ptr[0]='}';
			}
			break;
		default:
			ret = config.seek;
		}
		return ret;
	} else {
		char *ch = strchr (arg, ' ');
		if (ch) *ch='\0';
		flag = flag_get(arg);
		if (flag)
			return flag->offset; // - config.vaddr;
	}
#endif
	if (arg[i] == 'x' && i>0 && arg[i-1]=='0') {
		sscanf(arg, "0x"OFF_FMTx, &ret);
	} else {
		sscanf(arg, OFF_FMTd, &ret);

		switch(arg[strlen(arg)-1]) {
		case 'f': // float
			{
			float f;
			sscanf(arg, "%f", &f);
			memcpy(&ret, &f, sizeof(f));
			}
			break;
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

ut32 get_offset32(const char *foo)
{
	return (ut32) get_offset (foo);
}

int set0word(char *str)
{
	int i;
	char *p, *q;
	if (str[0]=='\0')
		return 0;
	/* chop trailing spaces */
	while(*str==' ') str_cpy(str, str+1);
	for(i=strlen(str)-1;str[i]==' ';i--) str[i]='\0';
	/* count words */
	for(i=1,q=p=str;*p;p+=1) {
		if (*p!=' '&&(*q=='\0'||*q==' ')) { i++; }
		else if (*p==' ') { *p='\0'; }
		q = p;
	}
	return i;
}

const char *get0word(const char *str, int idx)
{
	int i;
	const char *ptr = str;
	if (ptr == NULL)
		return nullstr;
	for (i=0;*ptr && i != idx;i++)
		ptr = ptr + strlen(ptr) + 1;
	return ptr;
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
ut64 get_math(const char* text)
{
	ut64 t;
	ut64 new_off = 0;
	ut64 cmp_off = 0;
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
	for(tmp = txt = strdup(text); txt && txt[0] && txt[1]; txt = txt+1) {
#if 1
		if (txt[0] == '!' && txt[1]=='=')
			str_cpy(txt+1, txt+2);
		else
#endif
		if (txt[0] == '=' && txt[1]=='=')
			txt[0]=' ';
		else
		if (txt[0] == ' ')
			str_cpy(txt, txt+1);
	}
	txt = tmp;

	// TODO: use alloca here
	sign = 0;
#if RADARE_CORE
	txt2 = strdup(txt);
	if (*txt=='+') {
		new_off = config.seek;
	} else
	if (*txt=='-')
		new_off = config.seek;
#endif
		//sign = -1;
	//sign = (*txt=='+')?1:(*txt=='-')?-1:0;
	if (cmp_off == 0)
		cmp_off  = get_offset(ptr);

	for(ptr = txt; ptr && ptr[0]; ptr = ptr + strlen(ptr)+1)
	{
		/* XXX this only works when there're spaces like in 1 == 1, but 1==1 */
		tmp = mytok(ptr, "=!+-<>%*/[\\", &op);
		switch(oop) {
		case '!':
			if (is_cmp == 0) {
				is_cmp  = -1;
				cmp_off = new_off;
				if (tmp != NULL)
				new_off = get_offset(tmp+1);
			}
			break;
		case '=':
			if (is_cmp == 0) {
				is_cmp = 1;
				cmp_off = new_off;
				if (tmp != NULL)
				new_off = get_offset(tmp+1);
			}
			break;
#if RADARE_CORE
		case '[':
			end = strchr(txt2+(ptr-txt+1),']');
			// todo. support nested lol
			if (end) {
				int ptrsize = 4; // XXX use 8 for 64bit boxes ??
				char *p = strchr(txt2+(ptr-txt),':');

				if (p != NULL) {
					p[0] = '\0';
					ptrsize = atoi(txt2+(ptr-txt));
					p = p+1;
				} else
					p = txt2+(ptr-txt);

				end[0]='\0';
				new_off = get_pointer(get_math(p), ptrsize);
				end[0]=']';
				tmp = ptr + (end-txt2);
			} else {
				//eprintf("Unbalanced ']' (%s)\n", text);
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
		default : new_off += get_offset(ptr); break;
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
	ut64 a,b;
	a = get_math(str0);
	b = get_math(str1);
	return (int)(a-b);
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
	int len = 0;

	if (arg && (!arg[0] || !arg[1])) {
		eprintf("Too short\n");
		return -1;
	}

	for (ptr = (unsigned char *)arg; ;ptr = ptr + 1) {
		if (ptr[0]=='\0'||ptr[0]==' ' || j==2)
			break;
		d = c;
		if (hex2int(&c, ptr[0])) {
			eprintf("Invalid hexa string at char '%c'.\n", ptr[0]);
			return -1;
		}
		c |= d;
		if (j++ == 0)
			c <<= 4;
		len++;
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
			ut64 addr   = get_math(ptr);
			unsigned int addr32 = (ut32) addr;
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
			// eprintf("binstr: Invalid hexa string at %d ('0x%02x') (%s).\n", (int)(ptr-in), ptr[0], in);
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
			if (buf[i+1]=='e') {
				buf[i] = 0x1b;
				strcpy(buf+i+1, buf+i+2);
			} else
			if (buf[i+1]=='r') {
				buf[i] = 0x0d;
				strcpy(buf+i+1, buf+i+2);
			} else
			if (buf[i+1]=='n') {
				buf[i] = 0x0a;
				strcpy(buf+i+1, buf+i+2);
			} else
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

int strsub_memcmp (char *string, char *pat, int len)
{
	int res = 0;
	while(len--) {
		if (*pat!='?')
			res += *string - *pat;
		string = string+1;
		pat = pat+1;
	}
	return res;
}

char *strsub (char *string, char *pat, char *rep, int global)
{
	int patlen, templen, tempsize, repl, i;
	char *temp, *r;

	patlen = strlen (pat);
	for (temp = (char *)NULL, i = templen = tempsize = 0, repl = 1; string[i]; )
	{
//		if (repl && !memcmp(string + i, pat, patlen)) {
		if (repl && !strsub_memcmp(string + i, pat, patlen)) {
			RESIZE_MALLOCED_BUFFER (temp, templen, patlen, tempsize, 4096); //UGLY HACK (patlen * 2));
			if (temp == NULL)
				return NULL;
			for (r = rep; *r; )
				temp[templen++] = *r++;

			i += patlen;
			repl = global != 0;
		} else {
			RESIZE_MALLOCED_BUFFER (temp, templen, 1, tempsize, 4096); // UGLY HACK 16);
			temp[templen++] = string[i++];
		}
	}
	if (temp != NULL)
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

int str_ccpy(char *dst, char *orig, int ch)
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

ut64 htonq(ut64 value) {
	ut64 ret  = value;
#if LIL_ENDIAN
	endian_memcpy_e((u8*)&ret, (u8*)&value, 8, 0);
#endif
	return ret;
}

int memcpy_r(u8 *from, u8 *to, int len)
{
	u8 *p = from+len;
	u8 *d = to + len;
	while(p>from) {
		*d = *p;
		p = p - 1;
		d = d - 1;
	}
	return len;
}

const char *strget(const char *str)
{
	if (str == NULL)
		return nullstr_c;
	return str;
}

int str_grep(const char *str, const char *needle)
{
	int len = strlen(needle);
	int lenstr = strlen(str);
	if (*needle=='^') {
		if (len > lenstr)
			return 0;
		return !memcmp(str, needle+1, len-1);
	}
	if (needle[lenstr-1]=='*') {
		if (len > lenstr)
			return 0;
		return !memcmp(str, needle, lenstr);
	}
	if (needle[lenstr-1]=='$') {
		if (len > lenstr)
			return 0;
		return !memcmp(str+(lenstr-len+1), needle, len-1);
	}
	return (strstr(str, needle) != NULL)?1:0;
}

/* Profiling utilities */
static int timeval_subtract(struct timeval *result, struct timeval *end, struct timeval *begin)
{
	// Perform the carry for the later subtraction by updating Y
	if (end->tv_usec < begin->tv_usec) {
		int nsec = (begin->tv_usec - end->tv_usec) / 1000000 + 1;
		begin->tv_usec -= 1000000 * nsec;
		begin->tv_sec += nsec;
	}
	if (end->tv_usec - begin->tv_usec > 1000000) {
		int nsec = (end->tv_usec - begin->tv_usec) / 1000000;
		begin->tv_usec += 1000000 * nsec;
		begin->tv_sec -= nsec;
	}

	// Compute the time remaining to wait. 'tv_usec' is certainly positive.
	result->tv_sec = end->tv_sec - begin->tv_sec;
	result->tv_usec = end->tv_usec - begin->tv_usec;

	// Return 1 if result is negative
	return end->tv_sec < begin->tv_sec;
}

void r_prof_start(struct r_prof_t *p)
{
	struct timeval *begin = &p->begin;
	p->result = 0.0;
	gettimeofday(begin, NULL);
}

double r_prof_end(struct r_prof_t *p)
{
	struct timeval *begin = &p->begin;
	struct timeval end;
	struct timeval diff;
	int    sign;

	gettimeofday(&end, NULL);
	sign = timeval_subtract(&diff, begin, &end);
	p->result = R_ABS(((double)(diff.tv_sec) + ((double)diff.tv_usec / 1000000.)));
	return R_ABS(sign);
}

const char *resolve_path(const char *str)
{
	char buf[1024];
	char *path = getenv("PATH");
	char *pwd = getenv("PWD");
	char *foo;
	char *bar;
#if __WINDOWS__
	return strdup(str);
#else
	if (path != NULL && pwd != NULL) {
		if (*str == '.') {
			snprintf(buf, 1022, "%s/%s", pwd, str);
			char *ret = strdup(buf);
			return ret;
		} else if (*str != '/') {
			bar = alloca(strlen(str)+1);
			strcpy(bar, str);
			path = strdup(path);
			foo = path;
			while(bar) {
				bar = strchr(foo, ':');
				if (bar != NULL)
					bar[0]='\0';
				snprintf(buf, 1022, "%s/%s", foo, str);
				//if (strstr(buf, ".exe") || strstr(buf, ".com")) {
				if (!access(buf, X_OK)) {
					char *ret = strdup(buf);
					free (path);
					return ret;
				}
				foo = bar + 1;
			}
			free (path);
		}
	}
	return strdup(str);
#endif
}

char *r_sys_cmd_str(const char *cmd, const char *input, int *len)
{
#if __UNIX__
	char *b = malloc(1024);
	FILE *fd = popen(cmd, "r");
	*len = 0;
	if (fd == NULL)
		return NULL;
	*len = fread(b, 1, 1024, fd);
	pclose(fd);
	return b;
#else
#warning NO r_sys_cmd_str support for this platform
	return NULL;
#endif
}

static const char *home = NULL;
const char *get_home_directory()
{
	if (home == NULL) {
#if __WIN32__
		home = getenv("HOMEPATH");
#else
		home = getenv("HOME");
#endif
	}
	return strdup(strget(home));
}
