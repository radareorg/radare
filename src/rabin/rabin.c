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

#include "../main.h"
#include <stdio.h>
#if __UNIX__
#include <fcntl.h>
#include <sys/mman.h>
#include <dlfcn.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

/* var */

enum {
	FILETYPE_UNK = 0,
	FILETYPE_ELF,
	FILETYPE_MZ,
	FILETYPE_PE,
	FILETYPE_CLASS,
	FILETYPE_DEX,
	FILETYPE_MACHO
};

#define ACTION_UNK      0x0000
#define ACTION_ENTRY    0x0001 
#define ACTION_IMPORTS  0x0002 
#define ACTION_SYMBOLS  0x0004 
#define ACTION_LIBS     0x0008 
#define ACTION_EXPORTS  0x0010 
#define ACTION_SECTIONS 0x0020 
#define ACTION_CHECKSUM 0x0040 
#define ACTION_BASE     0x0080
#define ACTION_ARCH     0x0100
#define ACTION_FILETYPE 0x0200
#define ACTION_NOP      0x1000

// TODO : move into rabin_t
char *file = NULL;
int filetype = FILETYPE_UNK;
int action   = ACTION_UNK;
int verbose  = 0;
int fd       = -1;
static int pebase = 0;

/* fun */

int rabin_show_help()
{
	printf(
"rabin [-erlis] [bin-file]\n"
" -e        shows entrypoints one per line\n"
" -i        imports (symbols imported from libraries)\n"
" -c        checksum\n"
" -E        exports (symbols exported)\n"
" -s        all program symbols\n"
" -t        type of binary\n"
" -S        show sections\n"
" -l        linked libraries\n"
" -L [lib]  dlopen library and show address\n"
" -r        flag makes show the output in radare format\n"
" -v        be verbose\n");
	return 1;
}

void rabin_show_checksum()
{
	unsigned char buf[32];
	unsigned long addr = 0;
	int i;

	switch(filetype) {
	case FILETYPE_DEX:
		lseek(fd, 8, SEEK_SET);
		read(fd, &addr, 4);
		printf("Checksum: 0x%08lx\n", addr);
		read(fd, &buf, 20);
		printf("SHA-1 Signature: ");
		for(i=0;i<20;i++)
			printf("%08x ", buf[i]);
		break;
	case FILETYPE_ELF:
		break;
	case FILETYPE_MZ:
		break;
	case FILETYPE_PE:
		lseek(fd, pebase+0x18, SEEK_SET);
		read(fd, &addr, 4);
		printf("0x%x checksum file offset\n", pebase+0x18);
		printf("0x%04x checksum\n",
			(unsigned int)
			(unsigned short)addr);
		break;
	}
}

void rabin_show_entrypoint()
{
	unsigned long addr = 0;
	unsigned long base = 0;
	switch(filetype) {
	case FILETYPE_ELF:
		lseek(fd, 0x18, SEEK_SET);
		read(fd, &addr, 4);
		printf("0x%08lx memory\n", addr);
		printf("0x%08lx disk\n", addr - 0x8048000);
		break;
	case FILETYPE_MZ:
		break;
	case FILETYPE_PE:
		lseek(fd, pebase+0x28, SEEK_SET);
		read(fd, &addr, 4);
		printf("0x%08x disk offset for ep\n", pebase+0x28);
		printf("0x%08lx disk\n", addr-0xc00);

		lseek(fd, pebase+0x45, SEEK_SET);
		read(fd, &base, 4);
		printf("0x%08lx memory\n", base+addr);
		break;
	}
}

unsigned long addr_for_lib(char *name)
{
#if __UNIX__
	unsigned long *addr = dlopen(name, RTLD_LAZY);
	if (addr) {
		dlclose(addr);
		return (unsigned long)((addr!=NULL)?(*addr):0);
	} else {
		printf("cannot open '%s' library\n", name);
		return 0;
	}
#endif
	return 0;
}

