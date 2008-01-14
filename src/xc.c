/*
 * Copyright (C) 2007
 *       pancake <youterm.com>
 *
 * 'xc' is part of the radare project.
 *
 * xc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xc; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/*
 * Helper utility for command line for handling hexadecimal conversions
 * 
 * examples:
 *  $ xc $((`xc 0x33`+`xc 0x33`))
 *  $ declare -i foo=0x33 && echo $foo
 *  $ xc -33 -e -33
 *
 * TODO: Endian support for all conversions!
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int endian = 0;
int strbin = 0;

#define SWAP_ENDIAN \
	if (endian) { \
		unsigned char tmp; \
		tmp = p[0]; p[0]= p[3]; p[3] = tmp; \
		tmp = p[1]; p[1]= p[2]; p[2] = tmp; \
	}

int stdin_bin_to_hex_pairs()
{
	unsigned char c;

	for(;;) {
		if (read(0, &c, 1) <=0)
			break;
		printf("%02x ", c);
	}
	return 0;
}

void xc(char *arg)
{
	unsigned int a = 0;
	float f = 0;
	int neg = 0;

	if (arg[0]=='q')
		exit(0);

	if (!strcmp(arg, "-s")) {
		strbin ^= 1;
		return;
	}

	if (!strcmp(arg, "-e")) {
		endian ^= 1;
		return;
	}
	if (!strcmp(arg, "-")) {
		if (strbin) {
			unsigned int ci;
			unsigned char c;
			while(!feof(stdin)) {
				fscanf(stdin, "%02x", &ci);
				if (feof(stdin)) break;
				c = (int)ci;
				write(1,&c, 1);
			}
			exit(0);
		} else
			exit(stdin_bin_to_hex_pairs());
	}

	if (arg[0]=='-') {
		neg = 1;
		arg = arg + 1;
	}

	if (strbin) {
		unsigned char a;
		for(;arg[0]!='\0'; arg = arg + 2) {
			sscanf(arg, "%02hhx", &a);
			write(1, &a, 1);
			if (arg[2]==' ')
				arg = arg + 1;
		}
		return;
	}

	if ((arg[0] == '?')
	|| (arg[0] == 'h')) {
		printf(" int   ->  hex           ;  xc 10\n");
		printf(" hex   ->  int           ;  xc 0xa\n");
		printf(" -int  ->  hex           ;  xc -77\n");
		printf(" -hex  ->  int           ;  xc 0xffffffb3\n");
		printf(" float ->  hex           ;  xc 3.33f\n");
		printf(" hex   ->  float         ;  xc Fx40551ed8\n");
		printf(" oct   ->  hex           ;  xc 035\n");
		printf(" hex   ->  oct           ;  xc Ox12 (O is a letter)\n");
		printf(" bin   ->  hex           ;  xc 1100011b\n");
		printf(" hex   ->  bin           ;  xc Bx63\n");
		printf(" -e    swap endianness   ;  xc -e 0x33\n");
		printf(" -s    swap hex to bin   ;  xc -s 43 4a 50\n");
		printf(" -     read data from stdin until eof\n");
	} else
	if (!memcmp(arg, "Bx", 2)) {
		/* hex -> bin */
		char *bin1 = malloc((strlen(arg)-2)*4);
		char *bin2 = malloc((strlen(arg)-2)*4);
		char *aux;
		bin2[0] = '\0';
		sscanf(arg+2, "%x", &a);
		if (!a)
			printf("0x0\n");
		else {
			while (a) {
				if (a & 0x1)
					sprintf(bin1, "1%s", bin2);
				else sprintf(bin1, "0%s", bin2);
				aux = bin1;
				bin1 = bin2;
				bin2 = aux;
				a >>= 1;
			}
			printf("%sb\n", bin2);
		}
		free(bin1);
		free(bin2);
	} else
	if (!memcmp(arg, "Ox", 2)) {
		unsigned char *p = (unsigned char *)&a;
		sscanf(arg+2, "%x", &a);
		if (neg) a = -a;
		SWAP_ENDIAN
		printf("%oo\n", a);
	} else
	if (!memcmp(arg, "Fx", 2)) {
		unsigned char *p = (unsigned char *)&a;
		unsigned char *q = (unsigned char *)&f;

		sscanf(arg+2, "%x", &a);
		if (neg) a = -a;
		memcpy(q, p, 4);
		SWAP_ENDIAN
		printf("%ff\n", f);
	} else
	if (!memcmp(arg, "0x", 2)) {
		unsigned char *p = (unsigned char *)&a;
		sscanf(arg,"0x%x", &a);
		if (neg) a = -a;
		SWAP_ENDIAN
		printf("%d\n", a);
	} else
	if ((arg[0]=='0'&&arg[1]!='x')||arg[strlen(arg)-1]=='o') {
		unsigned char *p = (unsigned char *)&a;
		sscanf(arg, "%o", &a);
		if (neg) a = -a;
		SWAP_ENDIAN
		printf("0x%x\n", a);
	} else
	/* bin -> hex */
	if (arg[strlen(arg)-1]=='b') {
		int b = 0;
		for (;arg[0] != 'b'; arg++) {
			switch(arg[0]) {
			case '0': b<<=1; break;
			case '1': b|=(1<<a); break; //a++; break;
			case 'b': /* ignore */ a>>=1; break;
			default:
				fprintf(stderr, "Invalid binary character: %c\n", arg[0]);
				exit(1);
			}
			a++;
		}
		printf("Bx%x\n", b);
	} else
	if (arg[strlen(arg)-1]=='f') {
		unsigned char *p = (unsigned char *)&f;
		sscanf(arg, "%f", &f);
		if (neg) f = -f;
		SWAP_ENDIAN
		printf("Fx%02x%02x%02x%02x\n", p[0], p[1], p[2], p[3]);
	} else {
		unsigned char *p = (unsigned char *)&a;
		sscanf(arg,"%d", (int*)&a);
		if (neg) a = -a;
		SWAP_ENDIAN
		if (arg[0]=='0')
			printf("0x%08x\n", a);
		else
			printf("0x%x\n", a);
	}
}

int main(int argc, char **argv)
{
	int i;
	char buf[1024];

	if (argc == 1) {
		while(!feof(stdin)) {
			fgets(buf, 1000, stdin);
			buf[strlen(buf)-1] = '\0';
			if (feof(stdin)) break;
			xc(buf);
		}
		return 0;
	}

	if (!strcmp(argv[1], "-h"))
		printf("Usage: xc [-] | [-s] [-e] [int|0x|Fx|.f|.o] [...]\n");


	for(i=1; i<argc; i++)
		xc( argv[i] );

	return 0;
}
