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

/* TODO: make the disassembly stuff more generic, avoid code dupz */


#include "main.h"
#include "code.h"
#include "flags.h"
#include "arch/arm/disarm.h"
/* http://devnull.owl.de/~frank/Disassembler_e.html */
#include "arch/ppc/ppc_disasm.h"
#include "arch/m68k/m68k_disasm.h"
#include "list.h"

static int lines = 0;

struct list_head comments;

void metadata_comment_add(u64 offset, const char *str)
{
	struct comment_t *cmt = (struct comment_t *)
		malloc(sizeof(struct comment_t));
	cmt->offset = offset;
	cmt->comment = strdup(str);
	if (cmt->comment[strlen(cmt->comment)-1]=='\n')
		cmt->comment[strlen(cmt->comment)-1]='\0';
	list_add_tail(&(cmt->list), &(comments));
}

void metadata_comment_del(u64 offset)
{
}

char *metadata_comment_list()
{
	struct list_head *pos;
	list_for_each(pos, &comments) {
		struct comment_t *cmt = list_entry(pos, struct comment_t, list);
		cons_printf("0x"OFF_FMTx" %s\n", cmt->offset, cmt->comment);
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
			if (cmt->offset == offset) {
				i++;
			}
		}
		if (i>cmtlines) {
			cmtlines = i-cmtlines;
		}
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
			lines++;
		}
	}
	return str;
}

void metadata_comment_init(int new)
{
	INIT_LIST_HEAD(&(comments));
}

static int print_metadata(int delta)
{
	FILE *fd;
	int lines = 0;
	u64 off = 0;
	char *ptr,*ptr2;
	char buf[4096];
	char *rdbfile;
	int i;
	u64 offset = (u64)config.seek + (u64)delta; //(u64)ud_insn_off(&ud_obj);
	//	u64 seek = config.baddr + (u64)delta; //ud_insn_off(&ud_obj);

// config.baddr everywhere???
	D {} else return 0;
	ptr = flag_name_by_offset( offset );
	if (ptr[0]) {
		C	cons_printf(C_RESET C_BWHITE""OFF_FMT" %s:"C_RESET"\n",
				config.baddr+offset, ptr);
		else	cons_printf(OFF_FMTs" %s:\n",
				config.baddr+offset, ptr);
		lines++;
	}

	ptr = metadata_comment_get(offset);
	if (ptr && ptr[0]) {
		C 	cons_printf(C_MAGENTA"%s"C_RESET, ptr);
		else 	cons_printf("%s", ptr);
		free(ptr);
	}
#if 0
	/* comments */
	rdbfile = config_get("file.rdb");
	fd = fopen(rdbfile,"r");
	if (fd == NULL)
		return lines;	

	while(!config.interrupted && !feof(fd)) {
		buf[0]='\0';
		fgets(buf, 1023, fd);
		if (buf[0]=='\0'||feof(fd)) break;
		buf[strlen(buf)-1]='\0';

		ptr = strchr(buf, '=');
		if (!ptr) continue;
		ptr[0]='\0'; ptr = ptr + 1;

		if (!strcmp(buf, "comment")) {
			ptr2 = strchr(ptr, ' ');
			if (!ptr2) continue;
			ptr2[0]='\0'; ptr2=ptr2+1;
			off = get_offset(ptr);
			if (offset == off) {
				for(i=0;i<cmtmargin;i++) pstrcat(" ");
				C 	cons_printf(C_BWHITE"  ; %s"C_RESET, ptr2);
				else 	cons_printf("  ; %s", ptr2);
				NEWLINE;
				lines++;
			}
		}
	}
	fclose(fd);
	INILINE;
#endif

	return lines;
}

#include "arch/x86/udis86/types.h"
#include "arch/x86/udis86/extern.h"

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

