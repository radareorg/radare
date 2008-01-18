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
	FILETYPE_CLASS
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
" -S        show sections\n"
" -l        linked libraries\n"
" -L [lib]  dlopen library and show address\n"
" -r        flag makes show the output in radare format\n"
" -v        be verbose\n");
	return 1;
}

void rabin_show_checksum()
{
	unsigned long addr = 0;
	unsigned long base = 0;
	switch(filetype) {
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
		printf("0x%08x memory\n", addr);
		printf("0x%08x disk\n", addr - 0x8048000);
		break;
	case FILETYPE_MZ:
		break;
	case FILETYPE_PE:
		lseek(fd, pebase+0x28, SEEK_SET);
		read(fd, &addr, 4);
		printf("0x%08x disk offset for ep\n", pebase+0x28);
		printf("0x%08x disk\n", addr-0xc00);

		lseek(fd, pebase+0x45, SEEK_SET);
		read(fd, &base, 4);
		printf("0x%08x memory\n", base+addr);
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
	char buf[1024];

	switch(filetype) {
	case FILETYPE_ELF:
		sprintf(buf, "objdump -d '%s' | grep '>:' | sed -e 's,<,,g' -e 's,>:,,g' -e 's,^,0x,' | sort | uniq", file);
		system(buf);
		break;
	}
}

void rabin_show_imports()
{
	char buf[1024];

	//sprintf(buf, "readelf -sA '%s'|grep GLOBAL | awk ' {print $8}'", file);
	sprintf(buf, "readelf -s '%s' | grep FUNC | grep GLOBAL | grep DEFAULT  | grep ' UND ' | awk '{ print \"0x\"$2\" \"$8 }' | sort | uniq" , file);
	system(buf);
}

void rabin_show_exports(char *file)
{
	char buf[1024];

	sprintf(buf, "readelf -s '%s' | grep FUNC | grep GLOBAL | grep DEFAULT  | grep ' 12 ' | awk '{ print \"0x\"$2\" \"$8 }' | sort | uniq" , file);
	system(buf);
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

	sprintf(buf, "strings '%s' | grep -e '^lib'", file);
	system(buf);
}

int rabin_identify_header()
{
	unsigned char buf[1024];

	lseek(fd, 0, SEEK_SET);
	read(fd, buf, 1024);
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

	while ((c = getopt(argc, argv, "cerlishL:ESv")) != -1)
	{
		switch( c ) {
		case 'b':
			action |= ACTION_BASE;
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
			printf("0x%08x %s\n", addr_for_lib(optarg), optarg);
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
