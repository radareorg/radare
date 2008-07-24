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

#include "main.h"
#include "code.h"
#include "utils.h"
#include "rdb.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

/* code analyzer */
int (*arch_aop)(u64 addr, const u8 *bytes, struct aop_t *aop);

/* code lines */
struct reflines_t *code_lines_init()
{
	struct reflines_t *list = (struct reflines_t*)malloc(sizeof(struct reflines_t));
	struct reflines_t *list2;
	int bar = (int)config_get("asm.linesout");
	unsigned char *ptr = config.block;
	unsigned char *end = config.block + config.block_size;
	struct aop_t aop;
	int sz, bsz = 0;
	int index = 0;
	u64 seek = 0;

	INIT_LIST_HEAD(&(list->list));

	/* analyze code block */
	while( ptr < end ) {
		if (config.interrupted)
			break;
		{
			int dt = data_type(config.baddr+config.seek+bsz);
			if (dt) {
				u64 sz = data_size(config.baddr+config.seek+bsz);
				if (sz > 0) {
					ptr= ptr +sz;
					bsz=bsz+sz;
					continue;
				}
			}
		}
		seek = config.baddr + config.seek + bsz;
		sz = arch_aop(seek, ptr, &aop);
		//sz = arch_aop(config.seek+bsz, ptr, &aop);
		if (sz <1) {
			sz = 1;
		} else {
			/* store data */
			switch(aop.stackop) {
			case AOP_TYPE_LOCAL_SET:
				{
				char buf[1024];
				int ref = (int)aop.ref;
				if (ref<0)
					sprintf(buf, "CC Set arg%d@ 0x%08llx\n", -ref, seek);
				else
					sprintf(buf, "CC Set var%d@ 0x%08llx\n", ref, seek);
				radare_cmd(buf, 0);
				radare_seek(config.seek, SEEK_SET);
				radare_read(0);
				}
				break;
			case AOP_TYPE_ARG_SET:
				{
				int ref = (int)aop.ref;
				char buf[1024];
				sprintf(buf, "CC Set arg%d@ 0x%08llx\n", ref, seek);
				radare_cmd(buf, 0);
				radare_seek(config.seek, SEEK_SET);
				radare_read(0);
				}
				break;
			case AOP_TYPE_ARG_GET:
				{
				char buf[1024];
				int ref = (int)aop.ref;
				sprintf(buf, "CC Get arg%d@ 0x%08llx\n", ref, seek);
				radare_cmd(buf, 0);
				radare_seek(config.seek, SEEK_SET);
				radare_read(0);
				}
				break;
			case AOP_TYPE_LOCAL_GET:
				{
				char buf[1024];
				int ref = (int)aop.ref;
				if (ref<0)
					sprintf(buf, "CC Get arg%d@ 0x%08llx\n", -ref, seek);
				else
					sprintf(buf, "CC Get var%d@ 0x%08llx\n", ref, seek);
				radare_cmd(buf, 0);
				radare_seek(config.seek, SEEK_SET);
				radare_read(0);
				}
				break;
			case AOP_TYPE_INCSTACK:
				{
				char buf[1024];
				sprintf(buf, "CC Stack size +%d@ 0x%08llx\n", (int)aop.ref, seek);
				radare_cmd(buf, 0);
				radare_seek(config.seek, SEEK_SET);
				radare_read(0);
				}
				break;
			}
			switch(aop.type) {
			case AOP_TYPE_CALL:
			case AOP_TYPE_CJMP:
			case AOP_TYPE_JMP:
				if (!bar) {
					/* skip outside lines */
					if (aop.jump > seek+config.block_size)
						goto __next;
					/* skip outside lines */
					if (aop.jump < seek-30)
						goto __next;
				} else
					if (aop.jump == 0)
						goto __next;
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
	// WTF!!1   What The Free!!
	free(list);
}

void code_lines_print(struct reflines_t *list, u64 addr, int expand)
{
	struct list_head *pos;
	int foo = config_get_i("asm.linestyle");
	int bar = config_get_i("asm.nlines");
	int cow = 0;
	char ch = ' ';

	if (!list)
		return;

	cons_strcat(" ");
#define head &(list->list)
	if (bar) {
		int count = 0;
		for (pos = foo?(head)->next:(head)->prev; pos != (head); pos = foo?pos->next:pos->prev)
			count++;
		for (;count<bar;count++)
			cons_strcat(" ");
	}
	for (pos = foo?(head)->next:(head)->prev; pos != (head); pos = foo?pos->next:pos->prev) {
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
				else
					cons_strcat("`");
				ch = '-';
			} else
				ch = '.';
		} else
		if (addr == ref->from) {
			if (!expand) {
				if (ref->from > ref->to)
					cons_strcat("`");
				else
					cons_strcat(".");
				ch = '=';
			}
		} else {
			if (ref->from < ref->to) {
				/* down */
				C cons_strcat(C_YELLOW);
				if (addr > ref->from && addr < ref->to) {
					if (ch=='-'||ch=='=')
						cons_printf("%c",ch);
					else
						cons_strcat("|");
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
				} else {
					cons_printf("%c",ch);
				}
			}
		}
		if (config_get("asm.lineswide")) { // TODO: convert to integer ?
			switch(ch) {
			case '=':
			case '-':
				cons_printf("%c", ch);
				break;
			default:
				cons_printf(" ");
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

	C cons_printf(C_RESET);
}

/* code analyze */

int code_analyze_r_split(struct program_t *prg, u64 seek, int depth)
{
	struct aop_t aop;
	struct block_t *blk;
	u64 oseek = seek;
	u64 tmp = config.seek;
	unsigned int sz = 0, ret;
	int bsz = 0;// block size
	char buf[4096]; // bytes of the code block
	unsigned char *ptr = (unsigned char *)&buf;
	int callblocks =(int) config_get("graph.callblocks");
	int jmpblocks = (int) config_get("graph.jmpblocks");
	struct block_t *blf = NULL;
	
	// too deep! chop branch here!
	if (depth<=0)
		return 0;
	if (config.interrupted)
		return 0;

	/* if already analyzed skip */
	if (block_get(prg,seek))
		return 0;

	radare_seek(tmp, SEEK_SET);
	bsz = 0;
	config.seek = seek;
	radare_read(0);
	aop.eob = 0;

	ret = radare_read(0);

	for(bsz = 0;(!aop.eob) && (bsz <config.block_size); bsz+=sz) {
		/// Miro si l'adreca on soc es base d'algun bloc
		blf = block_get ( prg , config.seek+bsz );

		sz = arch_aop(config.seek+bsz, config.block+bsz, &aop);
		if (sz<=0) {
			//eprintf("Invalid opcode (%02x %02x)\n", config.block[0], config.block[1]);
			break;
		}
		
		if ( blf != NULL )
		{	
			//printf ("Address %llx already analed\n", config.seek+bsz );
			aop.eob = 1;
			aop.jump = blf->tnext; //config.seek+bsz;
			aop.fail = blf->fnext;
//printf("%llx, %llx\n", aop.fail, aop.jump);
	//bsz+=sz;
			break;
		}

		blf = block_split_new ( prg, config.seek+bsz  );
		if ( blf != NULL )
		{		
			//printf ("--Address %llx already analed\n", config.seek+bsz );
//printf("-- %llx, %llx\n", aop.fail, aop.jump);
			
			bsz = blf->n_bytes;
			aop.eob = 1;
		if (blf->tnext)
			aop.jump = blf->tnext;
		if (blf->fnext)
			aop.fail = blf->fnext;
			break;
		}		

		if (config.interrupted)
			break;

		if (!callblocks && aop.type == AOP_TYPE_CALL) {
			block_add_call(prg, oseek, aop.jump);
                	if (callblocks)
				aop.eob = 1;
			else aop.eob = 0;
		}
		if (!jmpblocks && (aop.type == AOP_TYPE_JMP || aop.type == AOP_TYPE_CJMP))
			aop.eob = 0;

		switch(aop.type) {
		case AOP_TYPE_JMP:
		case AOP_TYPE_CJMP:
		case AOP_TYPE_CALL:
			block_add_call(prg, oseek, aop.jump);
			break;
		case AOP_TYPE_PUSH:
			/* TODO : add references */
			if (config_get("graph.refblocks"))
				block_add_call(prg, oseek, aop.ref);
			break;
		}

		memcpy(ptr+bsz, config.block+bsz, sz); // append bytes
	}
	config.seek = tmp;

	blk = block_get_new(prg, oseek);

	bsz+=sz;
	blk->bytes = (unsigned char *)malloc(bsz);
	blk->n_bytes = bsz;
	memcpy(blk->bytes, buf, bsz);
	blk->tnext = aop.jump;
	blk->fnext = aop.fail;
	

		blk->type = BLK_TYPE_HEAD;
	if (aop.jump && !aop.fail)
		blk->type = BLK_TYPE_BODY;
	else
	if (aop.jump && aop.fail)
		blk->type = BLK_TYPE_BODY;
	else
	if (aop.type == AOP_TYPE_RET)
		blk->type = BLK_TYPE_LAST;

	oseek = seek;

	/* walk childs */
	if (blk->tnext && (blf == NULL) )
		code_analyze_r_split(prg, blk->tnext, depth-1);
	if (blk->fnext  )
		code_analyze_r_split(prg, blk->fnext, depth-1);
	bsz = 0;

	depth--;

	return 0;
}

int code_analyze_r_nosplit(struct program_t *prg, u64 seek, int depth)
{
        struct aop_t aop;
        struct block_t *blk;
        u64 oseek = seek;
        off_t tmp = config.seek;
        unsigned int sz = 0, ret;
        int bsz = 0;// block size
        char buf[4096]; // bytes of the code block
        unsigned char *ptr = (unsigned char *)&buf;
        int callblocks = (int)config_get("graph.callblocks");
        int refblocks = (int)config_get("graph.refblocks");

        if (depth<=0)
                return 0;
        if (config.interrupted)
                return 0;

        /* if already analyzed skip */
        if (block_get(prg,seek))
                return 0;

        radare_seek(tmp, SEEK_SET);
        bsz = 0;
        config.seek = seek;
        radare_read(0);
        aop.eob = 0;

        ret = radare_read(0);

        for(bsz = 0;(!aop.eob) && (bsz <config.block_size); bsz+=sz) {
                if (config.interrupted)
                        break;
                sz = arch_aop(config.seek+bsz, config.block+bsz, &aop);
                if (sz<=0) {
                        eprintf("Invalid opcode (%02x %02x)\n", config.block[0], config.block[1]);
                        break;
                }

		if (aop.type == AOP_TYPE_CALL) {
			block_add_call(prg, oseek, aop.jump);
                	if (callblocks)
				aop.eob = 1;
			else aop.eob = 0;
                } else
		if (aop.type == AOP_TYPE_PUSH && aop.ref !=0) {
			/* TODO : add references */
			if (refblocks) {
				block_add_call(prg, oseek, aop.ref);
				aop.eob = 1;
				aop.jump = aop.ref;
				aop.fail = oseek+bsz+sz;
			}
		//		block_add_ref(prg, oseek, aop.ref);
				//block_add_call(prg, oseek, aop.ref);
		}

                memcpy(ptr+bsz, config.block+bsz, sz); // append bytes
        }
	bsz-=1;
        config.seek = tmp;

        blk = block_get_new(prg, oseek);

        blk->bytes = (unsigned char *)malloc(bsz);
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
                code_analyze_r_nosplit(prg, (unsigned long)blk->tnext, depth-1);
        if (blk->fnext)
                code_analyze_r_nosplit(prg, (unsigned long)blk->fnext, depth-1);
        bsz = 0;

        depth--;

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
		cons_printf("Usage: as [?][-][file]\n");
		cons_printf("Analyze structure using the spcc descriptor\n");
		cons_printf("  > as name   :  create/show structure\n");
		cons_printf("  > as -name  :  edit structure\n");
		cons_printf("  > as ?      :  list all spcc in dir.spcc\n");
		return;
	}

	buf[0]='\0';
	ptr = strchr(name, '-');

	if (ptr)
		sprintf(buf, "!!${EDITOR} %s/%s.spcc", path, ptr+1);
	else
	if (strchr(name, '?'))
		sprintf(buf, "!!rsc spcc-fe list");
	else sprintf(buf, "!!rsc spcc-fe %s ${BLOCK} 0", name);

	radare_cmd_raw(buf,0);
}

/* CALLBACK defined with graph.split which is false by default */
int (*code_analyze_r)(struct program_t *prg, u64 seek, int depth) = &code_analyze_r_nosplit;

struct program_t *code_analyze(u64 seek, int depth)
{
	struct program_t *prg = program_new(NULL);
	prg->entry = config.seek;

	radare_controlc();

	if (config_get("graph.split"))
		code_analyze_r = &code_analyze_r_split;
	else	code_analyze_r = &code_analyze_r_nosplit;

	if (prg == NULL)
		eprintf("Cannot create program\n");
	else
		code_analyze_r(prg, seek, depth);
		//code_analyze_r_nosplit(prg, seek, depth);

	// TODO: construct xrefs from tnext/fnext info
	radare_controlc_end();

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

int radare_analyze(u64 seek, int size, int depth)
{
	char cmd[1024];
	int count=0;
	unsigned int num, nume; // little endian
	int i;
	unsigned char str[1024];
	int str_i=0;
	unsigned char word[128];
	int word_i=0;
	u64 tmp = config.seek;
	int v = config.verbose;
	config.verbose = 0;
	int lastnull = 0;

	if (depth<0)
		return 0;

	config.seek = seek;
	radare_read(0);

	if ((size*4)>config.block_size)
		size = config.block_size/4;

	if (size<0) {
		size = 64;
		count=1;
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
			//for(j=config_get_i("cfg.analdepth"); j>depth;j--)
				cons_strcat("   ");
			print_addr((u64)(seek+i-str_i));
			C	cons_printf("string "C_BYELLOW"\"%s\""C_RESET"\n", str);
			else	cons_printf("string \"%s\"\n", str);
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
			//		for(j=config_get_i("cfg.analdepth"); j>depth;j--)
						cons_strcat("   ");
					print_addr(seek+i-3);
					C cons_printf(C_YELLOW"(NULL)"C_RESET"\n");
					else cons_printf("(NULL)\n");
				}
			} else if (num == -1) {
				/* ignore -1 */
				//print_addr(seek+i-3);
				//cons_printf("0xffffffff (-1)\n");
			} else {
				if (lastnull>1)
					cons_printf("(last null repeated %d times)\n", lastnull);
				lastnull = 0;
			//	for(j=config_get_i("cfg.analdepth"); j>depth;j--)
			//		cons_strcat("   ");
				print_addr(seek+i-3);
				C {
					if (config.endian)
					cons_printf("int be="C_YELLOW"0x%08x"C_RESET" le=0x%08x ",
						num, nume);
					else
					cons_printf("int be=0x%08x le="C_YELLOW"0x%08x"C_RESET" ",
						num, nume);
				} else
					cons_printf("int be=0x%08x le=0x%08x ",
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
					unsigned int n = (config.endian)?num:nume;
					C cons_printf(C_TURQOISE);
					sprintf(cmd, ":fd @0x%08x", (config.endian)?num:nume);
					radare_cmd(cmd, 0);

					//cons_strcat("     ");
					if (n == (unsigned int)seek)
						cons_printf("  (self pointer)\n");
					else
						radare_analyze(n, size, --depth);

					config.seek = seek;
					radare_read(0);
					C cons_printf(C_RESET);
				}
			}
			if (count)
				break;
		}
	}

	/* restore */
	config.seek = tmp;
	radare_read(0);
	cons_strcat("\n");
	config.verbose = v;

	return 0;
}
