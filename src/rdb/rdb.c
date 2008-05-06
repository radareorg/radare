#include "../utils.h"
#include "../list.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "rdb.h"

#if 0

u64 get_offset(char *addr)
{
	u64 off;
	sscanf(addr, "0x%08llx", &off);
	return off;
}

// COPYPASTA
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

#if 0
// returns length, alters arg
int hexstr2binstr(unsigned char *arg) // 0A 3B 4E A0
{
	unsigned char *ptr;
	unsigned char  c = '\0';
	unsigned char  d = '\0';
	unsigned int len = 0, j = 0;

	for (ptr = arg; ;ptr = ptr + 1) {
		if (ptr[0]==0x52 || ptr[0]=='\n' || ptr[0]=='\t' || ptr[0]=='\r')
			continue;
		if (ptr[0]==' ' || j==2) {
			if (j>0) {
				arg[len] = c;
				len++;
				c = j = 0;
			}
			if (ptr[0]==' ')
				continue;
		}

		if (ptr[0] == '\0') break;

		d = c;
		if (hex2int(&c, ptr[0])) {
			eprintf("binstr: Invalid hexa string at char '0x%02x'.\n", ptr[0]);
			return 0;
		}
		c |= d;
		if (j++ == 0) c <<= 4;
	}

	return (int)len;
}
#endif
//==========================================================================================
/*
void *emalloc(void *ptr, int size)
{
	if (ptr == NULL)
		return malloc(size);
	return realloc(ptr, size);
}
*/

char *estrdup(char *ptr, char *string)
{
	if (ptr)
		free(ptr);
	ptr = strdup(string);
	return ptr;
}
#endif

struct xref_t *xref_new(unsigned long addr)
{
	struct xref_t *xt;

	xt = (struct xref_t *)malloc(sizeof(struct xref_t));
	xt->addr      = addr;
	INIT_LIST_HEAD(&(xt->list));

	return xt;
}

struct block_t *block_new(unsigned long addr)
{
	struct block_t *bt;

	bt = (struct block_t *)malloc(sizeof(struct block_t));
	memset(bt, '\0', sizeof(struct block_t));
	bt->addr      = addr;
	INIT_LIST_HEAD(&(bt->xrefs));
	INIT_LIST_HEAD(&(bt->calls));

	return bt;
}

struct block_t *block_get(struct program_t *program, unsigned long addr)
{
	struct list_head *i;

	list_for_each_prev(i, &(program->blocks)) {
		struct block_t *bt = list_entry(i, struct block_t, list);
		if (bt->addr == addr)
			return bt;
	}
	return NULL;
}

struct block_t *block_split_new(struct program_t *program, unsigned long addr)
{
	struct list_head *i;
	struct block_t *bta;
	unsigned int oldb;
	list_for_each_prev(i, &(program->blocks)) {
		struct block_t *bt = list_entry(i, struct block_t, list);
		if (( addr >= bt->addr )&& (addr < (bt->addr+(unsigned long)bt->n_bytes) ) )
		{
			#if 0
			printf ("addr: %lx , %lx-%lx\n", addr,bt->addr,(bt->addr+(unsigned long)bt->n_bytes));
			#endif
			oldb = bt->n_bytes;

			bt->n_bytes = addr - bt->addr  ;
			bta = block_get_new ( program, addr );
			bta->n_bytes = oldb - bt->n_bytes ;

			bta->tnext = bt->tnext;
			bta->fnext = bt->fnext;
			bt->tnext = addr;
			bt->fnext = 0;

			#if 0
			printf ("OLD %d , new %d\n", bt->n_bytes, bta->n_bytes);
			printf ("addr: %lx , %lx-%lx, [%d]\n",
				addr, bt->addr,
				(bt->addr+(unsigned long)bt->n_bytes),
				(int) bta->n_bytes);
			#endif
			bta->bytes = (unsigned char*) malloc (bta -> n_bytes);
			memcpy ( bta->bytes,  bt->bytes + bt->n_bytes, bta -> n_bytes);
			return bta;
		}
	}
	return NULL;
}


struct block_t *block_get_new(struct program_t *program, unsigned long addr)
{
	struct block_t *bt;

	bt = block_get(program, addr);
	if (bt == NULL) {
		bt = block_new(addr);
		program->n_blocks++;
		list_add_tail(&(bt->list), &(program->blocks));
	}

	return bt;
}

struct block_t *block_split(struct program_t *program, struct block_t *block, unsigned int offset)
{
	int n_bytes = block->n_bytes;
	struct block_t *new;

	n_bytes = block->n_bytes - offset;
	if (n_bytes < 1) {
		eprintf("Invalid offset for block_split\n");
		return NULL;
	}

	/* allocate new block */
	new = block_get_new(program, block->addr + offset);
	new->n_bytes = n_bytes;
	new->bytes = (unsigned char *)malloc(n_bytes);
	memcpy(new->bytes, block->bytes+offset, n_bytes);
	new->tnext = block->tnext;
	new->fnext = block->tnext;

	/* realloc previous block */
	block->n_bytes = offset;
	block->bytes = realloc(block->bytes, offset);
	block->tnext = new->addr;
	block->fnext = 0;

	return new;
}

int block_set_framesize(struct program_t *program, unsigned long addr, int size)
{
	struct block_t *bt = block_get_new(program, addr);

	bt->framesize = size;

	return 1;
}

// label
int block_set_name(struct program_t *program, unsigned long addr, char *name)
{
	struct block_t *bt = block_get_new(program, addr);

	bt->name = estrdup(bt->name, name);

	return 1;
}