void rabin_show_symbols()
{
	unsigned long addr, addr2, addr3;
	unsigned int num, i;
	char buf[1024];

	switch(filetype) {
	case FILETYPE_ELF:
		sprintf(buf, "objdump -d '%s' | grep '>:' | sed -e 's,<,,g' -e 's,>:,,g' -e 's,^,0x,' | sort | uniq", file);
		system(buf);
		break;
	case FILETYPE_DEX:
// METHODS AND CODE OFFSETS
		lseek(fd, 0x4c, SEEK_SET); // num of methods
		read(fd, &num, 4);
		lseek(fd, 0x50, SEEK_SET); // num of methods
		read(fd, &addr, 4);
		lseek(fd, addr, SEEK_SET);
		for(i=0;i<num;i++) {
			lseek(fd, (addr+i*32)+0xc, SEEK_SET);
			read(fd, &addr2, 4);
			lseek(fd, (addr2+0xc), 4);
			read(fd, &addr3, 4);
			printf("0x%08lx\n", addr3);
		}
// CLASSES
#if 0
		lseek(fd, 0x40, SEEK_SET); // class list
		read(fd, &addr, 4);
		printf("Offset of class list: %d\n", addr);
		lseek(fd, 0x3c, SEEK_SET); // number of classes
		read(fd, &num, 4);
		printf("Number of classes: %d\n", num);
		lseek(fd, addr, SEEK_SET);
		for(i=0;i<num;i++) {
			read(fd, &idx, 4);
			printf(" class name: (string-index %d)\n", idx);
		}

		/* class definition */
		lseek(fd, 0x58, SEEK_SET); // class definition tables
		read(fd, &addr, 4);
		printf("Offset of class definition table: %d\n", addr);
		lseek(fd, 0x54, SEEK_SET); // number of class definition tables
		read(fd, &num, 4);
		printf("Number of classes: %d\n", num);
		lseek(fd, addr, SEEK_SET);
		// TODO: needs to be parsed completely
#endif
		
		break;
	}
}

void rabin_show_arch()
{
	u32 dw;
	u16 w;

	switch(filetype) {
	case FILETYPE_ELF:
		lseek(fd, 16+2, SEEK_SET);
		read(fd, &w, 2);
		switch(w) {
		case 3:
			printf("arch: x86-32\n");
			break;
		case 0x28:
			printf("arch: ARM\n");
			break;
		default:
			printf("arch: 0x%x (unknown)\n", w);
			break;
		}
		break;
	case FILETYPE_PE:
		// [[0x3c]+4]
		lseek(fd, 0x3c, SEEK_SET);
		read(fd, &dw, 4);
		lseek(fd, dw+4, SEEK_SET);
		read(fd, &w, 2);
		switch(w) {
		case 0x1c0:
			printf("arch: ARM\n");
			break;
		case 0x14c:
			printf("arch: x86-32\n");
			break;
		default:
			printf("arch: 0x%x (unknown)\n", w);
		}
		break;
	}
}

void rabin_show_filetype()
{
	switch(filetype) {
	case FILETYPE_ELF:
		printf("ELF\n");
		break;
	case FILETYPE_PE:
		printf("PE\n");
		break;
	case FILETYPE_MZ:
		printf("DOS COM\n");
		break;
	case FILETYPE_CLASS:
		printf("Java class\n");
		break;
	case FILETYPE_DEX:
		printf("DEX (google android)\n");
		break;
	case FILETYPE_MACHO:
		printf("mach-o\n");
		break;
	}
}

void rabin_show_imports()
{
	char buf[1024];

	switch(filetype) {
	case FILETYPE_ELF:
		//sprintf(buf, "readelf -sA '%s'|grep GLOBAL | awk ' {print $8}'", file);
		sprintf(buf, "readelf -s '%s' | grep FUNC | grep GLOBAL | grep DEFAULT  | grep ' UND ' | awk '{ print \"0x\"$2\" \"$8 }' | sort | uniq" , file);
		system(buf);
		break;
	}
}

