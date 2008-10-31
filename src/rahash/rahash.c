/*
 * Copyright (C) 2006, 2007, 2008
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

#include "../radare.h"
#include "../../global.h"
#include "../utils.h"
#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <sys/types.h>
#if __linux__
#include <arpa/inet.h>
#endif
#if __BSD__
#include <netinet/in.h>
#endif
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "md5.h"
#include "sha1.h"
#include "sha2.h"
#include "hash.h"
#define eprintf(x) fprintf(stderr,x)

extern unsigned char hamming_distances(char *buf, int len);

#define HASHER_VERSION 1
//#define ALGO_STRING "md4, md5, crc16, crc32, sha1,\n               .. par, xor, xorpair, mod255, hamdist, entropy, all"
#define ALGO_STRING "md4, md5, crc16, crc32, sha1, sha256, sha384, sha512, par, xor, xorpair, mod255, hamdist, entropy, all"
#define OPEN_READ O_RDONLY
#define OPEN_WRITE O_TRUNC|O_CREAT|O_WRONLY
#define OPEN_WRITE_PERMS S_IRUSR|S_IWUSR

enum {
	ALGO_MD4     = 0,
	ALGO_MD5     = 1,
	ALGO_CRC16   = 2,
	ALGO_CRC32   = 3,
	ALGO_SHA1    = 4,
	ALGO_SHA256  = 5,
	ALGO_SHA384  = 6,
	ALGO_SHA512  = 7,
	ALGO_PAR     = 8,
	ALGO_XOR     = 9,
	ALGO_XORPAIR = 10,
	ALGO_HAMDIST = 11,
	ALGO_ENTROPY = 12,
	ALGO_MOD255  = 13,
	ALGO_ALL     = 0x77,
	ALGO_SUPPORTED = 0x7f
};

struct {
	char *name;
	int size;
} algorithms[] = {
	{ "md4",    16 },
	{ "md5",    16 },
	{ "crc16",   2 },
	{ "crc32",   4 },
	{ "sha1",   20 },
	{ "sha256", 32 },
	{ "sha384", 48 },
	{ "sha512", 64 },
	{ "par",     1 },
	{ "xor",     1 },
	{ "xorpair", 4 },
	{ "hamdist", 1 },
	{ "entropy", 4 },
	{ "mod255",  1 },
	{ "unknown", 0 },
	{0,0}
};

#define algo_size(x) algorithms[x].size
#define algo_name(x) algorithms[x].name

enum {
	ACTION_GEN = 0,
	ACTION_CHK = 1,
	ACTION_OUT = 2
};

#define ALGO_NAME(a) \
(a==ALGO_MD5)?"md5":\
(a==ALGO_MD4)?"md4":\
(a==ALGO_CRC16)?"crc16":\
(a==ALGO_CRC32)?"crc32":\
(a==ALGO_SHA1)?"sha1":\
(a==ALGO_SHA256)?"sha256":\
(a==ALGO_SHA384)?"sha384":\
(a==ALGO_SHA512)?"sha512":\
(a==ALGO_PAR)?"par":\
(a==ALGO_XOR)?"xor":\
(a==ALGO_XORPAIR)?"xorpair":\
(a==ALGO_HAMDIST)?"hamdist":\
(a==ALGO_MOD255)?"mod255":\
"unkwnown"

MD5_CTX         context;
SHA_CTX         sha1_ctx;
SHA256_CTX	sha256_ctx;
SHA384_CTX	sha384_ctx;
SHA512_CTX	sha512_ctx;

int do_unlink = 0;
int verbose   = 1;
int whole     = 0;
int quite     = 1;

#define MAGIC "R#\02\00"
#define MAGIC_SZ 4
struct header_t {
	char magic[MAGIC_SZ];
	u64 header_size;
	unsigned char version;
	unsigned char offsz;
	unsigned char endian;
	u64 file_size;
	u64 from; /* seek */
	u64 to;
	u64 length;
	unsigned char algorithm;
	u64 block_size;
	char file_name[128]; // XXX must be variable size
} header;

