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

int hash_par(unsigned char *buffer, int len)
{
	unsigned int i, ones=0;
	for(i=0;i<len;i++) {
		unsigned char x = buffer[i];
		ones += ((x&128)?1:0) + ((x&64)?1:0) + ((x&32)?1:0) + ((x&16)?1:0) +
			((x&8)?1:0) + ((x&4)?1:0) + ((x&2)?1:0) + ((x&1)?1:0);
	}
	return ones%2;
}

/* These functions comes from 0xFFFF */
/* fmi: nopcode.org/0xFFFF */
unsigned short hash_xorpair(unsigned short *b, int len)
{
	unsigned short result = 0;
	for(len>>=1;len--;b=b+1)
		result^=b[0];
	return result;
}

unsigned char hash_xor(unsigned char *b, int len)
{
	unsigned char res = 0;
	for(;len--;b=b+1)
		res^=b[0];
	return res;
}

unsigned char hash_mod255(unsigned char *b, int len)
{
	int i, c = 0;
	/* from gdb */
	for (i = 0; i < len; i++)
		c += b[i];
	return c%255;
}
