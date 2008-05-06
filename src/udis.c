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
#include "undo.h"
#include "flags.h"
#include "arch/csr/dis.h"
#include "arch/arm/disarm.h"
/* http://devnull.owl.de/~frank/Disassembler_e.html */
#include "arch/ppc/ppc_disasm.h"
#include "arch/m68k/m68k_disasm.h"
#include "arch/x86/udis86/types.h"
#include "arch/x86/udis86/extern.h"
#include "list.h"

enum {
	ARCH_X86 = 0,
	ARCH_ARM = 1,
	ARCH_ARM16 = 2,
	ARCH_PPC = 3,
	ARCH_M68K = 4,
	ARCH_JAVA = 5,
	ARCH_MIPS = 6,
	ARCH_CSR = 7
};

struct list_head data;

extern int force_thumb;

int data_set_len(u64 off, u64 len)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (off>= d->from && off< d->to) {
			d->to = d->from+len;
			d->size = d->to-d->from;
			return 0;
		}
	}
	return -1;
}

int data_set(u64 off, int type)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (off>= d->from && off< d->to) {
			d->type = type;
			return 0;
		}
	}
	return -1;
}

void data_add(u64 off, int type)
{
	u64 tmp;
	struct data_t *d;

	if (type == FMT_UDIS) {
		struct list_head *pos;
		__reloop:
		list_for_each(pos, &data) {
			struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
			if (off>= d->from && off<= d->to) {
				list_del((&d->list));
				goto __reloop;
			}
		}
		return;
	}

	d = (struct data_t *)malloc(sizeof(struct data_t));

	d->from = off;
	d->to = d->from+config.block_size;  // 1 byte if no cursor // on strings should autodetect
	if (config.cursor_mode) {
		d->to = d->from + 1;
		d->from+=config.cursor;
		if (config.ocursor!=-1)
			d->to = config.seek+config.ocursor;
		if (d->to < d->from) {
			tmp = d->to;
			d->to  = d->from;
			d->from = tmp;
		}
	}
	d->type = type;
	d->size = (d->to - d->from);
	if (d->size<1)
		d->size = 1;

	list_add(&(d->list), &data);
}

struct data_t *data_get(u64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset >= d->from && offset < d->to)
			return d;
	}
	return NULL;
}

int data_type_range(u64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset >= d->from && offset < d->to) {
			return d->type;
		}
	}
	return -1;
}

int data_type(u64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset == d->from) {
			return d->type;
		}
	}
	return -1;
}

int data_end(u64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset == d->to) {
			return d->type;
		}
	}
	return -1;
}

int data_count(u64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset == d->from)
			return d->size;
	}
	return 0;
}

int data_list()
{
	struct data_t *d;
	struct list_head *pos;
	list_for_each(pos, &data) {
		d = (struct data_t *)list_entry(pos, struct data_t, list);
		switch(d->type) {
		case DATA_FOLD_O: cons_strcat("Cu "); break;
		case DATA_FOLD_C: cons_strcat("Cf "); break;
		case DATA_HEX:    cons_strcat("Cd "); break;
		case DATA_STR:    cons_strcat("Cs "); break;
		default:          cons_strcat("Cc "); break; }
		cons_printf("%d @ 0x%08llx\n", d->to - d->from, d->type);
	}
	return 0;
}

static struct reflines_t *reflines = NULL;
struct list_head comments;

/* -- metadata -- */

void metadata_comment_add(u64 offset, const char *str)
{
	struct comment_t *cmt;

	/* no null comments */
	if (strnull(str))
		return;

	cmt = (struct comment_t *) malloc(sizeof(struct comment_t));
	cmt->offset = offset;
	cmt->comment = strdup(str);
	if (cmt->comment[strlen(cmt->comment)-1]=='\n')
		cmt->comment[strlen(cmt->comment)-1]='\0';
	list_add_tail(&(cmt->list), &(comments));
}

