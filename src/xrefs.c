/*
 * Copyright (C) 2007
 *       pancake <pancake@youterm.com>
 *
 * 'xrefs' is part of the radare project.
 *
 * xrefs is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xrefs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xrefs; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/*
=====================================================================

 xrefs - find relative inverse references to an offset inside a file.

 author:  pancake <pancake@youterm.com>
 date:    2006-12-30
 context: external radare utility

=====================================================================

 usage example:

~:$ cat example.c  
#include <stdio.h>

void func() {
        printf("Hello ");
}

int main() {
        func(); func(); func();
}

~:$ gcc example.c 

~:$ ./a.out 
Hello Hello Hello

~:$ objdump -d a.out | grep func
10000400 <func>:
1000044c:       4b ff ff b5     bl      10000400 <func>
10000450:       4b ff ff b1     bl      10000400 <func>
10000454:       4b ff ff ad     bl      10000400 <func>

~:$ ./xrefs -a ppc a.out 0x400
match value ffffffb5 (ffffb5) at offset 0x44c
match value ffffffb1 (ffffb1) at offset 0x450
match value ffffffad (ffffad) at offset 0x454

*/

/**
========================================================================

 XXX and TODO:
 -------------

 - 64 bit offsets support
 - set opcode byte (bl == 0x4b, ...)
 -- set and find the jump/call byte before the offset address

========================================================================
**/

/* work in 32 bit mode...no need for 64 yet..
 * and it looks problematic */
#define _FILE_OFFSET_BITS 32
#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <getopt.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

/* XXX: ufly hack : override radare version number */
#if defined(VERSION)
#undef VERSION
#endif
#define VERSION "0.2.1"

#if 0
#if SIZEOF_OFF_T == 8
#define OFF_FMT "%016llX"
#define OFF_FMTx "%llx"
#define OFF_FMTd "%lld"
#else 
#define OFF_FMT "%08X"
#define OFF_FMTx "%x"
#define OFF_FMTd "%d"
#endif
#else
/* we're in 32 bits mode (FORCED BEFORE) */
#define OFF_FMT "%08x"
#define OFF_FMTx "%x"
#define OFF_FMTd "%d"
#define offtd int
#define offtx unsigned int
#endif

typedef enum {
	ARCH_NULL,
	ARCH_ARM,
	ARCH_PPC,
	ARCH_X86
} arch_t;

off_t delta   = 0;
off_t range   = 0;
off_t xylum   = 0;
off_t gamme   = 0;
off_t size    = 4;
int sysendian = 0;    // initialized in main
int endian    = -1; // little endian by default
int verbose   = 0;
int found     = 0;
int quite     = 0;
arch_t arch   = ARCH_NULL;
unsigned char *ma;

static int show_usage()
{
	printf(
	"Usage: xrefs [-options] [file] [offset]\n"
	" -v             Verbose mode\n"
	" -V             Show version\n"
	" -q             quite mode\n"
	" -h             Show this helpy message\n"
	" -e             Use big endian for offsets to search\n"
	" -a [arch]      Architecture profile (fmi: help) (autodetects ELF and PE hdrs)\n"
	" -f [from]      start scanning from this offset (default 0)\n"
	" -t [to]        limit address (default 99999999)\n"
	" -r [range]     Range in bytes of allowed relative offsets\n"
	" -s [size]      Size of offset (4 for intel, 3 for ppc, ...)\n"
	" -d [offset]    Sets a negative delta offset as padding (default 1)\n"
	" -X [offset]    Print out debugging information of a certain relative offset\n");

	return 1;
}

static off_t file_size_fd(int fd)
{
	off_t curr = lseek(fd, 0, SEEK_CUR);
	off_t size = lseek(fd, 0, SEEK_END); // XXX: this is not size, is rest!
	lseek(fd, curr, SEEK_SET);

	return size;
}

/* TODO: move+share in offset.c ? */
static off_t get_offset(char *arg)
{
        int i;
        off_t ret;

	for(i=0;arg[i]==' ';i++);
	for(;arg[i]=='\\';i++); i++;

        if (arg[i] == 'x')
                sscanf(arg, OFF_FMTx, (offtx *)&ret);
        else
                sscanf(arg, OFF_FMTd, (offtd *)&ret);

        return ret;
}