u64 get_offset (const char *arg)
{
	u64 ret = 0;
	int i;

	for(i=0;arg[i]==' ';i++);
	for(;arg[i]=='\\';i++); i++;

	if (arg[i] == 'x' && i>0 && arg[i-1]=='0')
		sscanf(arg, OFF_FMTx, &ret);
	else
		sscanf(arg, OFF_FMTd, &ret);

	switch(arg[strlen(arg)-1]) {
	case 'K': case 'k':
		ret*=1024;
		break;
	case 'M': case 'm':
		ret*=1024*1024;
		break;
	case 'G': case 'g':
		ret*=1024*1024*1024;
		break;
	}

	return ret;
}

void print_header()
{
	u64 one = header.file_size / header.block_size;
	u64 two = ((header.file_size % header.block_size) > 0)?(u64)1:(u64)0;

	if (header.algorithm > ALGO_SUPPORTED) {
		fprintf(stderr, "Unknown hash algorithm.\n");
		exit(1);
	}

	if (header.version != HASHER_VERSION)
		fprintf(stderr, "WARNING: Invalid header version.\n");

	if (quite == 0) {
		printf("file_name  %s\n", header.file_name);
		printf("offt_size  %d\n", header.offsz);
		printf("endian     %d (%s)\n", header.endian, header.endian?"big":"little");
		printf("version    %d\n", header.version);
		printf("block_size "OFF_FMTd"\n", header.block_size);
		printf("file_size  "OFF_FMTd"\n", header.file_size);
		printf("fragments  "OFF_FMTd"\n", one + two );
		printf("file_name  %s\n", header.file_name);
		printf("from       "OFF_FMTd"\n", header.from);
		printf("to         "OFF_FMTd"\n", header.to);
		printf("length     "OFF_FMTd"\n", header.length);
		printf("algorithm  %s\n", ALGO_NAME(header.algorithm));
		printf("algo_size  %d\n", algorithms[header.algorithm].size);
	}
}

void endian_header()
{
	header.header_size = htonl(header.header_size);
	header.file_size = htonl(header.file_size);
	header.from = htonl(header.from);
	header.to = htonl(header.to);
	header.length = htonl(header.length);
	header.block_size = htonl(header.block_size);
}

void unendian_header()
{
	header.header_size = ntohl(header.header_size);
	header.file_size = ntohl(header.file_size);
	header.from = ntohl(header.from);
	header.to = ntohl(header.to);
	header.length = ntohl(header.length);
	header.block_size = ntohl(header.block_size);
}

