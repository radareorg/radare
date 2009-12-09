/*
 * Copyright (C) 2007, 2008, 2009
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

// TODO: add radare related commands to stdout with -r (R printf..)

#include "../main.h"
#if __UNIX__
#include <arpa/inet.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdarg.h>

//int radare_output = 0;
extern int radare_output;

void eprintf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

int hex2int (unsigned char *val, unsigned char c)
{
	if ('0' <= c && c <= '9')      *val = (unsigned char)(*val) * 16 + ( c - '0');
	else if (c >= 'A' && c <= 'F') *val = (unsigned char)(*val) * 16 + ( c - 'A' + 10);
	else if (c >= 'a' && c <= 'f') *val = (unsigned char)(*val) * 16 + ( c - 'a' + 10);
	else return 1;
	return 0;
}

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

static int show_help()
{
	printf("Usage: javasm [-hV] [-a 'opcode'] [-d 'hexpairstring'] [-r][-c 'classfile']\n");
	return 0;
}

int main(int argc, char **argv)
{
	int c,i,j;
	int len;
	unsigned char buf[1024];
	char output[128];
	
	while ((c = getopt(argc, argv, "ra:d:c:hV")) != -1)
	{
		switch( c ) {
		case 'r':
			radare_output = 1;
			break;
		case 'a':
			j = java_assemble(buf, optarg);
			for(i=0;i<j;i++) {
				printf("%02x ", buf[i]);
			}
			printf("\n");
			return 0;
		case 'd':
			len = hexstr2binstr(optarg, buf);
			for(i=0;i<len;i+=j) {
				j = java_disasm(buf+i, output);
				if (j>0) {
					printf("0x%08x   %s\n", i, output);
				} else {
					printf("???\n");
					j = 1;
				}
			}
			return 0;
		case 'c':
			return java_classdump(optarg);
		case 'h':
			return show_help();
		case 'V':
			printf("0.1\n");
			return 0;
		}
	}
	show_help();
	return 0;
}