int block_add_call(struct program_t *program, unsigned long addr, unsigned long dest)
{
	struct block_t *bt = block_get_new(program, addr);
	struct xref_t *xr = xref_new(dest);

	bt->n_calls++;
	list_add_tail(&(xr->list), &(bt->calls));

	return 1;
}

int block_add_xref(struct program_t *program, unsigned long addr, unsigned long from)
{
	struct block_t *bt = block_get_new(program, addr);
	struct xref_t *xr = xref_new(addr); //(struct xref_t *)malloc(sizeof(struct xref_t));

	bt->n_xrefs++;
	list_add_tail(&(xr->list), &(bt->xrefs));

	return 1;
}

int block_set_comment(struct program_t *program, unsigned long addr, char *comment)
{
	struct list_head *i;
	struct block_t *b0;

	// XXX only supports one comment per block!! this is not ok
	// XXX we should store the comments on a separate linked list
	list_for_each_prev(i, &(program->blocks)) {
		b0 = list_entry(i, struct block_t, list);
		if (addr >= b0->addr && (addr < (b0->addr-b0->n_bytes))) {
			b0->comment = estrdup(b0->comment, comment);
			break;
		}
	}

	return 1;
}

int block_set_bytes(struct program_t *program, unsigned long addr, char *hexpairs)
{
	unsigned char *bytes = (unsigned char *)strdup(hexpairs);
	struct block_t *bt = block_get_new(program, addr);
	
	bt->n_bytes = hexstr2binstr((char *)bytes, bytes);
	bt->bytes   = (unsigned char *)malloc(bt->n_bytes);
	memcpy(bt->bytes, bytes, bt->n_bytes);
	bt->checksum = crc16(0, bt->bytes, bt->n_bytes);
	free(bytes);

	return 1;
}

int program_serialize(struct program_t *program, char *file)
{
	// TODO
	return 0;
}

int program_free(struct program_t *prg)
{
	printf("program_free: MEMORY LEAK!\n");
	return 0;
}

void program_reset(struct program_t *program)
{
	struct list_head *i;
	struct block_t *b0;

	list_for_each_prev(i, &(program->blocks)) {
		b0 = list_entry(i, struct block_t, list);
		b0->ignored = 0;
	}
}

struct program_t *program_new(char *file)
{
	struct program_t *program;
	/* initialize program_t structure */
	program = (struct program_t *)malloc(sizeof(struct program_t));
	program->n_blocks = 0;
	INIT_LIST_HEAD(&(program->blocks));
	if (file == NULL)
		return program;
	program->name = strdup(file);
	return program;
}

struct program_t *program_open(char *file)
{
	struct program_t *program;
	FILE *fd;
	int len;
	char buf[1024];
	char *ptr, *ptr2;
	off_t off;

	if (file != NULL) {
		fd = fopen(file, "r");
		if (fd == NULL) {
			printf("Cannot open %s\n", file);
			return NULL;
		}
	}

	program = program_new(file);

	while(!feof(fd)) {
		buf[0]='\0';
		fgets(buf, 1023, fd);
		if (buf[0]=='\0'||feof(fd)) break;
		buf[strlen(buf)-1]='\0';
		len = strlen(buf)-1;
		if (buf[len] == '\n' || buf[len] == '\r')
			buf[len]='\0';

		ptr = strchr(buf, '=');
		if (!ptr) continue;
		ptr[0]='\0'; ptr = ptr + 1;

		if (!strcmp(buf, "label")) {
			ptr2 = strchr(ptr, ' ');
			if (!ptr2) continue;
			ptr2[0]='\0'; ptr2=ptr2+1;
			off = get_offset(ptr);
			block_set_name(program, (unsigned long) off, ptr2);
		} else
		if (!strcmp(buf, "xref")) {
			ptr2 = strchr(ptr, ' ');
			if (!ptr2) continue;
			ptr2[0]='\0'; ptr2=ptr2+1;
			off = get_offset(ptr);
			block_add_xref(program, (unsigned long) off, (unsigned long)get_offset(ptr2));
		} else
		if (!strcmp(buf, "entry")) {
			ptr2 = strchr(ptr, ' ');
			if (!ptr2) continue;
			ptr2[0]='\0'; ptr2=ptr2+1;
			off = get_offset(ptr);
			program->entry = off; // XXX only one entry point ???
		} else
		if (!strcmp(buf, "framesize")) {
			ptr2 = strchr(ptr, ' ');
			if (!ptr2) continue;
			ptr2[0]='\0'; ptr2=ptr2+1;
			off = get_offset(ptr);
			block_set_framesize(program, (unsigned long) off, atoi(ptr2));
		} else
		if (!strcmp(buf, "bytes")) {
			ptr2 = strchr(ptr, ' ');
			if (!ptr2) continue;
			ptr2[0]='\0'; ptr2=ptr2+1;
			off = get_offset(ptr);
			block_set_bytes(program, (unsigned long) off, ptr2);
		} else
		if (!strcmp(buf, "comment")) {
			ptr2 = strchr(ptr, ' ');
			if (!ptr2) continue;
			ptr2[0]='\0'; ptr2=ptr2+1;
			off = get_offset(ptr);
			block_set_comment(program, (unsigned long) off, ptr2);
		}
	}
	fclose(fd);
	// XXX TODO: generate tnext/fnext entries !!

	return program;
}

void rdb_help()
{
	printf("Usage: R[?] [argument] (TODO)\n"
	"  R              ; list all RDBs loded in memory\n"
	"  R [rdb-file]   ; load rdb file into memory\n"
	"  R -[idx]       ; removes an rdb indexed\n"
	"  Rm [range]     ; performs a merge between selected rdbs\n"
	"  RG [num]       ; graph RDB number 'num'\n"
	"  Rd [a] [b]     ; rdb diff. should generate a new rdb\n");
}

