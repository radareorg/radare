/*
 * Copyright (C) 2008, 2009
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
#include "radiff.h"

ut64 from = 0;
ut64 to = 0;
int radare_fmt = 0;
int radiff_bytediff(const char *a, const char *b, int count);

ut64 get_num64(const char *str)
{
	ut64 ret = 0LL;
	if (str[0]=='0'&&str[1]=='x')
		sscanf(str, "0x%llx", &ret);
	else sscanf(str, "%lld", &ret);
	return ret;
}

void radiff_help()
{
	printf("Usage: radiff [-bcdgerpsS] [file-a] [file-b]\n");
	printf("  -b        bytediff (faster but doesnt support displacements)\n");
	printf("  -c        code differences (with disassembly and delta support)\n");
	printf("  -d        use gnu diff as backend (default)\n");
	printf("  -n        count of bytes changed\n");
	printf("  -e        use erg0ts bdiff (c++) as backend\n");
	printf("  -p        use program diff (code analysis diff)\n");
	printf("  -s        use rsc symdiff\n");
	printf("  -S        use rsc symbytediff\n");
//	printf("  -i   converts a source idc file to a rdb (radare database)\n");
	printf("  -r        output radare commands\n");
	printf("  -f [from] start at this address\n");
	printf("  -t [to] stop at this address\n");
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
	if (radare_fmt) {
		snprintf(buf, 8095, "rsc bindiff %s %s | rsc bdf2rad", a, b);
	} else snprintf(buf, 8095, "rsc bindiff %s %s | rsc bdcolor 0", a, b);
	return system(buf);
}

int radiff_ng_bindiff(const char *a, const char *b)
{
	char buf[8096];
	snprintf(buf, 8095, "rsc bindiff-ng '%s' '%s'", a, b);
	return system(buf);
}

int radiff_ergodiff(const char *a, const char *b)
{
	char buf[8096];
	snprintf(buf, 8095, "rsc bdiff '%s' '%s'", a, b);
	return system(buf);
}

int main(int argc, char **argv)
{
	int c;
	int action = 'd';

	while ((c = getopt(argc, argv, "f:t:cbdesSriphn")) != -1)
	{
		switch( c ) {
		case 'r':
			radare_fmt = 1;
			break;
		case 'h':
			radiff_help();
			return 0;
		case 'f':
			from = get_num64(optarg);
			break;
		case 't':
			to = get_num64(optarg);
			break;
		case 'b':
		case 'd':
		case 'c':
		case 'p':
		case 'e':
		case 's':
		case 'S':
		case 'n':
			action = c;
			break;
		default:
			printf("Invalid option\n");
		}
	}
	if ((argc-optind) != 2)
		radiff_help();

	switch(action) {
	case 'n':
		return radiff_bytediff(argv[optind], argv[optind+1], 1);
	case 'e': // erg0t c++ bin diff
		return radiff_ergodiff(argv[optind], argv[optind+1]);
	case 'c': // gnu diff
		return radiff_ng_bindiff(argv[optind], argv[optind+1]);
	case 'd': // gnu diff
		return radiff_bindiff(argv[optind], argv[optind+1]);
	case 'b': // bytediff
		return radiff_bytediff(argv[optind], argv[optind+1], 0);
	case 'p': // rdbdiff
		return main_rdb_diff(argv[optind], argv[optind+1]);
	case 's':
		return radiff_symdiff(argv[optind], argv[optind+1]);
	case 'S':
		return radiff_symbytediff(argv[optind], argv[optind+1]);
	}

	radiff_help();
	return 1;
}