int radare_hash_generate(char *src, char *dst)
{
	int sd, dd = -2;
	unsigned char *buffer;
	unsigned char *digest;
	u64 ptr = 0;
	u64 len, i;
	float floah = 0;

	if (!strcmp(src, "-"))
		sd = 0;
	else	sd = open(src, OPEN_READ, NULL);

	if (dst)
		dd = open(dst, OPEN_WRITE, OPEN_WRITE_PERMS);

	if (sd == -1 || dd == -1) {
		printf("Cannot open source or destination files.\n");
		return 1;
	}

	// alloc 1024 bytes..no bigger hash atm..
	digest = (unsigned char *)malloc(1024); //(unsigned char *)malloc(algorithms[header.algorithm].size+1);
	memset(digest, 0, algorithms[header.algorithm].size);

	/* write header */
	header.header_size = sizeof(struct header_t);
	if (sd != 0) {
		header.file_size = lseek(sd, 0, SEEK_END);
		lseek(sd, 0, SEEK_SET);
	}

	if (header.length != 0)
		header.to = header.from + header.length;

	if (header.from && (header.from > header.file_size)) {
		fprintf(stderr, "Initial seek out of range.\n");
		return 1;
	}

	if (header.length && ((header.from + header.length) > header.file_size)) {
		header.length = header.file_size - header.from;
		fprintf(stderr, "Length too far. Truncated to "OFF_FMTd" bytes.\n", header.length);
	} else {
		header.length = header.file_size - header.from;
	}

	if (header.to) {
		if (header.to > header.file_size) {
			header.to = header.file_size;
			fprintf(stderr,"End reaches EOF. Truncated to "OFF_FMTd" bytes.\n", header.to);
		}
		header.length = header.to - header.from;
	} else
		header.to = header.file_size;


	if (header.from && (header.from >= header.to)) {
		fprintf(stderr, "Initial seek cannot be greater than end.\n");
		return 1;
	}

	strncpy(header.file_name, src, 128);

#if 0
	if (header.block_size > header.to - header.from) {
		header.block_size = header.to - header.from + 1;
	}
#endif
	if (sd == 0) {
		header.to = -1;
		header.file_size = -1;
		header.length = -1;
	}
	if (header.file_size == 0)
		header.file_size = -1;

	print_header();

	if (dst) {
		u64 size = header.header_size;
		endian_header();
		write(dd, &header, size);
		unendian_header();
	}

	buffer = (unsigned char *)malloc(header.block_size);
	for (ptr = header.from; header.file_size == -1 || ptr < header.to; ptr += header.block_size) {
		len = read(sd, buffer, header.block_size);
		// sometimes read from stdin is 4096!!
		//printf("LEN %d %d\n", (int) header.block_size, (int)len);
		if (len == 0) break;
		if (((int)len) < 0) {
			fprintf(stderr,"Unexpected error reading from the source file.\n");
			free(buffer);
			return 1;
		}

		if (header.length!=-1 && header.block_size > header.length)
			len = header.length;

		switch(header.algorithm) {
		case ALGO_ALL:
			// XXX handle endian stuff here
			printf("par:     %01x\n", hash_par(buffer+header.from, len));
			printf("xor:     %02x\n", hash_xor(buffer+header.from, len));
			printf("hamdist: %02x\n", hash_hamdist(buffer+header.from, len));
			printf("xorpair: %04x\n", hash_xorpair((u8 *)buffer+header.from, len));
			printf("entropy: %.2f\n", hash_entropy(buffer+header.from, len));
			printf("mod255:  %02x\n", hash_mod255(buffer+header.from, len));
			printf("crc16:   %04x\n", crc16(0, buffer, len));
			printf("crc32:   %04x\n", crc32(buffer, len));
			{ /* md4 */
				mdfour(digest, buffer+header.from, len);
				printf("md4:     ");
				for (i = 0; i < 16; i++)
					printf ("%02x", digest[i]);
				printf("\n");
			}
			{ /* md5 */
				MD5Init(&context);
				MD5Update(&context, buffer + header.from, len);
				MD5Final(digest, &context);
				printf("md5:     ");
				for (i = 0; i < 16; i++)
					printf ("%02x", digest[i]);
				printf("\n");
			}
			{ /* sha1 */
				SHA1_Init(&sha1_ctx);
				SHA1_Update(&sha1_ctx, buffer+header.from, len);
				SHA1_Final(digest, &sha1_ctx);
				printf("sha1:    ");
				for (i = 0; i < 20; i++)
					printf ("%02x", digest[i]);
				printf("\n");
			}
			{ /* sha256 */
				SHA256_Init(&sha256_ctx);
				SHA256_Update(&sha256_ctx, buffer+header.from, len);
				SHA256_Final(digest, &sha256_ctx);
				printf("sha256:  ");
				for (i = 0; i < 32; i++)
					printf ("%02x", digest[i]);
				printf("\n");
			}
			{ /* sha384 */
				SHA384_Init(&sha384_ctx);
				SHA384_Update(&sha384_ctx, buffer+header.from, len);
				SHA384_Final(digest, &sha384_ctx);
				printf("sha384:  ");
				for (i = 0; i < 48; i++)
					printf ("%02x", digest[i]);
				printf("\n");
			}
			{ /* sha512 */
				SHA512_Init(&sha512_ctx);
				SHA512_Update(&sha512_ctx, buffer+header.from, len);
				SHA512_Final(digest, &sha512_ctx);
				printf("sha512:  ");
				for (i = 0; i < 64; i++)
					printf ("%02x", digest[i]);
				printf("\n");
			}
			break;
		case ALGO_PAR:
			digest[0] = hash_par(buffer+header.from, len);
			break;
		case ALGO_XOR:
			digest[0] = hash_xor(buffer+header.from, len);
			break;
		case ALGO_XORPAIR: {
			unsigned short s;
			unsigned char *p = (unsigned char *)&s;
			s = hash_xorpair(buffer+header.from, len); //(unsigned short *)(buffer + header.from), len);
			digest[0] = p[0]; digest[1] = p[1];
			} break;
		case ALGO_MD5:
			MD5Update(&context, buffer + header.from, len);
			MD5Final(digest, &context);
			break;
		case ALGO_MD4:
			mdfour(digest, buffer+header.from, len);
			break;
		case ALGO_CRC16: {
			unsigned int *p = (unsigned int *)digest;
			*p = (unsigned int)htons(crc16(0, buffer+header.from, len));
			} break;
		case ALGO_CRC32: {
			unsigned int *p = (unsigned int*) digest;
			*p = (unsigned int)htonl(crc32((buffer + header.from), len));
			} break;
		case ALGO_SHA1:
			SHA1_Update(&sha1_ctx, buffer+header.from, len);
			SHA1_Final(digest, &sha1_ctx);
			break;
		case ALGO_SHA256:
			SHA256_Update(&sha256_ctx, buffer+header.from, len);
			SHA256_Final(digest, &sha256_ctx);
			break;
		case ALGO_SHA384:
			SHA384_Update(&sha384_ctx, buffer+header.from, len);
			SHA384_Final(digest, &sha384_ctx);
			break;
		case ALGO_SHA512:
			SHA512_Update(&sha512_ctx, buffer+header.from, len);
			SHA512_Final(digest, &sha512_ctx);
			break;
		case ALGO_HAMDIST:
			digest[0] = hash_hamdist(buffer+header.from, len);
			break;
		case ALGO_ENTROPY:
			floah = hash_entropy(buffer+header.from, len);
			break;
		case ALGO_MOD255:
			digest[0] = hash_mod255(buffer+header.from,len);
			//for (i = 0; i < header.block_size; i++)
			//	digest[0] += buffer[header.from+i];
			break;
		default:
			eprintf("Unimplemented\n");
			break;
		}

		if (dst)
			write(dd, digest, algorithms[header.algorithm].size);

		if (verbose && header.algorithm != ALGO_ALL) {
			if (quite == 0)
			printf("0x"OFF_FMT" ", ptr);
			if (header.algorithm == ALGO_ENTROPY)
				printf("%.3f", floah);
			else
			for (i = 0; i < algo_size(header.algorithm); i++)
				printf ((header.algorithm==ALGO_PAR)?"%d":"%02x", digest[i]);
			printf("\n");
		}
	}

	close(sd);
	if (dst) close(dd);
	free( digest );
	if (do_unlink) unlink(src);
	free(buffer);

	return 0;
}