void rabin_show_exports(char *file)
{
	char buf[1024];

	switch(filetype) {
	case FILETYPE_ELF:
		sprintf(buf, "readelf -s '%s' | grep FUNC | grep GLOBAL | grep DEFAULT  | grep ' 12 ' | awk '{ print \"0x\"$2\" \"$8 }' | sort | uniq" , file);
		system(buf);
		break;
	}
}

void rabin_show_sections()
{
	char buf[1024];

	sprintf(buf, "readelf -S '%s'|grep '\\[' | grep -v '\\[Nr\\]' | cut -c 4- | awk '{ print \"0x\"$4\" \"$2 }'", file);
	system(buf);
}

void rabin_show_libs()
{
	char buf[1024];

	switch(filetype) {
	case FILETYPE_MACHO:
		sprintf(buf, "otool -L '%s'", file);
		system(buf);
		break;
	default:
		sprintf(buf, "strings '%s' | grep -e '^lib'", file);
		system(buf);
		break;
	}
}

int rabin_identify_header()
{
	unsigned char buf[1024];

	lseek(fd, 0, SEEK_SET);
	read(fd, buf, 1024);

        if (!memcmp(buf, "\xCA\xFE\xBA\xBE", 4))
		if (buf[9])
                	filetype = FILETYPE_CLASS;
		else	filetype = FILETYPE_MACHO;
	else
	if (!memcmp(buf, "dex\n009\0", 8))
		filetype = FILETYPE_DEX;
	else
	if (!memcmp(buf, "\x7F\x45\x4c\x46", 4))
		filetype = FILETYPE_ELF;
	else
	if (!memcmp(buf, "\x4d\x5a", 2)) {
		int pe = buf[0x3c];
		filetype = FILETYPE_MZ;
		if (buf[pe]=='P' && buf[pe+1]=='E') {
			filetype = FILETYPE_PE;
			pebase = pe;
		}
	} else {
		printf("Unknown filetype\n");
	}

	return filetype;
}


int main(int argc, char **argv, char **envp)
{
	int c;

	while ((c = getopt(argc, argv, "acerlishL:ESvt")) != -1)
	{
		switch( c ) {
		case 'a':
			action |= ACTION_ARCH;
			break;
		case 'b':
			action |= ACTION_BASE;
			break;
		case 't':
			action |= ACTION_FILETYPE;
			break;
		case 'i':
			action |= ACTION_IMPORTS;
			break;
		case 'c':
			action |= ACTION_CHECKSUM;
			break;
		case 'E':
			action |= ACTION_EXPORTS;
			break;
		case 's':
			action |= ACTION_SYMBOLS;
			break;
		case 'S':
			action |= ACTION_SECTIONS;
			break;
		case 'e':
			action |= ACTION_ENTRY;
			break;
		case 'l':
			action |= ACTION_LIBS;
			break;
		case 'L':
			printf("0x%08lx %s\n", addr_for_lib(optarg), optarg);
			action |= ACTION_NOP;
			break;
		case 'r':
			break;
		case 'v':
			verbose = 1;
			break;
		case 'h':
		default:
			return rabin_show_help();
		}
	}

	file = argv[optind];

	if (action == ACTION_UNK)
		return rabin_show_help();
	if (action != ACTION_NOP) {
		if (file == NULL)
			return rabin_show_help();
		fd = open(file, O_RDONLY);
		if (fd == -1) {
			fprintf(stderr, "Cannot open file\n");
			return 0;
		}
	} else return 0;

	rabin_identify_header();

	if (action&ACTION_FILETYPE)
		rabin_show_filetype();
	if (action&ACTION_ARCH)
		rabin_show_arch(file);
	if (action&ACTION_ENTRY)
		rabin_show_entrypoint(file);
	if (action&ACTION_EXPORTS)
		rabin_show_exports(file);
	if (action&ACTION_IMPORTS)
		rabin_show_imports(file);
	if (action&ACTION_SYMBOLS)
		rabin_show_symbols(file);
	if (action&ACTION_SECTIONS)
		rabin_show_sections();
	if (action&ACTION_LIBS)
		rabin_show_libs();
	if (action&ACTION_CHECKSUM)
		rabin_show_checksum();

	close(fd);

	return 0;
}
