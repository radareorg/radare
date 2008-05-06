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
int (*arch_aop)(u64 addr, const u8 *bytes, struct aop_t *aop);

// called from env_update
void arch_set_callbacks()
{
	const char *a = config_get("asm.arch");
	arch_aop = &arch_x86_aop;
	if (!a) return;

	if (!strcmp(a, "arm")) {
		arch_aop = &arch_arm_aop;
	} else
	if (!strcmp(a, "arm16")) {
		arch_aop = &arch_arm_aop;
	} else
	if (!strcmp(a, "java")) {
		arch_aop = &arch_java_aop;
	} else
	if (!strcmp(a, "mips")) {
		arch_aop = &arch_mips_aop;
	} else
	if (!strcmp(a, "ppc")) {
		arch_aop = &arch_ppc_aop;
	} else
	if (!strcmp(a, "csr")) {
		arch_aop = &arch_csr_aop;
	} else
	if (!strcmp(a, "intel16")) {
		arch_aop = &arch_x86_aop;
	} else
	if (!strcmp(a, "intel32")) {
		arch_aop = &arch_x86_aop;
	} else
	if (!strcmp(a, "intel64")) {
		arch_aop = &arch_x86_aop;
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
	int bar = (int)config_get("asm.linesout");
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
				if (!bar) {
					/* skip outside lines */
					if (aop.jump > config.seek+config.block_size)
						goto __next;
					/* skip outside lines */
					if (aop.jump < config.seek-30)
						goto __next;
				} else
					if (aop.jump == 0)
						goto __next;
				list2 = (struct reflines_t*)malloc(sizeof(struct reflines_t));
				list2->from = config.seek + bsz;
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
	int i, cow= 0;
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

int code_analyze_r(struct program_t *prg, u64 seek, int depth)
{
	struct aop_t aop;
	struct block_t *blk;
	unsigned long oseek = seek;
	u64 tmp = config.seek;
	unsigned int sz = 0, ret;
	int bsz = 0;// block size
	char buf[4096]; // bytes of the code block
	unsigned char *ptr = (unsigned char *)&buf;
	int callblocks =(int) config_get("graph.callblocks");
	int jmpblocks = (int) config_get("graph.jmpblocks");
	struct block_t *blf;
	
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
printf("%llx, %llx\n", aop.fail, aop.jump);
	//bsz+=sz;
			break;
		}

		blf = block_split_new ( prg, config.seek+bsz  );
		if ( blf != NULL )
		{		
			//printf ("--Address %llx already analed\n", config.seek+bsz );
printf("-- %llx, %llx\n", aop.fail, aop.jump);
			
			bsz = blf->n_bytes;
			aop.eob = 1;
			aop.jump = blf->tnext;
			aop.fail = blf->fnext;
			break;
		}		

		if (config.interrupted)
			break;

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
	config.seek = tmp;

	blk = block_get_new(prg, oseek);

	bsz+=sz;
	blk->bytes = (unsigned char *)malloc(bsz);
	blk->n_bytes = bsz;
	memcpy(blk->bytes, buf, bsz);
	blk->tnext = aop.jump;
	blk->fnext = aop.fail;

	oseek = seek;

	/* walk childs */
	if (blk->tnext && (blf == 0) )
		code_analyze_r(prg, blk->tnext, depth-1);
	if (blk->fnext  )
		code_analyze_r(prg, blk->fnext, depth-1);
	bsz = 0;

	depth--;

	return 0;
}

struct program_t *code_analyze(u64 seek, int depth)
{
	struct program_t *prg = program_new(NULL);
	prg->entry = config.seek;
	
	radare_controlc();
	arch_set_callbacks();

	if (prg == NULL)
		eprintf("Cannot create program\n");
	else
		code_analyze_r(prg, seek, depth);

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
					C cons_printf(C_TURQOISE);
					sprintf(cmd, ":fd @0x%08x", (config.endian)?num:nume);
					radare_cmd(cmd, 0);

					cons_strcat("     ");
					radare_analyze((config.endian)?num:nume, size, --depth);

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