int radare_hash_check(char *src, char *dst)
{
	int sd = -1, dd = -2;
	u8 *buffer = (u8*)malloc(header.block_size); //header.block_size];
	u64 ptr = 0;
	unsigned int i, hsize;
	int cmp;
	u64 len = header.block_size;
	u8 digest[256];
	u8 digest_hashed[256];

	if (src)
	sd = open(src, OPEN_READ, NULL);
	dd = open(dst, OPEN_READ, NULL);
	if (src == NULL) { 
		if (dd == -1) {
			printf(" 1 Cannot open source or destination files.\n");
			return 1;
		}
	} else 
	if ((sd == -1 || dd == -1)) {
		printf(" 2 Cannot open source or destination files.\n");
		return 1;
	}

	/* read header */
	lseek(dd, 0, SEEK_SET);
	read(dd, &header, 4);
	if (memcmp(MAGIC, header.magic, MAGIC_SZ)) {
		fprintf(stderr, "Invalid MAGIC signature.\n");
		goto _close;
		return 1;
	}
	lseek(dd, MAGIC_SZ, SEEK_SET);
	read(dd, &hsize, sizeof(u64));
	hsize = ntohl(hsize);
	unendian_header();
 	lseek(dd, 0, SEEK_SET);
	if (hsize != sizeof(struct header_t)) {
		printf("Incompatible header size.\n");
		exit(1);
	}
	read(dd, &header, hsize); //? ---.
	unendian_header();         //     )
	//lseek(dd, hsize, SEEK_SET); //? -' // not necesary

	/* allocate memory */
	memset(digest, 0, algorithms[header.algorithm].size);

	print_header();

	for (ptr = 0; header.file_size==-1 || ptr < header.file_size; ptr += header.block_size) {
		read(dd, &digest_hashed, algo_size(header.algorithm));

		/* check or show? */
		if (src) {
			/* read block */
			len = read(sd, &buffer, header.block_size);
			if ((int)len < header.block_size) {
			//	fprintf(stderr,"ERROR: Unexpected end of file reading %d/%d bytes from the source file.\n", len, header.block_size);
				//exit(1);
			}

			/* hash block */
			// XXX dupped repeated code
			switch(header.algorithm) {
			case ALGO_PAR:
				digest[0] = hash_par(buffer+header.from, len);
				break;
			case ALGO_XOR:
				digest[0] = hash_xor(buffer+header.from, len);
				break;
			case ALGO_XORPAIR: {
				unsigned short s;
				unsigned char *p = (unsigned char *)&s;
				s = hash_xorpair((u8*)buffer + header.from, len);
				digest[0] = p[0]; digest[1] = p[1];
				} break;
			case ALGO_MD5:
				MD5Update(&context, &buffer, len);
				MD5Final(digest, &context);
				break;
			case ALGO_CRC16:
				{
				unsigned int *p = (unsigned int *)digest;
				*p = (unsigned int)htons(crc16(0,buffer, len));
				}
				break;
			case ALGO_CRC32:
				{
				u32 *p = (u32 *)digest;
				*p = (u32)htonl(crc32(buffer, len));
				}
				break;
			case ALGO_SHA1:
				SHA1_Update(&sha1_ctx, &buffer, len);
				SHA1_Final(digest, &sha1_ctx);
				break;
			default:
				eprintf("Unimplemented\n");
				break;
			}

			/* compare block */
			cmp = memcmp(digest, digest_hashed, algorithms[header.algorithm].size);

			if (cmp || verbose) {
				printf(OFF_FMT" ", (u64)ptr);
				for (i = 0; i < algorithms[header.algorithm].size; i++)
					printf ("%02X", digest[i]);
			}
			if ( cmp ) {
				printf(" != ");
				for (i = 0; i < algorithms[header.algorithm].size; i++)
					printf ((header.algorithm==ALGO_PAR)?"%d":"%02X", digest_hashed[i]);
				printf("\n");
			}
		} else {
			if (quite == 0)
			printf(OFF_FMT" ", ptr);
			for (i = 0; i < algo_size(header.algorithm); i++)
				printf ((header.algorithm==ALGO_PAR)?"%d":"%02X", digest_hashed[i]);
			printf("\n");
		}

		if (len < header.block_size)
			break;
	}

_close:
	if (src)
	close(sd);
	close(dd);

	return 0;
}

