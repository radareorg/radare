/*
 * Copyright (C) 2007, 2008, 2009
 *       pancake <youterm.com>
 *       nibble <.ds@gmail.com>
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

#include "main.h"
#include "data.h"
#include "code.h"
#include "utils.h"
#include "rdb.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_FUN_SIZE 1023

/* code analyzer */
// int (*arch_aop)(ut64 addr, const u8 *bytes, struct aop_t *aop);

int section_is_x (ut64 addr)
{
	struct section_t *sec = section_get (addr);
	// XXX: rabin -rS is necesary here
	// We need to load the section information!!1
//	return 1;
	return (sec && sec->rwx & SECTION_X);
}

/* code lines */
struct reflines_t *code_lines_init(int linescall)
{
	struct reflines_t *list = (struct reflines_t*)malloc(sizeof(struct reflines_t));
	struct reflines_t *list2;
	int bar = PTRCAST (config_get("asm.linesout"));
	unsigned char *ptr = config.block;
	unsigned char *end = config.block + config.block_size;
	struct aop_t aop;
	int dt, sz, bsz = 0;
	int index = 0;
	ut64 seek = 0;
	int lines = -1;

	if (config.visual)
		lines = config.height;

	INIT_LIST_HEAD(&(list->list));

	if (arch_aop == NULL)
		return NULL;

	/* analyze code block */
	while( ptr < end ) {
		if (lines != -1 && --lines == 0)
			break;
		if (config.interrupted)
			break;
		dt = data_type(config.seek+bsz);
		if (dt != DATA_FUN && dt != DATA_CODE) {
			ut64 sz = data_size(config.seek+bsz);
			if (sz > 0) {
				ptr= ptr +sz;
				bsz=bsz+sz;
				continue;
			}
		}
		seek = config.seek + bsz;
		sz = arch_aop(seek, ptr, &aop);
		//sz = arch_aop(config.seek+bsz, ptr, &aop);
		if (sz < 1) {
			sz = 1;
		} else {
			/* store data */
			switch(aop.type) {
			case AOP_TYPE_CALL:
				if (!linescall)
					goto __next;
			case AOP_TYPE_CJMP:
			case AOP_TYPE_JMP:
				if (!bar) {
					/* skip outside lines */
					if (aop.jump > seek+config.block_size)
						goto __next;
					/* skip outside lines */
#if 0
//XXXX DO NOT ENABLE 
					if (aop.jump < seek-20) //config.block_size)
						goto __next;
#endif
				} else {
					if (aop.jump == 0)
						goto __next;
				}
				if (ptr-config.block <= config.cursor) {
					config.acursor = ptr-config.block;
					config.cursor_ptr = aop.jump;
				}
				list2 = (struct reflines_t*)malloc(sizeof(struct reflines_t));
				list2->from = seek;
				list2->to = aop.jump;
				list2->index = index++;
				list_add_tail(&(list2->list), &(list->list));
				break;
			}
		}
	__next:
		ptr = ptr + sz;
		bsz += sz;
	}
	
	return list;
}

void code_lines_free(struct list_head *list)
{
	// TODO: WTF!!1   What The Free!!
	free(list);
}

