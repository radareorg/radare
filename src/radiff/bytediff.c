#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define u64 unsigned long long
#define CTXMAXB 3

extern int radare_fmt;
#define R if (radare_fmt)
#define NR if (!radare_fmt)
#define BS 4096

static int do_byte_diff_count (int fd1, int fd2, u64 bytes) 
{
	unsigned char block1[BS];
	unsigned char block2[BS];
	u64 rb, bproc = 0;
	int i, nr, count = 0;

	while ( bproc < bytes ) {
		nr = ( (bytes-bproc) < BS )?(bytes-bproc):BS ;
		rb = read ( fd1, block1 , nr );
		rb = read ( fd2, block2 , nr );
		for ( i = 0 ; i < nr ; i ++ ) {
			if ( block1[i] != block2[i] )
				count++;
		}
		bproc +=  nr;
	}
	printf("%d\n", count);
	return count;
}

static int do_byte_diff (int fd1, int fd2, u64 bytes) 
{
	unsigned char block1[BS];
	unsigned char block2[BS];
	u64 rb, bproc = 0;
	int nr;

	unsigned char bufb[CTXMAXB];
	int atbufb=0;

	int i,k=0,l;
	int outctx=0;

	int inctx=0;
	int cf=0;

	while ( bproc < bytes ) {
		nr = ( (bytes-bproc) < BS )?(bytes-bproc):BS ;
		rb = read ( fd1, block1 , nr );
		rb = read ( fd2, block2 , nr );
		
		for ( i = 0 ; i < nr ; i ++ ) {
			if ( block1[i] != block2[i] ) {
				if ( inctx == 0 ) {
					NR printf ("------\n");
					l = ( (atbufb+1)>=CTXMAXB)?0:atbufb+1;
					for ( k = 0 ; k < CTXMAXB ; k ++ ) {
						NR printf ( "%8.8llx %2.2x\n", (u64)bproc+i-CTXMAXB+k,bufb[l] );
						l  = ( (l+1)>=CTXMAXB)?0:l+1;
					}
					inctx=1;
				}
				R { printf("wx %02x @ 0x%llx\n", block2[i], (u64) bproc+i);
				} else { printf ( "%8.8llx %2.2x   |   %2.2x \n", (u64) bproc+i , block1[i], block2[i]); }
				outctx = CTXMAXB;
				cf++;
			} else {
				bufb[atbufb]=block1[i];
				atbufb = ( (atbufb+1)>=CTXMAXB)?0:atbufb+1;

				if ( outctx > 0 ) {
					NR printf ( "%8.8llx %2.2x\n", (u64)bproc+i-CTXMAXB+k,block1[i] );
					outctx--;
				} else inctx=0;
			}
		}

		bproc +=  nr;
	}
	return cf;
}

int radiff_bytediff(const char *a, const char *b, int count)
{
	int fd1,fd2;
	u64 size1, size2;
	int r;

	fd1 = open (a , O_RDONLY);
	if ( fd1 < 0 ) {	
		fprintf(stderr, "Could not open %s\n", a);
		return -1; 
	}	
	
	fd2 = open (b , O_RDONLY);
	if ( fd2 < 0 ) {	
		fprintf(stderr, "Could not open %s\n", b);
		return -1; 
	}	

	size1 = lseek(fd1 , 0 , SEEK_END);
	size2 = lseek(fd2 , 0 , SEEK_END);
	
	lseek(fd1 , 0 , SEEK_SET);
	lseek(fd2 , 0 , SEEK_SET);

	if (size1 != size2)
		fprintf(stderr, "Warning: Files have different size %lld vs %lld\n", size1, size2);

	if (count)
		r = do_byte_diff_count(fd1, fd2, (size1>size2)?size2:size1);
	else r = do_byte_diff (fd1, fd2, (size1>size2)?size2:size1);

	if ( ( r == 0) && ( size1 == size2 ) )
		fprintf (stderr, "Files are the same\n");

	close(fd1);
	close(fd2);

	return 0;
}
