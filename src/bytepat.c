/*
 * Copyright (C) 2006, 2007, 2008, 2009
 *       esteve <eslack.org>
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <stdlib.h>

#include "radare.h"
#include "config.h"
#include "main.h"

#define CTXMINB 5

#define BSIZE (4096)

#define MAX_PATLEN 1024

/* XXX memory leak!!! malloc-ed data not free'd */

typedef struct _fnditem
{
	unsigned char str[MAX_PATLEN];
	void* next;
} fnditem;

static fnditem* init_fi()
{
	fnditem* n;
	n = (fnditem*) malloc ( sizeof ( fnditem ) );
	n->next = NULL;
	return n;
}

static void add_fi ( fnditem* n, unsigned char* blk, int patlen )
{
	fnditem* p;

	for(p=n;p->next!=NULL;p=p->next);

	p->next = (fnditem*) malloc(sizeof(fnditem));
	p = p->next;

	memcpy(p->str, blk, patlen);
	p->next = NULL;
}

static int is_fi_present(fnditem* n, unsigned char* blk , int patlen)
{
	fnditem* p;
	for(p=n;p->next!=NULL; p=p->next) {
		if (!memcmp(blk, p->str, patlen))
			return 1;
	}
	return 0;
}

int do_byte_pat(int patlen) 
{
	unsigned char block[BSIZE+MAX_PATLEN];
	unsigned char sblk[MAX_PATLEN+1];
	static fnditem* root;
	u64 bproc = 0;
	int rb, nr;
	int range_n = 0;
	int i, moar;
	int pcnt, cnt=0, k=0;
	u64 intaddr;
	/* end addr */
	u64 bytes;
 	/* start addr */
	u64 bact = config.seek;

	if (patlen < 1 || patlen > MAX_PATLEN) {
		eprintf("Invalid pattern length (must be > 1 and < %d)\n", MAX_PATLEN);
		return 0;
	}
	if (!strnull(config_get("search.from")))
		bact = config_get_i("search.from");
	if (!strnull(config_get("search.to")))
		bytes = config_get_i("search.to");
	if (bact==0)
		bact = config.seek;
 	if (bytes==0)
		bytes = (config.limit!=0)?(config.limit-config.seek):config.block_size;
	if (bytes==0)
		bytes = 0xFFFFFFFF; /* default end */
	D eprintf("Searching from 0x%08llx to 0x%08llx\n", bact, bytes);

	bytes += bact;

	root = init_fi();

	radare_controlc();

	pcnt = -1;
if (config_get("search.inar")) {
	if (! ranges_get_n(range_n++, &bact, &bytes)) {
		eprintf("No ranges defined\n");
		return 0;
	}
	printf("Searching using ranges...\n");
}
do {
	while ( !config.interrupted && bact < bytes ) {
	//	radare_seek ( bact , SEEK_SET );
		bproc = bact + patlen ;
//		read ( fd, sblk, patlen );
		radare_read_at(bact, sblk, patlen);
		sblk[patlen]=0;

		intaddr = bact;
		cnt = 0;
		while ( !config.interrupted && bproc < bytes ) {
			radare_controlc();
			nr = ((bytes-bproc) < BSIZE)?(bytes-bproc):BSIZE;
			nr = nr + ( patlen - (nr % patlen) ); // tamany de bloc llegit multiple superior de tamany busqueda
			//rb = read ( fd, block, nr );
			rb = radare_read_at(bproc, block, nr);
			if (rb<1) {
			//	eprintf("skip %d vs %d at 0x%08llx\n", rb, nr, bact);
				bproc+=nr;
				break;
			}
			nr = rb;
			moar = 0;
			for(i=0; i<nr; i++){
				if (!memcmp(&block[i], sblk, patlen) && !is_fi_present(root, sblk, patlen)){
					if (cnt == 0) {
						cons_newline();
						add_fi( root, sblk , patlen);
						pcnt++;
						cons_printf("bytes:%d: ", pcnt);
						for(k = 0; k<patlen; k++)
							cons_printf("%02x", sblk[k]);
						cons_printf("\nfound:%d: 0x%08llx ", pcnt, intaddr);
					}
					moar++;
					cnt++;
					cons_printf("0x%08llx ", bproc+i );
				}
			}
			if (moar>0) {
				cons_printf("\ncount:%d: %d\n", pcnt, moar+1);
				cons_flush();
			}
			bproc += rb;
		}

		if (moar > 0) {
			bact += (u64)patlen;
		} else bact++;
	}
	cons_newline();
} while(config_get("search.inar") && ranges_get_n(range_n++, &bact, &bytes));
	radare_controlc_end();
	return 0;
}