void code_lines_print(struct reflines_t *list, ut64 addr, int expand)
{
	struct list_head *pos;
	int foo = config_get_i("asm.linestyle");
	int bar = config_get_i("asm.nlines");
	int cow = 0;
	char ch = ' ';

	if (!list)
		return;

	cons_strcat(" ");
#define _h34d_ &(list->list)
	if (bar) {
		int count = 0;
		for (pos = foo?(_h34d_)->next:(_h34d_)->prev; pos != (_h34d_); pos = foo?pos->next:pos->prev)
			count++;
		for (;count<bar;count++)
			cons_strcat(" ");
	}
	for (pos = foo?(_h34d_)->next:(_h34d_)->prev; pos != (_h34d_); pos = foo?pos->next:pos->prev) {
		struct reflines_t *ref = list_entry(pos, struct reflines_t, list);
		if (config.interrupted)
			break;

		if (addr == ref->to)
			cow = 1;
		if (addr == ref->from)
			cow = 2;

		if (addr == ref->to) {
			if (!expand) {
				if (ref->from > ref->to)
					cons_strcat(".");
				else cons_strcat("`");
				ch = '-';
			} else ch = '.';
		} else
		if (addr == ref->from) {
			if (!expand) {
				if (ref->from > ref->to)
					cons_strcat("`");
				else cons_strcat(".");
				ch = '=';
			}
		} else {
			if (ref->from < ref->to) {
				/* down */
				C cons_strcat(C_YELLOW);
				if (addr > ref->from && addr < ref->to) {
					if (ch=='-'||ch=='=')
						cons_printf("%c",ch);
					else cons_strcat("|");
				} else
				if (!expand) {
					C {
						if (ch=='-')
							cons_printf(C_WHITE"-");
						else if (ch=='=')
							cons_printf(C_YELLOW"=");
						else cons_printf("%c",ch);
					} else cons_printf("%c",ch);
				}
			} else {
				C cons_strcat(C_WHITE);
				/* up */
				if (addr < ref->from && addr > ref->to) {
					if (ch=='-'||ch=='=')
						cons_printf("%c", ch);
					else // ^
						cons_strcat("|");
				} else cons_printf("%c",ch);
			}
		}
		if (config_get("asm.lineswide")) { // TODO: convert to integer ?
			switch(ch) {
			case '=':
			case '-':
				cons_printf("%c", ch);
				break;
			default:
				cons_strcat(" ");
				break;
			}
		}
	}

	if (expand) {
		cons_strcat("   ");
	} else
	if (cow==1) { 
		C cons_strcat(C_RED"-> "C_RESET);
		else cons_strcat("-> ");
	} else
	if (cow==2) {
		C cons_strcat(C_GREEN"=< "C_RESET);
		else cons_strcat("=< ");
	}
	else cons_strcat("   ");

	C cons_strcat(C_RESET);
}

/* XXX not working properly */
int code_analyze_r_split(struct program_t *prg, ut64 seek, int depth)
{
	struct aop_t aop;
	ut64 oseek = seek;
	ut64 tmp = config.seek;
	unsigned int sz = 0, ret;
	int bsz = 0;// block size
	char buf[4096]; // bytes of the code block
	unsigned char *ptr = (unsigned char *)&buf;
	int callblocks =(int) config_get_i("graph.callblocks");
	//int jmpblocks = (int) config_get_i("graph.jmpblocks");
    	int refblocks = (int) config_get_i("graph.refblocks");
	struct block_t *blf = NULL;
	
	if (arch_aop == NULL)
		return -1;
	// too deep! chop branch here!
	if (depth<=0)
		return 0;

	radare_seek(tmp, SEEK_SET);
	bsz = 0;
	config.seek = seek;
	radare_read(0);
	aop.eob = 0;

	ret = radare_read(0);

	blf = program_block_split_new (prg, config.seek);
	if ( blf != NULL ) {
		eprintf("Block split at address 0x%08llx\n",config.seek+bsz);
		aop.jump = blf->tnext;
		aop.fail = blf->fnext;
		aop.eob = 1;
	}		

	/* Walk for all bytes of current block */
	for(bsz = 0;(!aop.eob) && (bsz+4 <config.block_size); bsz+=sz) {
		if (config.interrupted)
			break;

		sz = arch_aop(config.seek+bsz, config.block+bsz, &aop);
		if (sz<=0) {
			break;
		}

		if ((aop.type == AOP_TYPE_JMP)
		|| (aop.type == AOP_TYPE_TRAP)
		|| (aop.type == AOP_TYPE_RET)) {
			//eprintf("GO DIE at 0x%08llx\n", seek);
			aop.eob = 1;
		} else
		if (aop.type == AOP_TYPE_CALL) {
			if (callblocks)
				aop.eob = 1;
			else aop.eob = 0;
		} else
			if (aop.type == AOP_TYPE_PUSH && aop.ref != 0) {
				/* TODO : add references */
				if (refblocks) {
					program_block_add_call(prg, oseek, aop.ref);
					aop.eob = 1;
					aop.jump = aop.ref;
					aop.fail = oseek+bsz+sz;
				}
			}
		memcpy(ptr+bsz, config.block+bsz, sz); // append bytes
	}
	config.seek = tmp;

	oseek = seek;

	/* walk childs */
	if (aop.jump)
		code_analyze_r_split(prg, aop.jump, depth-1);
	if (aop.fail)
		code_analyze_r_split(prg, aop.fail, depth-1);

	return 0;
}

