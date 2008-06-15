/*
 * Copyright (C) 2006, 2007, 2008
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

#define BSIZE (1024*1024)

#define MAX_PATLEN 10

typedef struct _fnditem
{
	unsigned char str[MAX_PATLEN];
	void* next;
} fnditem;

fnditem* init_fi (  )
{
	fnditem* n;
	n = (fnditem*) malloc ( sizeof ( fnditem ) );
	n->next = NULL;

	return n;
}

void add_fi ( fnditem* n, unsigned char* blk, int patlen )
{
	fnditem* p;

	p = n;
	while ( p->next != NULL ) p=p->next;

	p->next = (fnditem*) malloc ( sizeof ( fnditem ) );
	p=p->next ;

	memcpy ( p->str, blk, patlen);
	p->next = NULL;

}

int is_fi_present ( fnditem* n, unsigned char* blk , int patlen)
{
	fnditem* p;

	p = n;
	while ( p->next != NULL ) {
		if ( !memcmp ( blk, p->str, patlen ) )
			return 1;
		p=p->next;
	}

	return 0;
}

int do_byte_pat ( int patlen) 
{
	unsigned char block[BSIZE+MAX_PATLEN];
	unsigned char sblk[MAX_PATLEN+1];
	
	static fnditem* root;

	u64 bproc = 0;
	ssize_t rb;
	int srch=0;

	int nr,i;
	int pcnt, cnt=0, k =0 ;
	u64 intaddr;
	u64 bytes =  (config.limit!=0)?(config.limit-config.seek):config.block_size;
	u64 bact = config.seek ;

	bytes += bact;

	root = init_fi ( ) ;

	radare_controlc();

	pcnt = 0;
	while ( !config.interrupted && bact < bytes ) {
	//	radare_seek ( bact , SEEK_SET );
		bproc = bact + patlen ;
//		read ( fd, sblk, patlen );
		radare_read_at(bact, sblk, patlen);
		sblk[patlen]=0;

		intaddr = bact;
		cnt = 0;
		while ( !config.interrupted && bproc < bytes ) {
			nr = ( (bytes-bproc) < BSIZE)?(bytes-bproc):BSIZE;
			nr = nr + ( patlen - (nr % patlen) ); // tamany de bloc llegit multiple superior de tamany busqueda
			//rb = read ( fd, block, nr );
			rb = radare_read_at ( bproc , block, nr );
			for ( i = 0 ; i < nr ; i ++ ) {
				if ( !memcmp ( &block[i] , sblk	, patlen ) && !is_fi_present ( root, sblk, patlen) ) {	
					if ( cnt == 0) {
						cons_printf ( "\n");
						add_fi( root, sblk , patlen);
						cons_printf("pattern %d\n", ++pcnt);
						for ( k = 0 ; k < patlen ; k++ )
							cons_printf ( "%02x ", sblk[k] );
						cons_printf ( "\n   0x%08lx\n", intaddr );

						if (1) {
							char cmd[1024];
							sprintf(cmd, "pD %d @ 0x%08llx", patlen, intaddr);
							radare_cmd(cmd,0);
							radare_controlc();
						}
					}
					cnt++;
					cons_printf ( "   %.2d  0x%8.8lx\n", cnt, bproc+i );
				}
			}
			bproc +=  rb;
		}

		bact++;
	}
	radare_controlc_end();
}

/*
int main ( int argc, char* argv[] )
{
	int fd;
	u64 size;
	int r;

	fd = open ( argv[1] , O_RDONLY );
	
	if ( fd < 0 )
	{	
		printf ( "Could not open %s\n", argv[1]);
		return -1; 
	}	

	size = lseek ( fd , 0 , SEEK_END );
	
	lseek ( fd , 0 , SEEK_SET );
	

	r = do_byte_pat ( fd ,size  ) ;



}
*/