void metadata_comment_del(u64 offset, const char *str)
{
	struct comment_t *cmt;
	struct list_head *pos;
	u64 off = get_math(str);

	list_for_each(pos, &comments) {
		cmt = list_entry(pos, struct comment_t, list);
		if (!pos)
			return;
		if (off) {
			if (off == cmt->offset) {
				free(cmt->comment);
				list_del(&pos);
				free(cmt);
				if (str[0]=='*')
					metadata_comment_del(offset, str);
				pos = comments.next; // list_init
				return;
			}
		} else {
			if (str[0]=='*') {
				free(cmt->comment);
				list_del(&pos);
				free(cmt);
				pos = comments.next; // list_init
				//metadata_comment_del(offset, str);
			} else
			if (cmt->offset == offset) {
				list_del(&pos);
				return;
			}
		}
	}
}

void metadata_comment_list()
{
	struct list_head *pos;
	list_for_each(pos, &comments) {
		struct comment_t *cmt = list_entry(pos, struct comment_t, list);
		cons_printf("CC %s @ 0x"OFF_FMTx"\n", cmt->comment, cmt->offset);
	}
}

char *metadata_comment_get(u64 offset)
{
	struct list_head *pos;
	char *str = NULL;
	int cmtmargin = (int)config_get_i("asm.cmtmargin");
	int cmtlines = config_get_i("asm.cmtlines");
	char null[128];

	memset(null,' ',126);
	null[126]='\0';
	if (cmtmargin<0) cmtmargin=0; else
		// TODO: use screen width here
		if (cmtmargin>80) cmtmargin=80;
	null[cmtmargin] = '\0';
	if (cmtlines<0)
		cmtlines=0;

	if (cmtlines) {
		int i = 0;
		list_for_each(pos, &comments) {
			struct comment_t *cmt = list_entry(pos, struct comment_t, list);
			if (cmt->offset == offset)
				i++;
		}
		if (i>cmtlines)
			cmtlines = i-cmtlines;
	}

	list_for_each(pos, &comments) {
		struct comment_t *cmt = list_entry(pos, struct comment_t, list);
		if (cmt->offset == offset) {
			if (cmtlines) {
				cmtlines--;
				continue; // skip comment lines
			}
			if (str == NULL) {
				str = malloc(1024);
				str[0]='\0';
			} else {
				str = realloc(str, cmtmargin+strlen(str)+strlen(cmt->comment)+128);
			}
			strcat(str, null);
			strcat(str, "; ");
			strcat(str, cmt->comment);
			strcat(str, "\n");
		}
	}
	return str;
}

void metadata_comment_init(int new)
{
	INIT_LIST_HEAD(&(comments));
	INIT_LIST_HEAD(&(data));
}

static int metadata_print(int delta)
{
	int show_lines = (int)config_get("asm.lines");
	int show_flagsline = (int)config_get("asm.flagsline");
	u64 offset = (u64)config.seek + (u64)delta;
	int lines = 0;
	char *ptr;
	int i;

	// config.baddr everywhere???
	D {} else return 0;
	if (show_flagsline) {
		ptr = flag_name_by_offset( offset );
		if (ptr[0]) {
			if (show_lines&&reflines)
				code_lines_print(reflines, config.baddr+config.seek+i, 1);
			C
				cons_printf(C_RESET C_BWHITE""OFF_FMT" %s:"C_RESET"\n",
					config.baddr+offset, ptr);
			else
				cons_printf(OFF_FMTs" %s:\n",
					config.baddr+offset, ptr);
			lines++;
		}
	}

	ptr = metadata_comment_get(offset);
	if (ptr && ptr[0]) {
		int i;
#if 0
		if (show_lines&&reflines)
			code_lines_print2(reflines, config.baddr+config.seek +i);
		C	cons_printf(C_RESET C_BWHITE""OFF_FMT" %s:"C_RESET"\n", config.baddr+offset, ptr);
		else	cons_printf(OFF_FMTs" %s:\n", config.baddr+offset, ptr);
#endif
		for(i=0;ptr[i];i++)
			if (ptr[i]=='\n') lines++;
		C 	cons_printf(C_MAGENTA"%s"C_RESET, ptr);
		else 	cons_printf("%s", ptr);
		free(ptr);
	}

	return lines;
}


static ud_t ud_obj;
static unsigned char o_do_off = 1;
static int ud_idx = 0;
static int length = 0;

static int input_hook_x(ud_t* u)
{
	if (ud_idx>length)
		return -1;
	if (ud_idx>=config.block_size)
		return -1;
	return config.block[ud_idx++];
}

