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

//#define CHECK_LINES if ( config.visual && len != config.block_size && (cons_lines > config.height) ) break;
#define CHECK_LINES if ( config.visual && (cons_lines > config.height) ) break;

static int last_arch = ARCH_X86;
struct list_head data;
extern int force_thumb;

static ud_t ud_obj;
static int length = 0;
/* XXX UD_IDX SHOULD BE REPLACED BY 'BYTES'. THEY HAVE THE SAME FINALITY. DRY! */
static int ud_idx = 0;
// XXX udis does not needs to use this !!!
int udis86_color = 0;

extern int arm_mode;
extern int mips_mode;

int data_set_len(u64 off, u64 len)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (off>= d->from && off< d->to) {
			d->to = d->from+len+1;
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
			if (d && (off>= d->from && off<= d->to)) {
				list_del((&d->list)); //->list));
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
	if (d->to > d->from) {
		d->to++;
		d->size = d->to - d->from;
	} else d->size = d->from - d->to+1;
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

struct data_t *data_get_range(u64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset >= d->from && offset < d->to-1)
			return d;
	}
	return NULL;
}

int data_type_range(u64 offset)
{
	struct data_t *d = data_get_range(offset);
	if (d != NULL)
		return d->type;
	return -1;
}

int data_type(u64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset == d->from)
			return d->type;
	}
	return -1;
}

int data_end(u64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset == d->from+d->size) // XXX: must be d->to..but is buggy ?
			return d->type;
	}
	return -1;
}

int data_size(u64 offset)
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
		cons_printf("%d @ 0x%08llx\n", d->size, d->type);
	}
	return 0;
}

static struct reflines_t *reflines = NULL;
struct list_head comments;
struct list_head xrefs;

/* -- metadata -- */
int metadata_xrefs_print(u64 addr, int type)
{
	int n = 0;
	struct xrefs_t *x;
	struct list_head *pos;
	list_for_each(pos, &xrefs) {
		x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		if (x->addr == addr) {
			switch(type) {
			case 0: if (x->type == type) { cons_printf("; CODE xref %08llx\n", x->from); n++; } break;
			case 1: if (x->type == type) { cons_printf("; DATA xref %08llx\n", x->from); n++; } break;
			default: { cons_printf("; %s xref %08llx\n", (x->type==1)?"DATA":(x->type==0)?"CODE":"UNKNOWN",x->from); n++; };
			}
		}
	}

	return n;
}

int metadata_xrefs_add(u64 addr, u64 from, int type)
{
	struct xrefs_t *x;
	struct list_head *pos;

	/* avoid dup */
	list_for_each(pos, &xrefs) {
		x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		if (x->addr == addr && x->from == from)
			return 0;
	}

	x = (struct xrefs_t *)malloc(sizeof(struct xrefs_t));

	x->addr = addr;
	x->from = from;
	x->type = type;

	list_add(&(x->list), &xrefs);

	return 1;
}

void metadata_xrefs_del(u64 addr, u64 from, int data /* data or code */)
{
	struct xrefs_t *x;
	struct list_head *pos;
	list_for_each(pos, &xrefs) {
		x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		if (x->addr == addr && x->from == from) {
			list_del(&(x->list));
			break;
		}
	}
}

void metadata_comment_del(u64 offset, const char *str)
{
	struct comment_t *cmt;
	struct list_head *pos;
	//u64 off = get_math(str);

	list_for_each(pos, &comments) {
		cmt = list_entry(pos, struct comment_t, list);
#if 0
		if (!pos)
			return;
#endif

#if 0
		if (off) {
			if ((off == cmt->offset)) {
				free(cmt->comment);
				list_del(&(pos));
				free(cmt);
				if (str[0]=='*')
					metadata_comment_del(offset, str);
				pos = comments.next; // list_init
				return;
			}
		} else {
#endif
		    if (offset == cmt->offset) {
			    if (str[0]=='*') {
				    free(cmt->comment);
				    list_del(&(pos));
				    free(cmt);
				    pos = comments.next; // list_init
				    //metadata_comment_del(offset, str);
			    } else
			    if (!strcmp(cmt->comment, str)) {
				    list_del(&(pos));
				    return;
			    }
		    }
#if 0
		}
#endif
	}
}

