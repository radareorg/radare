/* ORIGINAL CODE */
/*********************************************************************\
MODULE NAME:    b64.c
AUTHOR:         Bob Trower 08/04/01
PROJECT:        Crypt Data Packaging
COPYRIGHT:      Copyright (c) Trantor Standard Systems Inc., 2001

NOTE:           This source code may be used as you wish, subject to
                the MIT license.
\******************************************************************* */
/* REDUCED CODE by pancake <nopcode><org> */
#if 0
all:
	gcc b64.c
	echo aGVsbG8gd29ybGQA | ./a.out -d
	echo hello world | ./a.out
	cat /etc/services | ./a.out | ./a.out -d > services.txt
	-diff -ru /etc/services services.txt
	cat /bin/true | ./a.out | ./a.out -d > true.txt
	md5sum /bin/true true.txt
	radiff /bin/true true.txt
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Translation Table as described in RFC1113 */
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* encode 3 8-bit binary bytes as 4 '6-bit' characters */
void base64_encodeblock(unsigned char in[3], unsigned char out[4], int len)
{
	out[0] = cb64[ in[0] >> 2 ];
	out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
	out[2] = (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
	out[3] = (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

/* decode 4 '6-bit' characters into 3 8-bit binary bytes */
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";
int base64_decodeblock(unsigned char in[4], unsigned char out[3])
{
	unsigned char len = 3, i, v[4];
	for(i=0;i<4;i++) {
		if (in[i]<43||in[i]>122)
			return -1;
		v[i] = cd64[in[i]-43];
		if (v[i]!='$') v[i]-=62;
		else { len = i-1; break; }
	}
	out[0] = v[0] << 2 | v[1] >> 4;
	out[1] = v[1] << 4 | v[2] >> 2;
	out[2] = ((v[2] << 6) & 0xc0) | v[3];
	return len;
}

#if 0
int main(int argc, char **argv)
{
	int len;
	char in[4], out[4];
	if (argc>1) {
		if (argv[1][1]=='h') {
			printf("Encode: b64 < file\n");
			printf("Decode: b64 -d < file\n");
			return 0;
		}
		/* decode */
		while(!feof(stdin)) {
			len = read(0, in, 4);
			if (len<1) break;
			len = base64_decodeblock(in, out);
			if (len<0)
				break;
			write(1, out, len);
		}
	} else {
		/* encode */
		while(!feof(stdin)) {
			len = read(0, in, 3);
			if (len<1) break;
			base64_encodeblock(in, out, len);
			write(1, out, 4);
		}
	}
	return 0;
}

int test()
{
	/* encode example */
	{
		int i,o;
		char *a = strdup("hello world");
		char *b = malloc(1024);
		memset(b,0,1023);
		for(i=o=0;i<strlen(a);i+=3,o+=4) {
			base64_encodeblock(a+i,b+o,73);
		}
		printf("%s\n", b);
	}

	/* decode example */
	{
		int i,o;
		char *a = strdup("aGVsbG8gd29ybGQA");
		char *b = malloc(1024);
		memset(b,0,1023);
		for(i=o=0;i<strlen(a);i+=4,o+=3) {
			base64_decodeblock(a+i,b+o);
		}
		printf("%s\n", b);
	}
	return 0;
}
#endif