int get_system_endian()
{
	int   a = 1;
	char *b = (char*)&a;
	return (int)(b[0]);
}

int set_arch_settings()
{
	switch(arch) {
	case ARCH_PPC:
		gamme  = 1;
		delta  = 1;
		size   = 3;
		break;
	case ARCH_ARM:
		gamme  = -1;
		delta  = 1;
		size   = 3;
		break;
	case ARCH_X86:
		gamme  = 1;
		delta  = 0; //-5;
		size   = 4;
		break;
	case ARCH_NULL:
		/* autodetect architecture */
		// ELF
		if (!memcmp(ma, "\x7f\x45\x4c\x46", 4)) {
			short ar = (ma[0x12]<<8) + ma[0x13];
			switch(ar) {
			case 0x0300:
				if (endian==-1)
					endian = 1;
				if (!quite)
					printf("# -a x86\n");
				arch = ARCH_X86;
				endian = 1;
				return 1;
			case 0x0014:
				if (endian==-1)
					endian = 0;
				if (!quite)
				printf("# -a ppc\n");
				arch = ARCH_PPC;
				return 1;
			case 0x2800:
				if (endian==-1)
					endian = 1;
				if (!quite)
				printf("# -a arm\n");
				arch = ARCH_ARM;
				return 1;
			default:
				printf("Unsupported architecture '%04x'.\n", ar);
				exit(1);
			}
		} else
		// MZ
		if (!memcmp(ma, "\x4d\x5a",2)) {
			unsigned short off = ma[0x3c];
			if (!memcmp(ma+off, "PE\x00\x00",4)) {
				unsigned short ar = (ma[off+4]<<8)+ma[off+5];
				switch(ar) {
				case 0x4c01: // x86
					if (endian==-1)
						endian = 1;
					printf("# -a x86\n");
					arch = ARCH_X86;
					endian = 1;
					return 1;
				case 0xc001: // arm
					if (endian==-1)
						endian = 1;
					printf("# -a arm\n");
					arch = ARCH_ARM;
					endian = 1;
					return 1;
				default:
					fprintf(stderr, "Unknown architecture.\n");
					break;
				}
			}
		} else {
			fprintf(stderr, "Plz. gimmie an architecture.\n");
			exit(1);
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	int i, c, src;
	off_t offset = 0;
	off_t from   = 1,
	      to     = INT_MAX;
	off_t sa;

	if (argc==2)
	if (!strcmp(argv[1],"-V")) {
		printf("%s\n", VERSION);
		return 0;
	}

	if (argc<3)
		return show_usage();

	/* parse arguments */
	while ((c = getopt(argc, argv, "qa:d:hves:f:t:r:X:")) != -1)
	{
		switch( c )
		{
		case 'q':
			quite = 1;
			break;
		case 'a':
			if (!strcmp(optarg, "x86"))
				arch = ARCH_X86;
			else
			if (!strcmp(optarg, "arm"))
				arch = ARCH_ARM;
			else
			if (!strcmp(optarg, "ppc")) {
				arch = ARCH_PPC;
			} else {
				printf("arm ppc x86\n");
				return 1;
			}
			break;
		case 'd':
			delta = get_offset(optarg);
			break;
		case 'X':
			xylum = get_offset(optarg);
			break;
		case 'e':
			endian = 1;
			break;
		case 'r':
			range = get_offset(optarg);
			if (range<0) range = -range;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'f':
			from = get_offset(optarg);
			break;
		case 't':
			to = get_offset(optarg);
			break;
		case 's':
			size = get_offset(optarg);
			break;
		case 'h':
			return show_usage();
		}
	}

	if (optind+2 != argc) {
		fprintf(stderr, "Plz. gimmie a file and offset.\n");
		return 1;
	}

	/* openning stuff */
	src    = open(argv[optind], O_RDONLY);
	offset = get_offset(argv[optind+1]);

	sa = file_size_fd(src) - size;
	ma = mmap(NULL, sa, PROT_READ, MAP_SHARED, src, 0);
	if (sa < 0x50) {
		fprintf(stderr, "Minimum length is 0x50 bytes.\n");
		return 1;
	}

	/* configure environment */
	sysendian = get_system_endian();

	while( set_arch_settings() );

	if (endian == -1)
		endian = 0;

	/* loopize looking for xrefs */
	for(i=from; i<sa && i<to; i++) {
		off_t value = offset - i + delta;
		off_t ovalue = value;
		off_t tmpvalue = 0;
		unsigned char *buf = (unsigned char *)&value;

		if (range!=0) {
			if (value<0 && -value>range)
					continue;
			else
			if (value>0 && value>range)
					continue;
		}

		if (verbose)
			printf("0x%08x  try %02x %02x %02x %02x (0x"
				OFF_FMTx") - "OFF_FMTd"\n",
				i, buf[0], buf[1], buf[2], buf[3], (offtx) value, (offtd)value);

		if (xylum && i == xylum) {
			printf("# offset: 0x"OFF_FMTx"\n", i);
			printf("# delta: "OFF_FMTd"\n", (offtd)delta);
			printf("# size:  "OFF_FMTd"\n", (offtd)size);
			printf("# value:  "OFF_FMTd"\n", (offtd)value);
			printf("# bytes:  %02x %02x %02x %02x (0x"OFF_FMTx") - "OFF_FMTd"\n",
				buf[0], buf[1], buf[2], buf[3], (offtx)value, (offtd)value);
			tmpvalue = ma[i+gamme];
			printf("# found:  %02x %02x %02x %02x\n",
				ma[i+gamme+0], ma[i+gamme+1],
				ma[i+gamme+2], ma[i+gamme+3]);
		}

		switch(arch) {
		case ARCH_ARM:
			value = (value-8)/4;
			break;
		case ARCH_X86:
			value-=5;
			break;
		default:
			break;
		}

		// force little endian //
		if (sysendian) {
			unsigned char tmp;
			tmp = buf[0]; buf[0]= buf[3]; buf[3] = tmp;
			tmp = buf[1]; buf[1]= buf[2]; buf[2] = tmp;
		}
		// target architecture endian //
		if (endian) {
			unsigned char tmp;
			tmp = buf[0]; buf[0] = buf[3]; buf[3] = tmp;
			tmp = buf[1]; buf[1] = buf[2]; buf[2] = tmp;
		}
		if (arch==ARCH_ARM) {
			buf[3] = buf[2]; buf[2] = buf[1]; buf[1] = buf[0];
		}

		if (xylum && ovalue == xylum) {
			printf("# buf:  %02x %02x %02x %02x (+"OFF_FMTd")\n",
				buf[0], buf[1], buf[2], buf[3], (offtd)(4-size));
			printf("# map:  %02x %02x %02x \n",
				ma[i+gamme], ma[i+1+gamme], ma[i+2+gamme]);
			printf("# cmp:  %02x %02x %02x\n", ma[i], ma[i+1], ma[i+2]);
		}

		if (xylum && i == xylum) {
			printf("# a:  %02x %02x %02x %02x\n",
				ma[i+gamme+0], ma[i+gamme+1],
				ma[i+gamme+2], ma[i+gamme+3]);
			printf("# b:  %02x %02x %02x %02x\n",
				buf[0], buf[1], buf[2], buf[3]);
		}

		if (memcmp((unsigned char *)ma+i+gamme, (unsigned char *)buf+(4-size), size) == 0) {
			if (quite)
				printf("0x"OFF_FMTx"\n", (offtx)i);
			else
				printf("match value 0x"OFF_FMTx" (%02x%02x%02x) at offset 0x"OFF_FMTx"\n",
					(offtx)ovalue,
					buf[0+(4-size)], buf[1+(4-size)], buf[2+(4-size)],
					(offtx)((off_t)i)+((gamme<0)?-1:0));
			found++;
		}
	}

	if (found == 0 && !quite)
		puts("no matches found.");

	return 0;
}