void udis_init()
{
	char *syn = config_get("asm.syntax");
	char *ptr = config_get("asm.arch");

	ud_init(&ud_obj);

	if (!strcmp(ptr, "intel16")) {
		ud_set_mode(&ud_obj, 16);
	} else {
		if((!strcmp(ptr, "intel"))
				|| (!strcmp(ptr, "intel32"))) {
			ud_set_mode(&ud_obj, 32);
		} else {
			if (!strcmp(ptr, "intel64")) {
				ud_set_mode(&ud_obj, 64);
			} else
				ud_set_mode(&ud_obj, 32);
		}
	}

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

void udis(int len, int rows)
{
	struct aop_t aop;
	char* hex1, *hex2;
	char c;
	int i,delta;
	int seek = 0;
	int lines = 0;
	int bytes = 0;
	int show_bytes;
	int show_offset;
	int show_splits;
	int show_comments;
	int show_lines;
	int show_traces;
	int nbytes = 12;
	int show_size = config_get("asm.size");
	u64 myinc = 0;
	struct reflines_t *reflines = NULL;
	char *follow; 
	jump_n = 0;

	len*=2; // uh?!

	length = len;
	ud_idx = 0;
	inc = 0;

	if (rows<0)
		rows = 1;

	udis_init();
	/* disassembly loop */
	ud_obj.pc = config.seek;
	delta = 0;
	show_bytes = config_get("asm.bytes");
	show_offset = config_get("asm.offset");
	show_splits = config_get("asm.split");
	show_lines = config_get("asm.lines");
	show_traces = config_get("asm.trace");
	show_comments = config_get("asm.comments");
	nbytes = (int)config_get_i("asm.nbytes");

	if (nbytes>8 ||nbytes<0)
		nbytes = 8;
	if (show_lines)
		reflines = code_lines_init();
	radare_controlc();

	while (!config.interrupted && ud_disassemble(&ud_obj)) {
		if ( (config.visual && len!=config.block_size) && (++lines>=(config.height-2)))
			return;
		myinc = ud_insn_len(&ud_obj);
		if (config.cursor_mode) {
			if (config.cursor == bytes)
				inc = myinc;
		} else
			if (inc == 0)
				inc = myinc;
		length-=myinc;
		bytes+=myinc;
		if (length<=0)
			return;

		INILINE;
		D { 
			if (show_comments)
				lines+=print_metadata(seek);

			// TODO autodetect stack frames here !! push ebp and so... and wirte a comment
			if (show_lines)
				code_lines_print(reflines, config.baddr+ud_insn_off(&ud_obj));

			if (show_offset) {
				C cons_printf(C_GREEN"0x%08llX "C_RESET, (unsigned long long)(config.baddr + ud_insn_off(&ud_obj)));
				else cons_printf("0x%08llX ", (unsigned long long)(config.baddr + ud_insn_off(&ud_obj)));
			}
			if (show_size)
				cons_printf("%d ", dislen(config.block+seek));
			if (show_traces) {
				cons_printf("%02d %02d ",
					trace_count(config.baddr + ud_insn_off(&ud_obj)),
					trace_times(config.baddr + ud_insn_off(&ud_obj)));
			}
			if (show_bytes) {
				int max = nbytes;
				int cur = myinc;
				if (cur + 1> nbytes)
					cur = nbytes - 1;

				for(i=0;i<cur; i++)
					print_color_byte_i(seek+i, "%02x", config.block[seek+i]); //ud_obj.insn_hexcode[i]);
				if (cur !=myinc)
					max--;
				for(i=(max-cur)*2;i>0;i--)
					cons_printf(" ");
				if (cur != myinc)
					cons_printf(". ");
			}
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
			C cons_printf(C_RESET);

			arch_x86_aop((unsigned long)ud_insn_off(&ud_obj), (const unsigned char *)config.block+bytes-myinc, &aop);
			if (aop.jump) {
				if (++jump_n<10) {
					jumps[jump_n-1] = aop.jump;
					cons_printf("\e[0m   [%d] (0x%llx)", jump_n,jumps[jump_n-1]);
				}
			}

			if (show_splits) {
				if (aop.jump||aop.eob) {
					NEWLINE
					cons_printf("; ------------------------------------ ");
					lines++;
				}
			}
		} 
		else cons_printf("%s", ud_insn_asm(&ud_obj));
		seek+=myinc;
		NEWLINE;
		if (rows && rows == lines)
			return;
	}
	radare_controlc_end();
}

extern int color;
void udisarm(int len, int rows)
{
	int i, endian;
	int show_bytes = config_get("asm.bytes");
	int show_offset = config_get("asm.offset");
	int show_split = config_get("asm.split");
	int show_lines = config_get("asm.lines");
	int show_size = config_get("asm.size");
	struct reflines_t *reflines = NULL;
	color = config_get("scr.color");

	if (show_lines)
		reflines = code_lines_init();

	inc = 4; // TODO : global var. should live inside config.inc!!
	lines = 0;
	endian = config_get("cfg.endian");
	for(i=0;!config.interrupted && i<=len;i+=4) {
		unsigned long ins;
		if ( (config.visual) && (++lines>=(config.height-2)))
			return;
		if (endian)
			ins = (config.block[i]<<24)+ (config.block[i+1]<<16)+ (config.block[i+2]<<8)+ (config.block[i+3]);
		else	ins = (config.block[i+3]<<24)+ (config.block[i+2]<<16)+ (config.block[i+1]<<8)+ (config.block[i+0]);
		INILINE;
		lines+=print_metadata(i);
		if (show_lines)
			code_lines_print(reflines, config.baddr+config.seek+i);
		if (show_offset) {
			C cons_printf(C_GREEN);
			cons_printf("0x%08llX ", config.baddr+ config.seek+i);
			C cons_printf(C_RESET);
		}
		if (show_size)
			cons_printf("4 ");
		if (show_bytes) {
			if (endian) {
				print_color_byte_i(i, "%02x", config.block[i]);
				print_color_byte_i(i+1, "%02x", config.block[i+1]);
				print_color_byte_i(i+2, "%02x", config.block[i+2]);
				print_color_byte_i(i+3, "%02x", config.block[i+3]);
			} else {
				print_color_byte_i(i+3, "%02x", config.block[i+3]);
				print_color_byte_i(i+2, "%02x", config.block[i+2]);
				print_color_byte_i(i+1, "%02x", config.block[i+1]);
				print_color_byte_i(i, "%02x", config.block[i]);
			}
		}
		cons_printf("    %s", disarm(ins, (int)(config.seek+i)));
		C cons_printf(C_RESET);
		if (show_split) {
			struct aop_t aop;
			arch_arm_aop((unsigned long)config.seek+i, (const unsigned char *)config.block+i, &aop);
			if (aop.jump||aop.eob) {
				NEWLINE
				cons_printf("; ------------------------------------ ");
				lines++;
			}
		}
		NEWLINE;
	}
	code_lines_free(reflines);
}

void ppc_disassemble(int len, int rows)
{
	int i, endian;
	int show_bytes  = config_get("asm.bytes");
	int show_offset = config_get("asm.offset");
	int show_split  = config_get("asm.split");
	int show_lines = config_get("asm.lines");
	int show_size = config_get("asm.size");
	char opcode[10];
	char operands[24];
	struct DisasmPara_PPC dp;
	struct reflines_t *reflines = NULL;

	/* initialize DisasmPara */
	dp.opcode = opcode;
	dp.operands = operands;
	if (show_lines)
		reflines = code_lines_init();

	inc = 4; // TODO : global var. should live inside config.inc!!
	lines = 0;
	endian = config_get("cfg.endian");
	for(i=0;!config.interrupted && i<=len;i+=4) {
		unsigned long ins;
		if ( (config.visual) && (++lines>=(config.height-2)))
			return;
		if (endian)
			ins = (config.block[i]<<24)+ (config.block[i+1]<<16)+ (config.block[i+2]<<8)+ (config.block[i+3]);
		else	ins = (config.block[i+3]<<24)+ (config.block[i+2]<<16)+ (config.block[i+1]<<8)+ (config.block[i+0]);
		INILINE;
		lines+=print_metadata(i);
		if (show_lines)
			code_lines_print(reflines, config.baddr+config.seek+i);
		if (show_offset) {
			C cons_printf(C_GREEN);
			cons_printf("0x%08llX ", config.baddr+ config.seek+i);
			C cons_printf(C_RESET);
		}
		if (show_size)
			cons_printf("4 ");
		if (show_bytes) {
			if (endian) {
				print_color_byte_i(i+3, "%02x", config.block[i+3]);
				print_color_byte_i(i+2, "%02x", config.block[i+2]);
				print_color_byte_i(i+1, "%02x", config.block[i+1]);
				print_color_byte_i(i, "%02x", config.block[i]);
			} else {
				print_color_byte_i(i, "%02x", config.block[i]);
				print_color_byte_i(i+1, "%02x", config.block[i+1]);
				print_color_byte_i(i+2, "%02x", config.block[i+2]);
				print_color_byte_i(i+3, "%02x", config.block[i+3]);
			}
		}
		dp.iaddr = config.baddr + config.seek + i;
		dp.instr = config.block + i;
		PPC_Disassemble(&dp, endian);
		cons_printf("    %s %s", opcode, operands);
		C cons_printf(C_RESET);
		if (show_split) {
			struct aop_t aop;
			arch_ppc_aop((unsigned long)config.seek+i, (const unsigned char *)config.block+i, &aop);
			if (aop.jump||aop.eob) {
				NEWLINE
					cons_printf("; ------------------------------------ ");
				lines++;
			}
		}
		NEWLINE;
	}
	code_lines_free(reflines);
}

void m68k_disassemble(int len, int rows)
{
	int i, endian;
	int show_bytes  = config_get("asm.bytes");
	int show_offset = config_get("asm.offset");
	int show_split  = config_get("asm.split");
	int show_lines = config_get("asm.lines");
	int show_comments = config_get("asm.comments");
	char opcode[10];
	char operands[24];
  	struct DisasmPara_68k dp;
	struct reflines_t *reflines = NULL;

	/* initialize DisasmPara */
	dp.opcode = opcode;
	dp.operands = operands;
	if (show_lines)
		reflines = code_lines_init();

	inc = 4; // TODO : global var. should live inside config.inc!!
	lines = 0;
	endian = config_get("cfg.endian");
	for(i=0;!config.interrupted && i<=len;i+=4) {
		unsigned long ins;
		if ( (config.visual) && (++lines>=(config.height-2)))
			return;
		if (endian)
			ins = (config.block[i]<<24)+ (config.block[i+1]<<16)+ (config.block[i+2]<<8)+ (config.block[i+3]);
		else	ins = (config.block[i+3]<<24)+ (config.block[i+2]<<16)+ (config.block[i+1]<<8)+ (config.block[i+0]);
		INILINE;
		lines+=print_metadata(i);
		if (show_lines)
			code_lines_print(reflines, config.baddr+config.seek +i);
		if (show_offset) {
			C cons_printf(C_GREEN);
			cons_printf("0x%08llX ", config.baddr+ config.seek+i);
			C cons_printf(C_RESET);
		}
		if (show_bytes) {
			print_color_byte_i(i, "%02x", config.block[i]);
			print_color_byte_i(i+1, "%02x", config.block[i+1]);
			print_color_byte_i(i+2, "%02x", config.block[i+2]);
			print_color_byte_i(i+3, "%02x", config.block[i+3]);
		}
		dp.iaddr = config.baddr + config.seek + i;
		dp.instr = config.block + i;
		// XXX read vda68k: this fun returns something... size of opcode?
    		M68k_Disassemble(&dp);
		cons_printf("    %s %s", opcode, operands);
		C cons_printf(C_RESET);
		if (show_split) {
			struct aop_t aop;
			arch_arm_aop((unsigned long)config.seek+i, (const unsigned char *)config.block+i, &aop);
			if (aop.jump||aop.eob) {
				NEWLINE
					cons_printf("; ------------------------------------ ");
				lines++;
			}
		}
		NEWLINE;
	}
	code_lines_free(reflines);
}

void java_disassemble(int len, int rows)
{
	char output[1024];
	int lines = 0;
	int i, j, l, oi;
	int show_bytes = config_get("asm.bytes");
	int show_offset = config_get("asm.offset");
	int show_split = config_get("asm.split");
	int show_lines = config_get("asm.lines");
	int show_size = config_get("asm.size");
	struct reflines_t *reflines = NULL;

	if (show_lines)
		reflines = code_lines_init();

	inc = -1;
	for(i = 0; !config.interrupted && i < len;) {
		if ((++lines>(config.height-2)))
			return;
		if (rows&&lines>=rows)
			return;
		INILINE;
		lines+=print_metadata(i);
		if (show_lines)
			code_lines_print(reflines, config.baddr+config.seek+i);
		l = java_disasm(config.block+i, output);
		if (show_size)
			cons_printf("%d ", l);
		if (show_offset) {
			C cons_printf(C_GREEN);
			cons_printf("0x%08llX ", config.baddr + config.seek+i); // TODO: use baddr too!
			C cons_printf(C_RESET);
		}
		if (config.cursor_mode) {
			if (config.cursor == i)
				if (inc == -1) inc = l;
		} else
			if (inc == -1) inc = l;
		oi = i;
		if (l >0) {
			if (show_bytes) {
				for (j = 0;j<l;j++)
					print_color_byte_i(i+j,   "%02x", config.block[i+j]);
				for (j = l ;j<5;j++)
					cons_printf("  ");
			}
			cons_printf("    %s", output);
			i += l;
		} else {
			print_color_byte_i(i,   "%02x", config.block[i]);

			cons_printf("            ???");
			i++; // skip wrong byte
		}
		C cons_printf(C_RESET);
		NEWLINE;
		if (show_split) {
			struct aop_t aop;
			arch_java_aop((unsigned long)config.seek+oi, (const unsigned char *)config.block+oi, &aop);
			if (aop.jump||aop.eob) {
				cons_printf("; ------------------------------------ ");
				lines++;
				NEWLINE
			}
		}
	}
	code_lines_free(reflines);
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
	if (!memcmp(ptr, "intel", 5)) {
		udis(len,rows);
	} else {
		if (!strcmp(ptr, "arm")) {
			udisarm(len,rows);
		} else {
			if (!strcmp(ptr, "java")) {
				java_disassemble(len,rows);
			} else {
				if (!strcmp(ptr, "ppc")) {
					ppc_disassemble(len, rows);
				} else {
					if (!strcmp(ptr, "m68k"))
						m68k_disassemble(len, rows);
				}
			}
		}
	}
	radare_controlc_end();
	fflush(stdout);
}
