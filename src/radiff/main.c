/*
 * Copyright (C) 2008
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

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "radare.h"
#include "rdb.h"

int radiff_bytediff(const char *a, const char *b);

void radiff_help()
{
	printf("Usage: radiff [-c] [-bgeirp] [file-a] [file-b]\n");
	printf("  -b   bytediff (faster but doesnt support displacements)\n");
	printf("  -d   use gnu diff as backend (default)\n");
	printf("  -e   use erg0ts bdiff (c++) as backend\n");
	printf("  -r   use rdb diff (code analysis diff)\n");
	printf("  -s   use rsc symdiff\n");
	printf("  -S   use rsc symbytediff\n");
//	printf("  -i   converts a source idc file to a rdb (radare database)\n");
	printf("  -p   binpatching (TODO)\n");
	exit(1);
}

/* from rdbdiff_main.c */
int main_rdb_diff(char *file0, char *file1)
{
	struct program_t *p0, *p1;
	struct block_t *block;

	p0 = program_open(file0);
	p1 = program_open(file1);

	if (p0 == NULL || p1 == NULL) {
		printf("Error opening rdb database\n");
		return 1;
	}
	printf("%s entrypoint = 0x%08llx\n", p0->name, p0->entry);
	printf("%s entrypoint = 0x%08llx\n", p1->name, p1->entry);

#if 0
	{
		struct list_head *i;
		list_for_each_prev(i, &(p0->blocks)) {
			struct block_t *bt = list_entry(i, struct block_t, list);
			printf("%08x\n", bt->addr);
		}
	}
#endif
   
	block = program_block_get(p0, p0->entry);
	if (block) 
		printf("checksum %08x\n", block->n_bytes);

	block = program_block_get(p1, p1->entry);
	if (block) 
		printf("checksum %08x\n", block->n_bytes);

	return rdb_diff(p0, p1, 0);
}

int radiff_symdiff(const char *a, const char *b)
{
	char buf[8096];
	snprintf(buf, 8095, "rsc symdiff %s %s", a, b);
	return system(buf);
}

int radiff_symbytediff(const char *a, const char *b)
{
	char buf[8096];
	snprintf(buf, 8095, "rsc symbytediff %s %s", a, b);
	return system(buf);
}

int radiff_bindiff(const char *a, const char *b)
{
	char buf[8096];
	snprintf(buf, 8095, "bindiff %s %s | rsc bdcolor 3", a, b);
	return system(buf);
}

int radiff_ergodiff(const char *a, const char *b)
{
	char buf[8096];
	snprintf(buf, 8095, "bdiff %s %s", a, b);
	return system(buf);
}

int main(int argc, char **argv)
{
	int c;
	int action = 'd';

	while ((c = getopt(argc, argv, "bderiph")) != -1)
	{
		switch( c ) {
		case 'b':
		case 'r':
		case 'd':
		case 'e':
			action = c;
			break;
		default:
			printf("Invalid option\n");
		case 'h':
			radiff_help();
			return 0;
		}
	}
	if ((argc-optind) != 2)
		radiff_help();

	switch(action) {
	case 'e': // erg0t c++ bin diff
		return radiff_ergodiff(argv[optind], argv[optind+1]);
	case 'd': // gnu diff
		return radiff_bindiff(argv[optind], argv[optind+1]);
	case 'b': // bytediff
		return radiff_bytediff(argv[optind], argv[optind+1]);
	case 'r': // rdbdiff
		return main_rdb_diff(argv[optind], argv[optind+1]);
	case 's':
		return radiff_symdiff(argv[optind], argv[optind+1]);
	case 'S':
		return radiff_symbytediff(argv[optind], argv[optind+1]);
	}

	radiff_help();
	return 1;
}