int code_analyze_r_nosplit(struct program_t *prg, ut64 seek, int depth)
{
	struct aop_t aop;
	struct block_t *blk;
	ut64 oseek = seek;
	ut64 tmp = config.seek;
	unsigned int sz = 0, ret;
	int bsz = 0;// block size
	char buf[4096]; // bytes of the code block
	unsigned char *ptr = (unsigned char *)&buf;
	// TODO: make those vars global.. its slow to have config_* functions in a recursive fun
	int callblocks = (int)config_get_i("graph.callblocks");
	//int pushblocks = (int)config_get_i("anal.push");
	//int jmpblocks = (int) config_get_i("graph.jmpblocks");
	int refblocks = (int)config_get_i("graph.refblocks");

	if (arch_aop == NULL)
		return -1;
	if (depth<=0)
		return 0;
	if (config.interrupted)
		return 0;

	/* if already analyzed skip */
	if (program_block_get(prg,seek))
		return 0;

	radare_seek(tmp, SEEK_SET);
	bsz = 0;
	config.seek = seek;
	radare_read(0);
	aop.eob = 0;

	ret = radare_read(0);

	/* XXX bsz+4 fixes a segfault! (avoid reading outside the block...) */
	for(bsz = 0;(!aop.eob) && (bsz+4 <config.block_size); bsz+=sz) {
		if (config.interrupted)
			break;

		sz = arch_aop(config.seek+bsz, config.block+bsz, &aop);
		if (sz<=0) {
			// eprintf("Invalid opcode (%02x %02x)\n", config.block[0], config.block[1]);
			break;
		}

		if (aop.type == AOP_TYPE_JMP) {
			//eprintf("GO DIE at 0x%08llx\n", seek);
			aop.eob = 1;
		} else
		if (aop.type == AOP_TYPE_CALL) {
			program_block_add_call(prg, oseek, aop.jump);

			if (callblocks)
				aop.eob = 1;
			else aop.eob = 0;
		} else
		if (aop.type == AOP_TYPE_PUSH && aop.ref != 0) {
			/* TODO : add references */
			if (refblocks) {
				program_block_add_call(prg, oseek, aop.ref);
				aop.eob = 1;
				aop.jump = aop.ref;
				aop.fail = oseek+bsz+sz;
			}
			//block_add_ref(prg, oseek, aop.ref);
			//block_add_call(prg, oseek, aop.ref);
		}
		memcpy(ptr+bsz, config.block+bsz, sz); // append bytes
	}
	config.seek = tmp;

	blk = program_block_get_new(prg, oseek);

	blk->bytes = (unsigned char *)malloc(bsz+1);
	if (blk->bytes == NULL) {
		eprintf("analyze.c: Cannot allocate\n");
		return 0;
	}
	blk->n_bytes = bsz;
	memcpy(blk->bytes, buf, bsz);
	blk->tnext = aop.jump;
	blk->fnext = aop.fail;
	oseek = seek;

	blk->type = BLK_TYPE_HEAD;
	if (aop.jump && !aop.fail)
		blk->type = BLK_TYPE_BODY;
	else
	if (aop.jump && aop.fail)
		blk->type = BLK_TYPE_BODY;
	else
	if (aop.type == AOP_TYPE_RET)
		blk->type = BLK_TYPE_LAST;

	/* walk childs */
	if (blk->tnext)
		code_analyze_r_nosplit(prg, blk->tnext, depth-1);
	if (blk->fnext)
		code_analyze_r_nosplit(prg, blk->fnext, depth-1);
	return 0;
}

void analyze_spcc(const char *name)
{
	char *ptr;
	char buf[1024];
	const char *path = config_get("dir.spcc");

	if (path)
		setenv("SPCCPATH", path, 1);
	if (name[0]=='\0') {
		radare_cmd_raw("!!rsc spcc-fe list", 0);
		return;
	}
	if (strchr(name, '?')) {
		cons_printf("Usage: as [?][-][file]\n"
		"Analyze structure using the spcc descriptor\n"
		"  > as name   :  create/show structure\n"
		"  > as -name  :  edit structure\n"
		"  > as ?      :  list all spcc in dir.spcc\n");
		return;
	}
	buf[0]='\0';
	ptr = strchr(name, '-');
	if (ptr) sprintf(buf, "!!${EDITOR} %s/%s.spcc", path, ptr+1);
	else sprintf(buf, "!!rsc spcc-fe %s ${BLOCK} 0", name);
	radare_cmd_raw(buf, 0);
}