int radare_go_hash(int action, char *src, char *dst)
{
	int ret = 0;

	switch(action) {
	case ACTION_GEN:
		ret = radare_hash_generate(src, dst);
		break;
	case ACTION_OUT:
	case ACTION_CHK:
		ret = radare_hash_check(src, dst);
		break;
	default:
		printf("Invalid action\n");
		break;
	}
	
	return ret;
}

int main(int argc, char **argv)
{
	int action = ACTION_GEN;
	int c, i;
	char *src = NULL;
	char *dst = NULL;

	header.length     = 0;
	header.version    = HASHER_VERSION;
	header.block_size = 32768;
	header.algorithm  = ALGO_MD5;
	header.endian     = !LIL_ENDIAN;
	header.offsz      = sizeof(u64);
	memcpy(header.magic, MAGIC, MAGIC_SZ);

	while ((c = getopt(argc, argv, "Ab:a:ofvgchqs:S:L:E:V")) != -1) {
		switch( c ) {
		case 'A':
			header.algorithm = ALGO_ALL;
			break;
		case 'a':
			if (!strcmp(optarg, "all")) {
				header.algorithm = ALGO_ALL;
			} else {
				for(i=0;algorithms[i].size;i++) {
					if (!strcmp(optarg, algorithms[i].name)) {
						header.algorithm = i;
						break;
					}
				}
				if (algorithms[i].size == 0) {
					fprintf(stderr,"Invalid algorithm. Available: " ALGO_STRING "\n");
					return 1;
				}
			}
			break;
		case 'V':
			printf("%s v%d "ALGO_STRING"\n", VERSION, HASHER_VERSION);
			return 0;
		case 'g':
			action = ACTION_GEN;
			break;
		case 'c':
			action = ACTION_CHK;
			break;
		case 'o':
			action = ACTION_OUT;
			break;
		case 'q':
			verbose = 0;
			break;
		case 'v':
			quite = 0;
			break;
		case 'f':
			whole = 1;
			break;
		case 's':
			src = (char *)malloc(16);
			strcpy(src, "/tmp/hs.XXXXXX");
			if (mkstemp(src) == 0 || (c = open(src, OPEN_WRITE, NULL))==-1) {
				fprintf(stderr, "Unable to create temporally file %s.", src);
				return 1;
			}
			write(c, optarg, strlen(optarg));
			close(c);
			do_unlink = 1;
			quite = 1;
			verbose = 1;
			break;
		case 'b':
			header.block_size = get_offset(optarg);
			break;
		case 'S':
			header.from = get_offset(optarg);
			break;
		case 'L':
			header.length = get_offset(optarg);
			break;
		case 'E':
			header.to = get_offset(optarg);
			break;
		case 'h':
		default:
			printf(
			"rahash [-action] [-options] [source] [hash-file]\n"
			" actions:\n"
			"  -g           generate (default action)\n"
			"  -c           check changes between source and hash-file\n"
			"  -o           shows the contents of the source hash-file\n"
			"  -A           use all hash algorithms\n"
			" options:\n"
			"  -a [algo]    algorithm to hash ("ALGO_STRING")\n"
			"  -s [string]  hash this string instead of a file\n"
			"  -S [offset]  seek initial offset to\n"
			"  -E [offset]  end hashing at offset\n"
			"  -L [length]  end hashing at length\n"
			"  -b [size]    sets the block size (default 32KB)\n"
			"  -f           block size = file size (!!)\n"
			"  -q           quite output (can be combined with -v)\n"
			"  -V           show version information\n"
			"  -v           be verbose\n");
			return 0;
		}
	}

	// XXX may be broken?
	//if (header.block_size > header.length)
		//header.block_size = header.length;

	if (do_unlink) {
		if (action != ACTION_GEN) {
			fprintf(stderr, "-s only for -g.\n");
			return 1;
		}
	} else
	if (optind < argc) {
		src = argv[optind++];
		if ( optind < argc )
			dst = argv[optind];
	} else {
		fprintf(stderr, "Usage: rahash [-qv] [-gcoA] [[-a algo] [-s str] [-bSEL num]] [src] [hash]\n");
		return 1;
	}
	if (action == ACTION_OUT) {
		dst = src;
		src = NULL;
	}

	switch(header.algorithm) {
	case ALGO_MD5:     MD5Init(&context);        break;
	case ALGO_SHA1:    SHA1_Init(&sha1_ctx);     break;
	case ALGO_SHA256:  SHA256_Init(&sha256_ctx); break;
	case ALGO_SHA384:  SHA384_Init(&sha384_ctx); break;
	case ALGO_SHA512:  SHA512_Init(&sha512_ctx); break; }

	if (whole) {
		FILE *fd = fopen(src, "r");
		if (fd) {
			fseek(fd, 0, SEEK_END);
			header.block_size = \
			header.file_size = ftell(fd);
			fclose(fd);
		}
	}

	return radare_go_hash(action, src, dst);
}