int udis86_color = 0;

extern int arm_mode;
extern int mips_mode;
void udis_init()
{
	const char *syn = config_get("asm.syntax");
	const char *ptr = config_get("asm.arch");

	ud_init(&ud_obj);

	arm_mode = 32;
	if (!strcmp(ptr, "arm16")) {
		arm_mode = 16;
	} else
	if (!strcmp(ptr, "intel16")) {
		ud_set_mode(&ud_obj, 16);
	} else
	if((!strcmp(ptr, "intel")) || (!strcmp(ptr, "intel32"))) {
			ud_set_mode(&ud_obj, 32);
	} else
	if (!strcmp(ptr, "intel64")) {
		ud_set_mode(&ud_obj, 64);
	} else
		ud_set_mode(&ud_obj, 32);

	udis86_color = config.color;

	/* set syntax */
	ud_set_syntax(&ud_obj, UD_SYN_INTEL);
	if (syn) {
		if (!strcmp(syn,"pseudo")) 
			ud_set_syntax(&ud_obj, UD_SYN_PSEUDO);
		else
			if (!strcmp(syn,"att")) 
				ud_set_syntax(&ud_obj, UD_SYN_ATT);
	}

#ifdef _WIN32
	_setmode(_fileno(stdin), _O_BINARY);
#endif  
	ud_set_input_hook(&ud_obj, input_hook_x);
}

static int jump_n = 0;
static u64 jumps[10]; // only 10 jumps allowed

void udis_jump(int n)
{
	if (n<jump_n) {
		radare_seek(jumps[n], SEEK_SET);
		config.seek = jumps [n];
		radare_read(0);
		undo_push();
	}
}

/* -- disassemble -- */

//#define CHECK_LINES printf("%d/%d\n",lines,rows); if ( (config.visual && len!=config.block_size && (++lines>=rows))) break;
#define CHECK_LINES if ( (config.visual && len!=config.block_size && (++lines>=rows))) break;