/* CALLBACK defined with graph.split which is false by default */
int (*code_analyze_r)(struct program_t *prg, ut64 seek, int depth) = &code_analyze_r_nosplit;

struct program_t *code_analyze(ut64 seek, int depth)
{
	ut64 bsize = config.block_size;
	struct program_t *prg = program_new(NULL);

	if (prg == NULL)
		return NULL;

	prg->entry = config.seek;

	radare_set_block_size_i(4096); // max function size = 5000
	radare_read(0);

	radare_controlc();

	if (depth>30) depth=30;
	if (config_get("graph.split")) {
		code_analyze_r_nosplit(prg, seek, --depth);
		code_analyze_r_split(prg, seek, --depth);
	} else code_analyze_r_nosplit(prg, seek, --depth);

	// TODO: construct xrefs from tnext/fnext info
	radare_controlc_end();
	radare_set_block_size_i(bsize);

	return prg;
}

/* memory analyzer */

#if 0
Memory analysis
===============
read a data block and identify it (new print mode)
The identify will consist on a dword analysis of the contents. Useful to read the stack

  0000 0000 -> probably a null pointer or a initialized integer variable
  0804 8xxx -> program code. probably a return address
  0BFx xxxx -> stack area, probably a local variable

resolve data using the flags and so
Recursively harvest the memory pointers to get the values of the data.

TODO: use maps here! must be mixed with flags and so

#endif

int radare_analyze(ut64 seek, int size, int depth, int rad)
{
	char cmd[1024];
	char str[1024];
	u8 word[128];
	ut64 tmp = config.seek;
	ut32 num, nume; // little endian
	int count=0;
	int i;
	int str_i=0;
	int word_i=0;
	int lastnull = 0;
	int v = config.verbose;

	config.verbose = 0;

	if (depth<0)
		return 0;

	config.seek = seek;
	radare_read(0);

	if ((size*4)>config.block_size)
		size = config.block_size/4;

	if (size<0) {
		count = 1;
		size = 64;
	}
	size<<=2;
	for(i=0;i<size;i++) {
		if (config.interrupted)
			break;

		if (is_printable(config.block[i])) {
			if(word_i<4) word[word_i++] = config.block[i];
			str[str_i++] = config.block[i];
			continue;
		}

		if (str_i>2) {
			str[str_i] = '\0';
			if (rad) {
				ut64 addr = (ut64)(seek+i-str_i);
				cons_printf("Cs %d @ 0x%08llx\n", strlen(str)+1, addr);
				flag_filter_name(str);
				cons_printf("f str.%s @ 0x%08llx\n", str, addr);
				//cons_printf("; TODO (if exists) f str_%s\n", str);
			} else {
				print_addr((ut64)(seek+i-str_i));
				cons_strcat("   ");
				C	cons_printf("string "C_BYELLOW"\"%s\""C_RESET"\n", str);
				else	cons_printf("string \"%s\"\n", str);
			}
			word_i = 0;
			str_i=0;
			continue;
		}

		str_i = 0;
		word[word_i++] = config.block[i];
		if (word_i==4) {
			word_i = 0;

			// compose big endian number (32bit)
			num  = word[3];
			num |= word[2]<<8;
			num |= word[1]<<16;
			num |= word[0]<<24;

			// compose little endian (32bit)
			nume  = word[0];
			nume |= word[1]<<8;
			nume |= word[2]<<16;
			nume |= word[3]<<24;

			if (num == 0) {
				if (lastnull++ == 0) {
					if (rad) {
					}else {
						cons_strcat("   ");
						print_addr(seek+i-3);
						C cons_printf(C_YELLOW"(NULL)"C_RESET"\n");
						else cons_printf("(NULL)\n");
					}
				}
			} else if (num == -1) {
				/* ignore -1 */
				//print_addr(seek+i-3);
				//cons_printf("0xffffffff (-1)\n");
			} else {
				if (rad) {
					ut32 n = (config.endian)?num:nume;
					str[0]='\0';
					string_flag_offset(NULL, str, (ut64)n, 0);
					if (!strnull(str)) {
						/* reference by pointer */
						cons_printf("Cx 0x%08llx @ 0x%08llx ; %s\n", (ut64)(seek+i-3), (ut64)n, str);
					} else
					if (n == (ut32)seek)
						cons_printf(" ;  (self pointer)\n");
					else radare_analyze(n, size, --depth, rad);
				} else {
					if (lastnull>1)
						cons_printf("(last null repeated %d times)\n", lastnull);
					lastnull = 0;
				//	for(j=config_get_i("anal.depth"); j>depth;j--)
				//		cons_strcat("   ");
					print_addr(seek+i-3);
					C {
						cons_printf(C_TURQOISE);
						if (config.endian)
						cons_printf("int be="C_YELLOW"0x%08x"C_RESET" le=0x%08x ",
							num, nume);
						else
						cons_printf("int be=0x%08x le="C_YELLOW"0x%08x"C_RESET" ",
							num, nume);
					} else cons_printf("int be=0x%08x le=0x%08x ",
							num, nume);
					if (num<0xffff)
						cons_printf("(be= %d )", num);
					if (nume<0xffff)
						cons_printf(", (le= %d ) ", nume);
					if (num>-0xfffff && num<0xfffff)
						cons_printf("(%d)\n", num);
					else
					if (nume>-0xfffff && nume<0xfffff)
						cons_printf("(%d)\n", nume);
					else {
						ut32 n = (config.endian)?num:nume;
						C cons_printf(C_TURQOISE);
						sprintf(cmd, ":fd @0x%08x", n);
						radare_cmd(cmd, 0);

						if (n == (ut32)seek)
							cons_printf("  (self pointer)\n");
						else radare_analyze(n, size, --depth, rad);

						config.seek = seek;
						radare_read(0);
						C cons_printf(C_RESET);
					}
				}
			}
			if (count)
				break;
		}
	}

	/* restore */
	config.seek = tmp;
	radare_read(0);
	config.verbose = v;

	return 0;
}

