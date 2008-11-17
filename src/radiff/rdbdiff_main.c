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

#include "radare.h"
#include <stdio.h>
#include <getopt.h>
#include "rdb.h"

int udis86_color = 1;

void show_help_message(int v)
{
	printf("Usage: rdbdiff [-hV] [-i fmt] [-o fmt] [file_0] [file_1]\n");
	if (v) {
		printf("  -i [format]    input format (idb, rdb (default))\n");
		printf("  -o [format]    change output format (gml, html, txt (default))\n");
		printf("  -h             shows this help\n");
		printf("  -V             print version information\n");
		printf("environment:\n");
		printf("  ARCH           set architecture (arm, java, x86 (default))\n");
	}
	exit(0);
}

void show_version()
{
	printf("0.1\n");
	exit(0);
}

int main_rdb_diff(char *file0, char *file1)
{
	struct program_t *p0, *p1;
	struct block_t *block;

	p0 = program_new(file0);
	p1 = program_new(file1);

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

int main(int argc, char **argv)
{
	int c;

	while ((c = getopt(argc, argv, "hi:o:V")) != -1)
	{
		switch( c ) {
		case 'h':
			show_help_message(1);
		case 'V':
			show_version();
			break;
		case 'i':
			break;
		case 'o':
			break;
		default:
			show_help_message(0);
		}
	}

	if ((argc-optind) != 2)
		show_help_message(0);

	return main_rdb_diff(argv[optind], argv[optind+1]);
}
