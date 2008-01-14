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
#include "rdb/rdb.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

/* code analyzer */
int (*arch_aop)(unsigned long addr, const unsigned char *bytes, struct aop_t *aop);

// called from env_update
void arch_set_callbacks()
{
	const char *a = config_get("asm.arch");
	arch_aop = &arch_x86_aop;
	if (!a) return;

	if (!strcmp(a, "arm")) {
		arch_aop = &arch_arm_aop;
	} else
	if (!strcmp(a, "java")) {
		arch_aop = &arch_java_aop;
	} else
	if (!strcmp(a, "ppc")) {
		arch_aop = &arch_ppc_aop;
	} else
	if (!strcmp(a, "intel")) {
		arch_aop = &arch_x86_aop;
	}
}

/* code lines */
struct reflines_t *code_lines_init()
{
	struct reflines_t *list = (struct reflines_t*)malloc(sizeof(struct reflines_t));
	struct reflines_t *list2;
	unsigned char *ptr = config.block;
	unsigned char *end = config.block + config.block_size;
	struct aop_t aop;
	int sz, bsz = 0;
	int index = 0;

	INIT_LIST_HEAD(&(list->list));

	/* analyze code block */
	while( ptr < end ) {
		if (config.interrupted)
			break;
		sz = arch_aop(config.baddr + config.seek+bsz, ptr, &aop);
		if (sz <1) {
			sz = 1;
		} else {
			/* store data */
			switch(aop.type) {
			case AOP_TYPE_CALL:
			case AOP_TYPE_CJMP:
			case AOP_TYPE_JMP:
				list2 = (struct reflines_t*)malloc(sizeof(struct reflines_t));
				list2->from = config.seek + bsz;
				list2->to = aop.jump;
				list2->index = index++;
				list_add_tail(&(list2->list), &(list->list));
				break;
			}
		}
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

void code_lines_print(struct reflines_t *list, off_t addr)
{
	struct list_head *pos;
	char ch = ' ';

	if (!list)
		return;

	printf(" ");
	list_for_each(pos, &(list->list)) {
		struct reflines_t *ref = list_entry(pos, struct reflines_t, list);
		if (config.interrupted)
			break;

		if (addr == ref->to) {
			printf("+");
			ch = '-';
		} else
		if (addr == ref->from) {
			printf("+");
			ch = '=';
		} else {
			if (ref->from < ref->to) {
				if (addr > ref->from && addr < ref->to) {
					if (ch=='-'||ch=='=')
						printf("(");
					else
					//if (0==addr%10)
					//	printf("v");
				//	else
						printf("|");
				} else
					printf("%c",ch);
			} else {
				if (addr < ref->from && addr > ref->to) {
					if (ch=='-'||ch=='=')
						printf("(");
					else
				//	if (0==addr%10)
				//		printf("^");
				//	else
						printf("|");
				} else {
					printf("%c",ch);
				}
			}
		}
	}

	if (ch=='-')
		printf("-> ");
	else
	if (ch=='=')
		printf("=< ");
	else	printf("   ");

	fflush(stdout);
}

/* code analyze */

int code_analyze_r(struct program_t *prg, unsigned long seek, int depth)
{
	struct aop_t aop;
	struct block_t *blk;
	unsigned long oseek = seek;
	off_t tmp = config.seek;
	unsigned int sz = 0, ret;
	int bsz = 0;// block size
	char buf[4096]; // bytes of the code block
	unsigned char *ptr = (unsigned char *)&buf;
	int callblocks = config_get("graph.callblocks");
	int jmpblocks = config_get("graph.jmpblocks");

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
		if (config.interrupted)
			break;
		sz = arch_aop(config.seek+bsz, config.block+bsz, &aop);
		if (sz<=0) {
			eprintf("Invalid opcode (%02x %02x)\n", config.block[0], config.block[1]);
			break;
		}

		if (!callblocks && aop.type == AOP_TYPE_CALL)
			aop.eob = 0;
		if (!jmpblocks && (aop.type == AOP_TYPE_JMP || aop.type == AOP_TYPE_CJMP))
			aop.eob = 0;

		switch(aop.type) {
		case AOP_TYPE_JMP:
		case AOP_TYPE_CJMP:
		case AOP_TYPE_CALL:
			block_add_call(prg, oseek, aop.jump);
		}


		memcpy(ptr+bsz, config.block+bsz, sz); // append bytes
	}
	bsz+=sz;
	config.seek = tmp;

	blk = block_get_new(prg, oseek);

#if 0
	if (aop.type == AOP_TYPE_UNK) {
		pprintf("Oopppsz!\n");
		return 0;
	}
#endif
	blk->bytes = (unsigned char *)malloc(bsz);
	blk->n_bytes = bsz;
	memcpy(blk->bytes, buf, bsz);
	blk->tnext = aop.jump;
/*
	if (blk->tnext<config.baddr)
		blk->tnext += config.baddr;
*/
	blk->fnext = aop.fail;
	oseek = seek;

	/* walk childs */
	if (blk->tnext)
		code_analyze_r(prg, (unsigned long)blk->tnext, depth-1);
	if (blk->fnext)
		code_analyze_r(prg, (unsigned long)blk->fnext, depth-1);
	bsz = 0;

	depth--;

	return 0;
}

struct program_t *code_analyze(off_t seek, int depth)
{
	struct program_t *prg = program_new(NULL);
	prg->entry = config.seek;
	