// XXX move to code.h
enum {
	VAR_TYPE_NONE=0,
	VAR_TYPE_ARG=1,
	VAR_TYPE_LOCAL=2
};

struct vars_t {
	int type;
	int delta;
	int count;
}; 

#define VAR_MAX 256
struct vars_t vars[VAR_MAX];

void analyze_var_reset()
{
	memset(&vars, '\0', sizeof(vars));
}

int analyze_var_add(int type, int delta)
{
	int i, hole = -1;
	for(i=0;i<VAR_MAX;i++) {
		if (vars[i].type == type && vars[i].delta == delta) {
			vars[i].count++;
			return 0;
		} else
		if (vars[i].type==VAR_TYPE_NONE && hole==-1) {
			hole = i;
		}
	}
	if (hole==-1) {
		eprintf("analyze.c: No space left in var pool\n");
		return -1;
	}
	vars[hole].type  = type;
	vars[hole].delta = delta;
	return 1;
}

int analyze_var_get(int type)
{
	int i, ctr = 0;
	for(i=0;i<VAR_MAX;i++) {
		if (vars[i].type == type)
			ctr++;
	}
	return ctr;
}

void analyze_progress(int _o, int _x, int _p, int _v)
{
	char buf[80];
	int i, j = 0;
	//static int refresh = 0;
	static int o=0,x=0,p=0,v=0;
	static int ch = '/';
	static int lastlen = 0;
	int tmp;
	o+=_o; x+=_x; p+=_p; v+=_v;
	/* TODO: change this value depending on delta times */
	//if (refresh++%20) return 0;
	tmp = p/12;
	p   = p%12;
	tmp = v/12;
	v   = v%12;
	for(i=0;i<tmp;i++) buf[j++]=')';
	for(i=0;i<p;i++)   buf[j++]='=';
	for(i=0;i<v;i++)   buf[j++]='-';
	for(i=0;i<tmp;i++) buf[j++]='>';
	buf[j]='\0';
	eprintf("                                             "
	        "\r  [%c] %02x:%02x:%02x:%02x %s", ch, o, x, p, v, buf);
	lastlen = p+v;
	eprintf("\r");
	switch(ch) {
	case '/': ch = '-'; break;
	case '-': ch = '\\'; break;
	case '\\': ch = '|'; break;
	case '|': ch = '/'; break;
	}
}

int analysis_interrupted = 0;