extern int color;
void udis_arch(int arch, int len, int rows)
{
	struct aop_t aop;
	char* hex1, *hex2;
	char c;
	int i,idata,delta;
	u64 seek = 0;
	int lines = 0;
	int bytes = 0;
	u64 myinc = 0;
	unsigned char b[32];
	const char *follow, *cmd_asm;
	int endian;
	int show_size, show_bytes, show_offset,show_splits,show_comments,show_lines,show_traces,show_nbytes, show_flags;
	int folder = 0; // folder level

	cmd_asm       = config_get("cmd.asm");
	show_size     = (int) config_get("asm.size");
	show_bytes    = (int) config_get("asm.bytes");
	show_offset   = (int) config_get("asm.offset");
	show_splits   = (int) config_get("asm.split");
	show_flags    = (int) config_get("asm.flags");
	show_lines    = (int) config_get("asm.lines");
	show_traces   = (int) config_get("asm.trace");
	show_comments = (int) config_get("asm.comments");
	show_nbytes   = (int) config_get_i("asm.nbytes");
	endian        = (int) config_get("cfg.endian");
	color         = (int) config_get("scr.color");

	len*=2; // uh?!
	jump_n = 0;
	length = len;
	ud_idx = 0;
	delta = 0;
	inc = 0;

	if (rows<1)
		rows = config.height + config.lines;

	switch(arch) {
	case ARCH_X86:
		udis_init();
		ud_obj.pc = config.seek;
		break;
	case ARCH_ARM:
		arm_mode = 32;
		force_thumb = 0;
		break;
	case ARCH_ARM16:
		arm_mode = 16;
		force_thumb = 1;
		break;
	case ARCH_JAVA:
		endian = 1;
		break;
	}

	if (show_nbytes>16 ||show_nbytes<0)
		show_nbytes = 16;

	reflines = NULL;
	if (show_lines)
		reflines = code_lines_init();
	radare_controlc();

	while (!config.interrupted) {
		if (bytes>=config.block_size)
			break;
		seek = config.baddr +config.seek+bytes;
		CHECK_LINES

		if (show_comments)
			lines+=metadata_print(bytes);
		/* is this data? */
		idata = data_count(seek);
		if (idata>0) {
			struct data_t *foo = data_get(seek);
			if (show_lines)
				code_lines_print(reflines, seek, 0);
			if (show_offset) {
				print_addr(seek);
				//C cons_printf(C_GREEN"0x%08llX "C_RESET, (unsigned long long)(seek));
				//else cons_printf("0x%08llX ", (unsigned long long)(seek));
			}
			switch(data_type(seek)) {
			case DATA_FOLD_C: 
				cons_printf("  { 0x%llx-0x%llx %lld }\n", foo->from, foo->to, (foo->to-foo->from));
				bytes+=(foo->to-foo->from);
				ud_idx+=(foo->size);
				//bytes+=idata;
				continue;
			case DATA_FOLD_O:
				cons_strcat("\r                                       \r");
				if (show_lines)
					code_lines_print(reflines, seek, 1);
				cons_strcat("           ");
				for(i=0;i<folder;i++)cons_strcat("  ");
					cons_strcat("  {\n");
				CHECK_LINES
				folder++;
				goto __outofme;
			case DATA_STR:
				cons_strcat("  .string \"");
				for(i=0;i<idata;i++) {
					print_color_byte_i(bytes+i, "%c", is_printable(config.block[bytes+i])?config.block[bytes+i]:'.');
				}
				cons_strcat("\"");
				break;
			case DATA_HEX:
			default:
				cons_printf("  .db  ");
				for(i=0;i<idata;i++) {
					print_color_byte_i(bytes+i,"%02x ", config.block[bytes+i]);
				}
				break;
			}
			cons_newline();
			CHECK_LINES
			bytes+=idata;
			continue;
		}
		__outofme:
		if (data_end(seek) == DATA_FOLD_O) {
			if (show_lines)
				code_lines_print(reflines, seek, 1);
			if (show_offset) {
				cons_strcat("           ");
				//C cons_printf(C_GREEN"0x%08llX "C_RESET, (unsigned long long)(seek));
				//else cons_printf("0x%08llX ", (unsigned long long)(seek));
			}
			folder--;
			for(i=0;i<folder;i++)cons_strcat("  ");
			cons_strcat("  }\n");
			CHECK_LINES
		}

		switch(arch) {
		case ARCH_X86:
			memcpy(b, config.block+bytes, 32);
			break;
		case ARCH_CSR:
			memcpy(b, config.block+bytes, 2);
			break;
		default:
			endian_memcpy_e(b, config.block+bytes, 4, endian);
		}

		if (cmd_asm && cmd_asm[0]) {
			char buf[1024];
			sprintf(buf, "%lld", seek);
			setenv("HERE", buf, 1);
			radare_cmd(cmd_asm, 0);
		}

		switch(arch) {
			case ARCH_X86:
				if (ud_disassemble(&ud_obj)<1)
					return;
				arch_x86_aop((unsigned long)ud_insn_off(&ud_obj), (const unsigned char *)config.block+bytes, &aop);
				myinc = ud_insn_len(&ud_obj);
				break;
			case ARCH_ARM16:
				arm_mode = 16;
				myinc = 2;
				arch_arm_aop(seek, (const unsigned char *)b, &aop);
				break;
			case ARCH_ARM:
				myinc = arch_arm_aop(seek, (const unsigned char *)b, &aop);
				break;
			case ARCH_MIPS:
				arch_mips_aop(seek, (const unsigned char *)b, &aop);
				myinc = aop.length;
				break;
			case ARCH_JAVA:
				arch_java_aop(seek, (const unsigned char *)config.block+bytes, &aop);
				myinc = aop.length;
				break;
			case ARCH_PPC:
				arch_ppc_aop(seek, (const unsigned char *)b, &aop);
				myinc = aop.length;
				break;
			case ARCH_CSR:
				arch_csr_aop(seek, (const unsigned char *)b, &aop);
				myinc = 2;
				break;
			default:
				// Uh?
				myinc = 4;
				break;
				// TODO
		}
		if (myinc<1)
			myinc = 1;

		if (config.cursor_mode) {
			if (config.cursor == bytes)
				inc = myinc;
		} else
			if (inc == 0)
				inc = myinc;
		length-=myinc;
		if (length<=0)
			break;
		D { 
			// TODO autodetect stack frames here !! push ebp and so... and wirte a comment
			if (show_lines)
				code_lines_print(reflines, seek, 0); //config.baddr+ud_insn_off(&ud_obj));

			if (show_offset) {
				print_addr(seek);
				//C cons_printf(C_GREEN"0x%08llX "C_RESET, (unsigned long long)(seek));
				//else cons_printf("0x%08llX ", (unsigned long long)(seek));
			}
			/* size */
			if (show_size)
				cons_printf("%d ", aop.length); //dislen(config.block+seek));
			/* trac information */
			if (show_traces)
				cons_printf("%02d %02d ", trace_count(seek), trace_times(seek));

			if (show_flags) {
				char buf[1024];
				char *flag = flag_name_by_offset(seek);
				sprintf(buf, "%%%ds:", show_nbytes);
				if (flag && flag[0]) {
					if (strlen(flag)>show_nbytes) {
						cons_printf(buf, flag);
						NEWLINE;
						CHECK_LINES
						if (show_lines)
							code_lines_print(reflines, seek, 0); //config.baddr+ud_insn_off(&ud_obj));
						if (show_offset) {
							C cons_printf(C_GREEN"0x%08llX  "C_RESET, (unsigned long long)(seek));
							else cons_printf("0x%08llX  ", (unsigned long long)(seek));
						}
						sprintf(buf, "%%%ds ", show_nbytes);
						cons_printf(buf,"");
					} else {
						cons_printf(buf, flag);
					}
				} else {
					sprintf(buf, "%%%ds ", show_nbytes);
					cons_printf(buf,"");
				}
			}
			/* cursor and bytes */
			if (is_cursor(bytes, myinc)) {
				cons_printf("*");
			} else  cons_printf(" ");
			if (show_bytes) {
				int max = show_nbytes;
				int cur = myinc;
				if (cur + 1> show_nbytes)
					cur = show_nbytes - 1;

				for(i=0;i<cur; i++)
					print_color_byte_i(bytes+i, "%02x", b[i]); //config.block[bytes+i]); //ud_obj.insn_hexcode[i]);
				if (cur !=myinc)
					max--;
				for(i=(max-cur)*2;i>0;i--)
					cons_printf(" ");
				if (cur != myinc)
					cons_printf(". ");
			}
			C switch(aop.type) {
			case AOP_TYPE_CMP:
				cons_strcat("\e[36m");
				break;
			case AOP_TYPE_REP:
			case AOP_TYPE_JMP:
			case AOP_TYPE_CJMP:
			case AOP_TYPE_CALL: // green
				cons_strcat("\e[32m");
				break;
			case AOP_TYPE_NOP:
				cons_strcat("\e[35m");
				break;
			case AOP_TYPE_UPUSH:
			case AOP_TYPE_PUSH:
			case AOP_TYPE_POP: // yellow
				cons_strcat("\e[33m");
				break;
			case AOP_TYPE_SWI:
			//case AOP_TYPE_UNK:
			case AOP_TYPE_TRAP:
			case AOP_TYPE_UJMP: //red
				cons_strcat("\e[31m");
				break;
			case AOP_TYPE_RET: // blue
				cons_strcat("\e[34m");
				break;
			}
			switch(arch) {
			case ARCH_X86:
				hex1 = ud_insn_hex(&ud_obj);
				hex2 = hex1 + 16;
				c = hex1[16];
				hex1[16] = 0;
				cons_printf("%-24s", ud_insn_asm(&ud_obj));
				hex1[16] = c;
				if (strlen(hex1) > 24) {
					C cons_printf(C_RED);
					cons_printf("\n");
					if (o_do_off)
						cons_printf("%15s .. ", "");
					cons_printf("%-16s", hex2);
				}
				break;
			case ARCH_CSR: {
				if (bytes+myinc<config.block_size)
					arch_csr_disasm((const unsigned char *)b, (u64)seek);
				}
				break;
			case ARCH_ARM16:
			case ARCH_ARM:
				       //unsigned long ins = (b[0]<<24)+(b[1]<<16)+(b[2]<<8)+(b[3]);
				       //cons_printf("  %s", disarm(ins, (unsigned int)seek));
				       gnu_disarm((unsigned char*)b, (unsigned int)seek);
				       break;
			case ARCH_MIPS:
				       //unsigned long ins = (b[0]<<24)+(b[1]<<16)+(b[2]<<8)+(b[3]);
				       //cons_printf("  %s", disarm(ins, (unsigned int)seek));
				       gnu_dismips((unsigned char*)b, (unsigned int)seek);
				       break;
			case ARCH_PPC: {
					       char opcode[128];
					       char operands[128];
					       struct DisasmPara_PPC dp;
					       /* initialize DisasmPara */
					       dp.opcode = opcode;
					       dp.operands = operands;
					       dp.iaddr = seek; //config.baddr + config.seek + i;
					       dp.instr = b; //config.block + i;
					       PPC_Disassemble(&dp, endian);
					       cons_printf("  %s %s", opcode, operands);
				       } break;
			case ARCH_JAVA: {
						char output[128];
						if (java_disasm(b, output)!=-1)
							cons_printf(" %s", output);
						else cons_strcat(" ???");
					} break;
			case ARCH_M68K: {
						char opcode[128];
						char operands[128];
						struct DisasmPara_PPC dp;
						/* initialize DisasmPara */
						dp.opcode = opcode;
						dp.operands = operands;
						dp.iaddr = seek; //config.baddr + config.seek + i;
						dp.instr = b; //config.block + i;
						// XXX read vda68k: this fun returns something... size of opcode?
						M68k_Disassemble(&dp);
						cons_printf("  %s %s", opcode, operands);
					} break;
			}
			C cons_printf(C_RESET);
			if (aop.ref) {
				char buf[1024];
				if (string_flag_offset(buf, aop.ref))
					cons_printf(" ; %s",buf);
			}

			if (aop.jump) {
				if (++jump_n<10) {
					jumps[jump_n-1] = aop.jump;
					if (arch == ARCH_X86)
						cons_printf("   [%d]", jump_n);
					else cons_printf("   [%d] -> 0x%x", jump_n, (unsigned int)aop.jump);
				}
			}

			if (show_splits) {
				char buf[1024];
				if (aop.jump||aop.eob) {
					if (config_get("asm.splitall") || aop.type == AOP_TYPE_RET) {
						NEWLINE;
						if (show_lines)
							code_lines_print(reflines, seek, 0);
						if (show_offset) {
							C cons_printf(C_GREEN"0x%08llX "C_RESET, (unsigned long long)(seek));
							else cons_printf("0x%08llX ", (unsigned long long)(seek));
						}
						sprintf(buf, "%%%ds ", show_nbytes);
						cons_printf(buf,"");
						cons_printf("; ------------------------------------ ");
						CHECK_LINES
					}
				}
			}
		} else {
			switch(arch) {
				case ARCH_X86:
					cons_printf("%s", ud_insn_asm(&ud_obj));
					break;
				case ARCH_ARM16:
				case ARCH_ARM:
					break;
			}
		}
		NEWLINE;
		seek+=myinc;
		bytes+=myinc;
	}
	radare_controlc_end();
}

void disassemble(int len, int rows)
{
	char *ptr = config_get("asm.arch");

	if (ptr == NULL) {
		eprintf("No ARCH defined.\n");
		return;
	}

	radare_controlc();

	/* handles intel16, intel32, intel64 */
	if (!memcmp(ptr, "intel", 5))
		udis_arch(ARCH_X86, len,rows);
	else
	if (!strcmp(ptr, "mips"))
		udis_arch(ARCH_MIPS, len,rows);
	else
	if (!strcmp(ptr, "arm"))
		udis_arch(ARCH_ARM, len,rows);
	else
	if (!strcmp(ptr, "arm16"))
		udis_arch(ARCH_ARM16, len,rows);
	else
	if (!strcmp(ptr, "java"))
		udis_arch(ARCH_JAVA, len, rows);
	else
	if (!strcmp(ptr, "ppc"))
		udis_arch(ARCH_PPC, len, rows);
	else
	if (!strcmp(ptr, "m68k"))
		udis_arch(ARCH_M68K, len, rows);
	else
	if (!strcmp(ptr, "csr"))
		udis_arch(ARCH_CSR, len, rows);

	radare_controlc_end();
	fflush(stdout);
}
