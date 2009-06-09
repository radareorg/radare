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

#include "rasm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

//#if !RADARE_CORE

//char *arch = "x86";
char *arch = "rsc";
u64 offset = 0;
int endian = 0;
int verbose = 0;

static int show_version()
{
	printf(VERSION"\n");
	return 0;
}

static int show_helpline()
{
	printf( "Usage: rasm [-elvV] [-f file] [-s offset] [-a arch] [-d bytes] \"opcode\"|-\n");
	return 0;
}

static int show_help()
{
	show_helpline();
	printf(" if 'opcode' is '-' reads from stdin\n");
	printf("  -v           enables debug\n");
	printf("  -d [bytes]   disassemble from hexpair bytes\n");
	printf("  -f [file]    compiles assembly file to 'file'.o\n");
	printf("  -s [offset]  offset where this opcode is suposed to be\n");
	printf("  -a [arch]    selected architecture (x86, x86_64, olly, ppc, arm, java, rsc)\n");
	printf("  -e           use big endian\n");
	printf("  -l           list all supported opcodes and architectures\n");
	printf("  -V           show version information\n");
	return 0;

}

/* assemble */
int rasm_assemble(char *str)
{
	unsigned char data[256];
	int ret = -1;

	if (str!=NULL)
		ret = rasm_asm(arch, &offset, str, data);

	if (ret>0) {
		int i;
		if (endian) {
			if (ret == 4) {
				int tmp = data[0];
				data[0] = data[3];
				data[3] = tmp;
				    tmp = data[1];
				data[1] = data[2];
				data[2] = tmp;
			}
		}
		if (ret<1) {
			fprintf(stderr, "Invalid shit\n");
		} else {
			for(i=0;i<ret;i++) {
				printf("%02x ", data[i]);
			}
			printf("\n");
		}
	}

	return ret;
}


int main(int argc, char **argv)
{
	int c;

	if (argc<2)
		return show_helpline();

	while ((c = getopt(argc, argv, "d:a:Vs:lhef:v")) != -1)
	{
		switch( c ) {
		case 'a':
			arch = optarg;
			break;
		case 'd':
			if (!strcmp(optarg, "-")) {
				char buf[1024];
				while(!feof(stdin)) {
					fgets(buf, 128, stdin);
					if (!feof(stdin)) {
						buf[strlen(buf)-1]='\0';
						rasm_disasm(arch, &offset, buf, NULL);
					}
				}
				return 0;
			}
			rasm_disasm(arch, &offset, optarg, NULL);
			break;
		case 'f':
			if (!verbose) printf("Compiling %s to %s.o\n", optarg, optarg);
			return rasm_file(arch, offset, optarg, NULL);
		case 's':
			offset = get_offset(optarg);
			break;
		case 'e':
			endian = 1;
			break;
		case 'l':
			show_helpline();
			return rasm_show_list();
		case 'h':
			return show_help();
		case 'v':
			verbose = 1;
			break;
		case 'V':
			return show_version();
		}
	}

	// TODO concat argv
	if (argv[optind]) {
		if (!strcmp(argv[optind], "-")) {
			char buf[1024];
			while(!feof(stdin)) {
				fgets(buf, 128, stdin);
				if (!feof(stdin)) {
					buf[strlen(buf)-1]='\0';
					rasm_assemble(buf);
				}
			}
			return 0;
		}
		return rasm_assemble(argv[optind]);
	}
	return 0;
}

//#endif