int analyze_function(ut64 from, int recursive, int report)
{
	struct aop_t aop;
	struct list_head *head;
	struct block_t *b0;
	struct program_t *prg;
	int ret;
	char buf[4096];
	//static int interrupted = 0;
	/*--*/
	ut8 *bytes;
	//ut64 from = config.vaddr + config.seek;
	ut64 seek = from; // to place comments
	ut64 end  = 0;
	int i, inc = 0;
	ut64 to;
	ut64 funsize;
	int len =0;
	int ref, count;
	int ncalls = 0;
	int nrefs = 0;
	int framesize = 0;
	int nblocks = 0;
	char tmpstr[16], fszstr[256];
ut64 addr= seek;

	if (recursive <0)
		return -1;
	if (config.interrupted)
		return -1;

	from += config.vaddr-config.paddr;

	if (arch_aop == NULL)
		return -1;
	fszstr[0]='\0';
#if 0
	struct data_t *d;
	d = data_get(config.vaddr+config.seek);
	if (d && d->type == DATA_FUN) {
		//cons_printf("; already analyzed\n");
	//	return 0;
	}
#endif
	/* Analyze function */
	/* XXX restore values later.. */
	config_set("graph.jmpblocks", "true");
	config_set("graph.callblocks", "false");
	// XXX cfg.bsize affects here!! WARN WARN WARN!

	analyze_var_reset(); // ??? control recursivity here ??

	//prg = code_analyze(config.vaddr + config.seek, 1024);
	prg = code_analyze(seek, config_get_i("graph.depth")); //1024);
	list_add_tail(&prg->list, &config.rdbs);

	list_for_each(head, &(prg->blocks)) {
		b0 = list_entry(head, struct block_t, list);
		//if ((b0->type == BLK_TYPE_HEAD)
		//if ((b0->type == BLK_TYPE_LAST)
		//|| (b0->type == BLK_TYPE_FOOT))
		if ((b0->addr + b0->n_bytes) > end)
			end = (b0->addr + b0->n_bytes - 1);
		nblocks++;
	}
	to = end;
	len = (int)(1+to-seek); //from;
	if (len<0)
		return -1;

	bytes = (char *)malloc(len);
	if (bytes == NULL)
		return -1;

	if (len > MAX_FUN_SIZE) {
		//D eprintf("o");
		D analyze_progress(1,0,0,0);
		len = MAX_FUN_SIZE;
	}

	ret = radare_read_at(seek, bytes, len);
	if (ret <0) {
		//eprintf("Invalid read at 0x%08llx len=%lld\n", from,len);
		// eprintf("x");
		D analyze_progress(0,1,0,0);
		return -1;
	}

	switch(report) {
	case 2:
		cons_printf("f -fun.%08llx\n", from);
		//cons_printf("fu -fun.%08llx @ 0x%08llx\n", from, from); // XXX should be fu?!? do not works :(
		cons_printf("CF-0 @ 0x%08llx\n", from); // XXX can be recursive
		break;
	case 1:
		buf[0]='\0';
		string_flag_offset(NULL, buf, from, 0);
		cons_printf("offset = 0x%08llx\n", from);
		cons_printf("label = %s\n", buf);
		cons_printf("size = %lld\n", to-seek);
		cons_printf("blocks = %lld\n", nblocks);
		cons_printf("bytes = ");
		for(i=0;i<len;i++) 
			cons_printf("%02x", bytes[i]);
		cons_newline();
	case 0:
		cons_strcat("fs functions\n");
		cons_printf("; from = 0x%08llx\n", from);
		cons_printf("; to   = 0x%08llx\n", end);
		funsize = to-seek+1;
		if (funsize > 8096) { /* OOPS TOO BIG FUN */
			D analyze_progress(0,1,0,0);
		} else {
			cons_printf("fu fun.%08llx @ 0x%08llx\n", from, from); // XXX should be fu?!? do not works :(
			cons_printf("CF %lld @ 0x%08llx\n", to-seek+1, from); // XXX can be recursive
		}
	}
	//D eprintf(".");
	D analyze_progress(0,0,1,0);

	for(;seek < to; seek+=inc) {
		ut64 delta = seek+config.vaddr-from;
		if (delta >= len) {
			//eprintf("analyze_function: oob %lld > %lld\n", delta, len);
			break;
		}
		inc = arch_aop(seek+config.vaddr, bytes+delta, &aop);
		if (inc<1) {
			inc = 1;
			break;
		}
		switch(aop.type) {
		case AOP_TYPE_PUSH:
			analyze_function(aop.ref-config.vaddr, recursive-1, report);
			break;
		case AOP_TYPE_CALL:
			switch(report) {
			case 2:
				cons_printf("Cx -0x%08llx @ 0x%08llx\n", aop.jump, seek+config.vaddr);
				break;
			case 0:
				buf[0]='\0';
				string_flag_offset(NULL, buf, aop.jump, 0);
				// if resolved as sym_ add its call
				cons_printf("Cx 0x%08llx @ 0x%08llx ; %s\n", aop.jump, seek+config.vaddr, buf);
			}
			analyze_function(aop.jump-config.vaddr, recursive-1, report);
			ncalls++;
			break;
		case AOP_TYPE_SWI:
			switch(report) {
			case 2: cons_printf("CC -syscall %s @ 0x%08llx\n", "(todo)", seek);
				break;
			case 0: cons_printf("CC syscall %s @ 0x%08llx\n", "(todo)", seek);
				break;
			}
			break;
		default:
			if (aop.ref != 0)
			switch(aop.type) {
			case AOP_TYPE_PUSH:
			case AOP_TYPE_STORE:
			case AOP_TYPE_LOAD:
				switch(report) {
				case 2:
					cons_printf("CX -0x%08llx @ 0x%08llx ; %s\n",
						aop.ref+config.vaddr, seek , buf);
					break;
				case 0:
					buf[0]='\0';
					string_flag_offset(NULL, buf, aop.jump, 0);
					// if resolved as sym_ add its call
					cons_printf("CX 0x%08llx @ 0x%08llx ; %s\n",
						seek, (ut64)aop.ref, buf);
				}
				nrefs++;
			}
		}
		ref = (int)aop.value;
		if (ref==0)
			ref = aop.ref;
		switch(aop.stackop) {
		case AOP_STACK_LOCAL_SET:
			if (report == 2) {
				sprintf(buf, "CC -* @ 0x%08llx\n", seek);
				cons_strcat(buf);
			} else
			if (!report) {
				if (ref<0)
					sprintf(buf, "CC Set var%d @ 0x%08llx\n", -ref, seek);
				else sprintf(buf, "CC Set var%d @ 0x%08llx\n", ref, seek);
				cons_strcat(buf);
				cons_printf("CFvs %d @ 0x%08llx\n", ref, seek);
			}
			if (ref<0) analyze_var_add(VAR_TYPE_ARG, -ref);
			else analyze_var_add(VAR_TYPE_LOCAL, ref);
			break;
		case AOP_STACK_ARG_SET:
			if (report == 2) {
				sprintf(buf, "CC -Set arg%d @ 0x%08llx\n", ref, seek);
				cons_strcat(buf);
			} else
			if (!report) {
				sprintf(buf, "CC Set arg%d @ 0x%08llx\n", ref, seek);
				cons_strcat(buf);
			}
			analyze_var_add(VAR_TYPE_ARG, ref);
			break;
		case AOP_STACK_ARG_GET:
			if (report == 2) {
				sprintf(buf, "CC -* @ 0x%08llx\n", seek);
				cons_strcat(buf);
			} else
			if (!report) {
				char buf[1024];
				if (ref<0) {
					cons_printf("CFvg %d @ 0x%08llx\n", -ref, seek);
					sprintf(buf, "CC Get var%d @ 0x%08llx\n", -ref, seek);
				} else {
					sprintf(buf, "CC Get arg%d @ 0x%08llx\n", ref, seek);
					cons_printf("CFag %d @ 0x%08llx\n", ref, seek);
				}
				cons_strcat(buf);
				//sprintf(buf, "CCC Get arg%d @ 0x%08llx\n", ref, seek);
				//cons_strcat(buf);
			}
			analyze_var_add(VAR_TYPE_ARG, ref);
			break;
		case AOP_STACK_LOCAL_GET:
			if (report == 2) {
				sprintf(buf, "CC -* @ 0x%08llx\n", seek);
				cons_strcat(buf);
				//cons_printf("CFvg %d @ 0x%08llx\n", ref, seek);
			} else
			if (!report) {
				if (ref<0) {
					sprintf(buf, "CC Get arg%d @ 0x%08llx\n", -ref, seek);
					cons_printf("CFag %d @ 0x%08llx\n", -ref, seek);
				} else {
					sprintf(buf, "CC Get var%d @ 0x%08llx\n", ref, seek);
					cons_printf("CFvg %d @ 0x%08llx\n", ref, seek);
				}
				cons_strcat(buf);
			}
			if (ref<0) analyze_var_add(VAR_TYPE_ARG, -ref);
			else analyze_var_add(VAR_TYPE_LOCAL, ref);
			break;
		case AOP_STACK_INCSTACK:
			// XXX ugly output
			switch(report) {
			case 0:
				if (ref<0)
					sprintf(buf, "CC Stack size %d @ 0x%08llx\n", (int)ref, seek);
				else sprintf(buf, "CC Stack size +%d @ 0x%08llx\n", (int)ref, seek);
				cons_strcat(buf);
				framesize += aop.value;
				sprintf(tmpstr, "%c%d",fszstr[0]?',':' ', framesize);
				strcat(fszstr, tmpstr); // XXX control overflow
				break;
			case 2:
				if (ref<0)
					sprintf(buf, "CC -Stack size %d @ 0x%08llx\n", (int)ref, seek);
				else sprintf(buf, "CC -Stack size +%d @ 0x%08llx\n", (int)ref, seek);
				cons_strcat(buf);
				framesize += aop.value;
				sprintf(tmpstr, "%c%d",fszstr[0]?',':' ', framesize);
				strcat(fszstr, tmpstr); // XXX control overflow
				break;
			}
			break;
		}

		/* recursivity */
		if (recursive) {
			switch(aop.type) {
			case AOP_TYPE_RET:
			case AOP_TYPE_TRAP:
				seek = to;
				break;
	#if 1
			case AOP_TYPE_JMP: // considered as new function
				radare_seek(aop.jump, SEEK_SET);
				//analyze_function(1);
				analyze_function(seek-config.vaddr, recursive-1, report);
				break;
	#endif
			case AOP_TYPE_CALL: // considered as new function
				radare_seek(aop.jump, SEEK_SET);
				analyze_function(seek-config.vaddr, recursive-1, report);
				break;
			}
		}
	}
	free(bytes);

	switch(report) {
	case 0:
		if (fszstr[0])
			cons_printf("CC framesize = %s @ 0x%08llx\n", fszstr, from);
		count = analyze_var_get(VAR_TYPE_ARG);
		if (count>0) cons_printf("CC args = %d @ 0x%08llx\n", count, from);
		count = analyze_var_get(VAR_TYPE_LOCAL);
		if (count>0) cons_printf("CC vars = %d @ 0x%08llx\n", count, from);
		if (nrefs>0) cons_printf("CC drefs = %d @ 0x%08llx\n", nrefs);
		cons_printf("fs *\n");
		break;
	case 1:
		cons_printf("framesize = %d\n", framesize);
		cons_printf("ncalls = %d\n", ncalls);
		cons_printf("drefs = %d\n", nrefs);
		cons_printf("xrefs = %d\n", data_xrefs_at(from));
		cons_printf("args = %d\n", analyze_var_get(VAR_TYPE_ARG));
		cons_printf("vars = %d\n", analyze_var_get(VAR_TYPE_LOCAL));
		break;
	case 2:
		cons_printf("CC -framesize = %s @ 0x%08llx\n", fszstr, from);
		cons_printf("CC -args = %d @ 0x%08llx\n", analyze_var_get(VAR_TYPE_ARG), from);
		cons_printf("CC -vars = %d @ 0x%08llx\n", analyze_var_get(VAR_TYPE_LOCAL), from);
		cons_printf("CC -drefs = %d @ 0x%08llx\n", nrefs);
		cons_printf("fs *\n");
	}

	return 0;
}

void analyze_preludes (char *input) {
	char cmd[1024];
	char *prelude;
	long len;
	const char *sfrom, *sto, *cmdhit;

	switch(config.arch) {
	case ARCH_X86: // X86-64 is implicit here
		prelude = "5589e5";
		break;
	default:
		eprintf ("Analyze preludes (ap) not supported for this architecture\n");
		return;
	}
	sfrom = config_get("search.from");
	sto   = config_get("search.to");
	cmdhit= config_get("cmd.hit");

	len = get_math (input);
	if (len <1) {
		len = config.size;
		if (len <0) {
			eprintf ("Dunno filesize..gimme some ranges\n");
			return;
		}
	}
	/* go search */
	config_set_i("search.from", config.seek);
	config_set_i("search.to", config.seek+ len);
	config_set("cmd.hit", ".af*");
	sprintf(cmd, "/x %s", prelude);
	radare_cmd(cmd, 0);

	/* restore */
	config_set("search.from", sfrom);
	config_set("search.to", sto);
	config_set("cmd.hit", cmdhit);
	radare_cmd("f-hit*", 0);
}