	radare_controlc();
	arch_set_callbacks();

	if (prg == NULL)
		eprintf("Cannot create program\n");
	else
		code_analyze_r(prg, (unsigned long)seek, depth);

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

int radare_analyze(off_t seek, int size)
{
	char cmd[1024];
	int num;
	int count=0;
	int nume; // little endian
	int i;
	unsigned char str[1024];
	int str_i=0;
	unsigned char word[128];
	int word_i=0;
	off_t tmp = config.seek;
	int v = config.verbose;
	config.verbose = 0;

	config.seek = seek;
	radare_read(0);

	if ((size*4)>config.block_size)
		size = config.block_size/4;

	if (size<0) {
		size = 64;
		count=1;
	}
	for(i=0;i<size;i++) {
		if (is_printable(config.block[i])) {
			if(word_i<4) word[word_i++] = config.block[i];
			str[str_i++] = config.block[i];
			continue;
		}
		if (str_i>2) {
			str[str_i] = '\0';
			print_addr((off_t)(seek+i-str_i));
			C	pprintf("string "C_BYELLOW"\"%s\""C_RESET"\n", str);
			else	pprintf("string \"%s\"\n", str);
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
				print_addr(seek+i-3);
				C pprintf(C_YELLOW"(NULL)"C_RESET"\n");
				else pprintf("(NULL)\n");
			} else if (num == -1) {
				/* ignore -1 */
				//print_addr(seek+i-3);
				//pprintf("0xffffffff (-1)\n");
			} else {
				print_addr(seek+i-3);
				C {
					if (config.endian)
					pprintf("int be="C_YELLOW"0x%08x"C_RESET" le=0x%08x ",
						num, nume);
					else
					pprintf("int be=0x%08x le="C_YELLOW"0x%08x"C_RESET" ",
						num, nume);
				} else
					pprintf("int be=0x%08x le=0x%08x ",
						num, nume);

				if (num>-0xfffff && num<0xfffff)
					pprintf("(%d)\n", num);
				else
				if (nume>-0xfffff && nume<0xfffff)
					pprintf("(%d)\n", nume);
				else {
					C pprintf(C_TURQOISE);
					fflush(stdout);
					sprintf(cmd, ":fd @0x%08x", (config.endian)?num:nume);
					radare_command(cmd, 0);

					pprintf("     ");
					radare_analyze((config.endian)?num:nume, -1);

					config.seek = seek;
					radare_read(0);
					C pprintf(C_RESET);
				}
			}
			if (count)
				break;
		}
	}

	/* restore */
	config.seek = tmp;
	radare_read(0);
	pprintf("\n");
	config.verbose = v;

	return 0;
}