void metadata_comment_add(u64 offset, const char *str)
{
	struct comment_t *cmt;
	char *ptr;

	/* no null comments */
	if (strnull(str))
		return;

	/* avoid dupped comments */
	metadata_comment_del(offset, str);

	cmt = (struct comment_t *) malloc(sizeof(struct comment_t));
	cmt->offset = offset;
	ptr = strdup(str);
	if (ptr[strlen(ptr)-1]=='\n')
		ptr[strlen(ptr)-1]='\0';
	cmt->comment = ptr;
	list_add_tail(&(cmt->list), &(comments));
}


void metadata_comment_list()
{
	struct list_head *pos;
	list_for_each(pos, &comments) {
		struct comment_t *cmt = list_entry(pos, struct comment_t, list);
		cons_printf("CC %s @ 0x%llx\n", cmt->comment, cmt->offset);
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
	INIT_LIST_HEAD(&(xrefs));
	INIT_LIST_HEAD(&(comments));
	INIT_LIST_HEAD(&(data));
}

static int metadata_print(int delta)
{
	int show_lines = (int)config_get("asm.lines");
	int show_flagsline = (int)config_get("asm.flagsline");
	u64 offset = (u64)config.seek + (u64)delta;// - config.baddr;
	int lines = 0;
	const char *ptr;
	int i = 0;

	D {} else return 0;
	if (config_get("asm.flags") && show_flagsline) {
		ptr = flag_name_by_offset( offset );
		if (ptr == NULL && config.baddr)
			ptr = flag_name_by_offset( config.seek + delta);
		if (ptr && ptr[0]) {
			if (show_lines&&reflines)
				code_lines_print(reflines, offset, 1);
			C
				cons_printf(C_RESET C_BWHITE""OFF_FMT" %s:"C_RESET"\n", offset, ptr);
			else
				cons_printf(OFF_FMTs" %s:\n", offset, ptr);
			lines++;
		}
	}

	ptr = metadata_comment_get(offset);
	if (ptr && ptr[0]) {
		int i;
		for(i=0;ptr[i];i++)
			if (ptr[i]=='\n') lines++;
		C 	cons_printf(C_MAGENTA"%s"C_RESET, ptr);
		else 	cons_printf("%s", ptr);
		free(ptr);
	}

	lines += metadata_xrefs_print(offset, -1);
	return lines;
}

static int input_hook_x(ud_t* u)
{
	if (ud_idx>length)
		return -1;
	if (ud_idx>=config.block_size)
		return -1;
	return config.block[ud_idx++];
}

void udis_init()
{
	const char *syn = config_get("asm.syntax");
	const char *ptr = config_get("asm.arch");

	ud_init(&ud_obj);
	ud_set_pc(&ud_obj, config.seek);

	arm_mode = 32;
	if (!strcmp(ptr, "arm16")) {
		arm_mode = 16;
	} else
	if (!strcmp(ptr, "intel16")) {
		ud_set_mode(&ud_obj, 16);
	} else
	if((!strcmp(ptr, "intel")) || (!strcmp(ptr, "intel32")) || (!strcmp(ptr,"x86"))) {
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
		else if (!strcmp(syn,"att")) 
			ud_set_syntax(&ud_obj, UD_SYN_ATT);
	}
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

// TODO: remove myinc here
int udis_arch_opcode(int arch, int endian, u64 seek, int bytes, int myinc)
{
	unsigned char *b = config.block + bytes;
	struct aop_t aop;
	int c, ret=0;
	ud_idx = bytes+myinc;

	switch(arch) {
	case ARCH_X86:
		ud_obj.insn_offset = seek+myinc; //+bytes;
		ud_obj.pc = seek+myinc;
		ret = ud_insn_len(&ud_obj);
		cons_printf("%-24s", ud_insn_asm(&ud_obj));
		//cons_printf("%08llx: %d %d %-24s", seek, ud_idx, bytes, ud_insn_asm(&ud_obj));
		break;
	case ARCH_CSR:
		if (bytes<config.block_size)
			arch_csr_disasm((const unsigned char *)b, (u64)seek);
		break;
	case ARCH_AOP:
		arch_aop(seek, b, &aop);
		ret = arch_aop_aop(seek, b, &aop);
		break;
	case ARCH_ARM16:
	case ARCH_ARM:
	       //unsigned long ins = (b[0]<<24)+(b[1]<<16)+(b[2]<<8)+(b[3]);
	       //cons_printf("  %s", disarm(ins, (unsigned int)seek));
	       gnu_disarm((unsigned char*)b, (unsigned int)seek);
	       break;
	case ARCH_MIPS:
	       //unsigned long ins = (b[0]<<24)+(b[1]<<16)+(b[2]<<8)+(b[3]);
	       gnu_dismips((unsigned char*)b, (unsigned int)seek);
	       break;
	case ARCH_SPARC:
	       gnu_disparc((unsigned char*)b, (unsigned int)seek);
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
	default:
		cons_printf("Unknown architecture\n");
		break;
	}
	C cons_printf(C_RESET);
	return ret;
}

extern int color;
void udis_arch(int arch, int len, int rows)
{
	struct aop_t aop;
	//char c;
	int i,idata,delta;
	u64 seek = 0;
	u64 sk = 0;
	int bytes = 0;
	u64 myinc = 0;
	unsigned char b[32];
	char buf[1024];
	const char *flag;
	const char *cmd_asm;
	int rrows = rows; /* rrows is in reality the num of bytes to be disassembled */
	int endian;
	int show_size, show_bytes, show_offset,show_splits,show_comments,show_lines,
	show_traces,show_nbytes, show_flags, show_reladdr, show_flagsline;
	int folder = 0; // folder level

	cmd_asm       = config_get("cmd.asm");
	show_size     = (int) config_get("asm.size");
	show_bytes    = (int) config_get("asm.bytes");
	show_offset   = (int) config_get("asm.offset");
	show_splits   = (int) config_get("asm.split");
	show_flags    = (int) config_get("asm.flags");
	show_flagsline= (int) config_get("asm.flagsline");
	show_lines    = (int) config_get("asm.lines");
	show_reladdr  = (int) config_get("asm.reladdr");
	show_traces   = (int) config_get("asm.trace");
	show_comments = (int) config_get("asm.comments");
	show_nbytes   = (int) config_get_i("asm.nbytes");
	endian        = (int) config_get("cfg.endian");
	color         = (int) config_get("scr.color");

	jump_n = 0;
	length = len;
	ud_idx = 0;
	delta = 0;
	inc = 0;

	if (config.visual && rows<1)
		rows = config.height - ((last_print_format==FMT_VISUAL)?11:0) - config.lines;

	/* XXX dupped move */
	switch(arch) {
	case ARCH_X86:
		udis_init();
		ud_set_pc(&ud_obj, config.seek);
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

	// XXX remove rows
	myinc = 0;
	if (rrows>0) rrows ++;
	/* DISASSEMBLY LOOP */
	while (!config.interrupted) {
		CHECK_LINES
		if (bytes >= length ) break;
		if (rrows>0 && --rrows == 0) break;
		if (bytes>=config.block_size)
			break;
		/* XXX : diff between sk and seek?? */
		//sk = seek = config.baddr + config.seek+bytes;
		seek = config.seek+bytes + config.baddr;
		sk   = config.seek+bytes;
		CHECK_LINES

		if (show_comments)
			metadata_print(bytes);

		/* handle data type block */
		struct data_t *foo = data_get_range(sk);
		if (foo != NULL) {
			int dt = foo->type;
			idata = foo->to-sk-1;
			myinc = idata;
			if (show_lines)
				code_lines_print(reflines, seek, 0);
			if (show_offset)
				print_addr(seek);
			if (show_reladdr) {
				if (bytes==0) cons_printf("%08llX ", seek);
				else cons_printf("+%7d ", bytes);
			}

			flag = flag_name_by_offset(seek);
			if (flag == NULL && config.baddr)
				flag = flag_name_by_offset(seek-config.baddr);
			if (!strnull(flag))
				cons_printf("%s: ", flag);

			if (foo->from != sk)
				cons_printf("<< %d <<", sk-foo->from);

			switch(foo->type) {
			case DATA_FOLD_C: 
				cons_printf("  { 0x%llx-0x%llx %lld(%d) }", foo->from, foo->to, foo->size, myinc);
				break;
			case DATA_FOLD_O:
				cons_strcat("\r                                       \r");
				if (show_lines)
					code_lines_print(reflines, seek, 1);
				if (show_reladdr)
					cons_printf("        ");
				if (show_offset)
					cons_strcat("            ");
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
				int w = 0;
				for(i=0;i<idata;i++) {
					print_color_byte_i(bytes+i,"%02x ", config.block[bytes+i]);
					w+=4;
					if (w >= config.height) {
						cons_printf("\n");
						if (show_lines)
							code_lines_print(reflines, sk+i, 1);
						if (show_reladdr)
							cons_printf("        ");
						if (show_offset)
							print_addr(sk+i);
						cons_printf("  .db  ");
						w = 0;
					}
				}
				break;
			}
			cons_newline();
			CHECK_LINES
			bytes+=idata+1;
			ud_idx+=idata;
			myinc = 0;
			continue;
		}
		__outofme:
		if (data_end(sk) == DATA_FOLD_O) {
			if (show_lines)
				code_lines_print(reflines, seek, 1);
			if (show_offset)
				cons_strcat("           ");
			folder--;
			for(i=0;i<folder;i++) cons_strcat("  ");
			cons_strcat("   }\n");
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
			char buf[128];
			sprintf(buf, "%lld", seek);
			setenv("HERE", buf, 1);
			radare_cmd(cmd_asm, 0);
		}
		if (arch==ARCH_AOP)
			arch = last_arch;
		
		// TODO: Use a single arch_aop here
		switch(arch) {
			case ARCH_X86:
				if (ud_disassemble(&ud_obj)<1)
					return;
				//arch_x86_aop((unsigned long)ud_insn_off(&ud_obj), (const unsigned char *)config.block+bytes, &aop);
				//ud_set_pc(&ud_obj, seek);
				//arch_x86_aop((unsigned long)ud_insn_off(&ud_obj), (const unsigned char *)b, &aop);
				arch_x86_aop((u64)seek, (const unsigned char *)b, &aop);
				myinc += ud_insn_len(&ud_obj);
				break;
			case ARCH_ARM16:
				arm_mode = 16;
				myinc += 2;
				arch_arm_aop(seek, (const unsigned char *)b, &aop);
				break;
			case ARCH_ARM:
				myinc += arch_arm_aop(seek, (const unsigned char *)b, &aop);
				break;
			case ARCH_MIPS:
				arch_mips_aop(seek, (const unsigned char *)b, &aop);
				myinc += aop.length;
				break;
			case ARCH_SPARC:
				arch_sparc_aop(seek, (const unsigned char *)b, &aop);
				myinc += aop.length;
				break;
			case ARCH_JAVA:
				arch_java_aop(seek, (const unsigned char *)config.block+bytes, &aop);
				myinc += aop.length;
				break;
			case ARCH_PPC:
				arch_ppc_aop(seek, (const unsigned char *)b, &aop);
				myinc += aop.length;
				break;
			case ARCH_CSR:
				arch_csr_aop(seek, (const unsigned char *)b, &aop);
				myinc += 2;
				break;
			default:
				// Uh?
				myinc += 4;
				break;
		}
		if (myinc<1)
			myinc = 1;

		if (config.cursor_mode) {
			if (config.cursor == bytes)
				inc = myinc;
		} else {
			if (inc == 0)
				inc = myinc;
		}
		D { 
			// TODO autodetect stack frames here !! push ebp and so... and wirte a comment
			if (show_lines)
				code_lines_print(reflines, sk, 0);

			if (show_offset) {
				print_addr(seek);
				//C cons_printf(C_GREEN"0x%08llX "C_RESET, (unsigned long long)(seek));
				//else cons_printf("0x%08llX ", (unsigned long long)(seek));
			}
			if (show_reladdr) {
				if (bytes==0) cons_printf("%08llX ", seek);
				else cons_printf("+%7d ", bytes);
			}
			/* size */
			if (show_size)
				cons_printf("%d ", aop.length); //dislen(config.block+seek));
			/* trac information */
			if (show_traces)
				cons_printf("%04x %02x ", trace_count(seek), trace_times(seek));

			if (show_flags && !show_flagsline) {
				char buf[1024];
				const char *flag = flag_name_by_offset( seek );
				//cons_printf("(%08x) ", seek-config.baddr);
				if (flag == NULL || !flag[0])
					flag = flag_name_by_offset(seek -config.baddr);
				//if (flag == NULL)
				//	flag = flag_name_by_offset(seek +config.baddr);
				//config.baddr?(config.seek+bytes-myinc-myinc):seek);
				if (flag && flag[0]) {
					sprintf(buf, "%%%ds:", show_nbytes);
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
						if (show_reladdr)
							cons_printf("        ");
						sprintf(buf, "%%%ds ", show_nbytes);
						cons_printf(buf,"");
					} else {
						cons_printf(buf, flag);
					}
				} else {
					sprintf(buf, "%%%ds ", show_nbytes);
					cons_printf(buf,flag);
				}
			}

			/* cursor and bytes */
			D switch(is_cursor(bytes,myinc)) {
			case 0:
 				cons_printf(" ");
				break;
			case 1:
 				cons_printf("*");
				break;
			case 2:
 				cons_printf(">");
				break;
			}
			if (show_bytes) {
				int max = show_nbytes;
				int cur = myinc;
				if (cur + 1> show_nbytes)
					cur = show_nbytes - 1;

				for(i=0;i<cur; i++)
					print_color_byte_i(bytes+i, "%02x", b[i]);
				if (cur !=myinc)
					max--;
				for(i=(max-cur)*2;i>0;i--)
					cons_printf(" ");
				if (cur != myinc)
					cons_printf(". ");
			}
			C switch(aop.type) {
			case AOP_TYPE_CMP: // cons_strcat("\e[36m");
				cons_strcat(cons_palette[PAL_CMP]);
				break;
			case AOP_TYPE_REP:
			case AOP_TYPE_JMP:
			case AOP_TYPE_CJMP:
				cons_strcat(cons_palette[PAL_JUMP]);
				break;
			case AOP_TYPE_CALL: // green cons_strcat("\e[32m");
				cons_strcat(cons_palette[PAL_CALL]);
				break;
			case AOP_TYPE_NOP: // cons_strcat("\e[35m");
				cons_strcat(cons_palette[PAL_NOP]);
				break;
			case AOP_TYPE_UPUSH:
			case AOP_TYPE_PUSH:
			case AOP_TYPE_POP: // yellow cons_strcat("\e[33m");
				cons_strcat(cons_palette[PAL_PUSH]);
				break;
			case AOP_TYPE_SWI:
			//case AOP_TYPE_UNK:
			case AOP_TYPE_TRAP:
			case AOP_TYPE_UJMP: //red cons_strcat("\e[31m");
				cons_strcat(cons_palette[PAL_TRAP]);
				break;
			case AOP_TYPE_RET: // blue cons_strcat("\e[34m");
				cons_strcat(cons_palette[PAL_RET]);
				break;
			}

			/* disassemble opcode! */
#if 0
cons_printf("DIS at 0x%08llx\n", seek);
cons_printf("BYTES at 0x%08lld\n", (u64)bytes+idata);
cons_printf("MYINC at 0x%08lld\n", (u64)myinc);
cons_printf("MYINC at 0x%02x %02x %02x\n", config.block[bytes],
	config.block[bytes+1], config.block[bytes+2]);

#endif
			/* XXX: The seek+myinc is weird but MANDATORY, the first opcode must
			 * be used to get the 'base incremental' address for calculating the
			 * rest of opcodes, or the jumps will be addr-myinc and only the *first*
			 * opcode will be disassmbled correctly.
			 *
			 * I know that this has no sense, but computer science is not an exact one.
			 */
			udis_arch_opcode(arch, endian, sk, bytes, myinc); //seek+myinc, bytes, myinc);

			/* show references */
			if (aop.ref) {
				if (string_flag_offset(buf, aop.ref-config.baddr));
					cons_printf(" ; %s",buf);
			}

			/* show comments and jump keys */
			if (show_comments) {
				if (aop.jump) {
					if (++jump_n<10) {
						jumps[jump_n-1] = aop.jump;
						if (string_flag_offset(buf, aop.jump))
							cons_printf("  ; %d = %s", jump_n,buf);
						else cons_printf("  ; %d = 0x%08llx", jump_n, aop.jump);
					} else {
						if (string_flag_offset(buf, aop.jump))
							cons_printf("  ; %s", buf);
						else cons_printf("  ; 0x%08llx", aop.jump);
					}
				}
			}

			/* show splits at end of code blocks */
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
			udis_arch_opcode(arch, endian, sk, bytes, myinc);
		}

		NEWLINE;
		bytes+=myinc;
		myinc = 0;
	}

	radare_controlc_end();
}

/* aop disassembler */
int arch_aop_aop(u64 addr, const u8 *bytes, struct aop_t *aop)
{
	switch(aop->type) {
	case AOP_TYPE_NOP:
		cons_printf("nop");
		break;
	case AOP_TYPE_RET:
		cons_printf("ret");
		break;
	case AOP_TYPE_CALL:
		cons_printf("call 0x%08llx", aop->jump);
		break;
	case AOP_TYPE_JMP:
		cons_printf("jmp 0x%08llx", aop->jump);
		break;
	case AOP_TYPE_CJMP:
		cons_printf("cjmp 0x%08llx", aop->jump);
		break;
	}
	return aop->length;
}

struct radis_arch_t {
	const char *name;
	int id;
	int (*fun)(u64 addr, u8 *bytes, struct aop_t *aop);
} radis_arches [] = {
	{ "intel"   , ARCH_X86   , &arch_x86_aop }   , 
	{ "intel16" , ARCH_X86   , &arch_x86_aop }   , 
	{ "intel32" , ARCH_X86   , &arch_x86_aop }   , 
	{ "intel64" , ARCH_X86   , &arch_x86_aop }   , 
	{ "x86"     , ARCH_X86   , &arch_x86_aop }   , 
	{ "mips"    , ARCH_MIPS  , &arch_mips_aop }  , 
	//{ "aop"   , ARCH_AOP   , &arch_aop_aop }   , 
	{ "arm"     , ARCH_ARM   , &arch_arm_aop }   , 
	{ "arm16"   , ARCH_ARM16 , &arch_arm_aop }   , 
	{ "java"    , ARCH_JAVA  , &arch_java_aop }  , 
	{ "sparc"   , ARCH_SPARC , &arch_sparc_aop } , 
	{ "ppc"     , ARCH_PPC   , &arch_ppc_aop }   , 
	{ "m68k"    , ARCH_M68K  , &arch_m68k_aop }  , 
	{ "csr"     , ARCH_CSR   , &arch_csr_aop }   , 
	{ NULL      , -1         , NULL }
};

void radis_update_i(int id)
{
	int i;
	for(i=0;radis_arches[i].name;i++) {
		if (radis_arches[i].id == id) {
			config.arch = radis_arches[i].id;
			arch_aop    = radis_arches[i].fun;
			break;
		}
	}
}

void radis_update()
{
	int i;
	char *arch = config_get("asm.arch");
	for(i=0;radis_arches[i].name;i++) {
		if (!strcmp(arch, radis_arches[i].name)) {
			config.arch = radis_arches[i].id;
			arch_aop    = radis_arches[i].fun;
			break;
		}
	}
}

void radis(int len, int rows)
{
	/* not always necesary */
	// radis_update();

	radare_controlc();
	udis_arch(config.arch, len, rows);
	radare_controlc_end();
}
