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
#include "data.h"
#include "undo.h"
#include "flags.h"
#include "arch/csr/dis.h"
#include "arch/arm/disarm.h"
#include "arch/msil/demsil.h"
/* http://devnull.owl.de/~frank/Disassembler_e.html */
#include "arch/ppc/ppc_disasm.h"
#include "arch/m68k/m68k_disasm.h"
#include "arch/x86/udis86/types.h"
#include "arch/x86/udis86/extern.h"
#include "list.h"

//#define CHECK_LINES if ( config.visual && len != config.block_size && (cons_lines > config.height) ) break;
#define CHECK_LINES if ( config.visual && (cons_lines > config.height) ) break;

static int last_arch = ARCH_X86;
extern int force_thumb;
static char funline[3];
extern struct reflines_t *reflines;

static ud_t ud_obj;
static int length = 0;
/* XXX UD_IDX SHOULD BE REPLACED BY 'BYTES'. THEY HAVE THE SAME FINALITY. DRY! */
static int ud_idx = 0;
// XXX udis does not needs to use this !!!
int udis86_color = 0;

extern int arm_mode;
extern int mips_mode;
int ollyasm_enable = 0;


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
		ollyasm_enable = 0;
		if (!strcmp(syn,"olly")) {
			ollyasm_enable = 1;
		} else
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
		if (ollyasm_enable) {
#include "arch/x86/ollyasm/disasm.h"
			t_disasm da;
			ret = Disasm(b, seek, seek, &da, DISASM_CODE);
			cons_printf("%-24s", da.result);
			if (da.comment[0])
				cons_printf("; %s", da.comment);
		} else {
			ud_obj.insn_offset = seek+myinc; //+bytes;
			ud_obj.pc = seek+myinc;
			ret = ud_insn_len(&ud_obj);
			cons_printf("%-24s", ud_insn_asm(&ud_obj));
		}
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
{
	char bu[4];
	
	       //unsigned long ins = (b[0]<<24)+(b[1]<<16)+(b[2]<<8)+(b[3]);
	       //cons_printf("  %s", disarm(ins, (unsigned int)seek));
		endian_memcpy(&bu, b, 4); //, endian);
	       ret=gnu_disarm((unsigned char*)bu, (u64)seek);
}
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
		{
		char buf[1024];
		endian_memcpy(buf, b, 4);
		dp.instr = buf;
	      // dp.instr = b; //config.block + i;
	       PPC_Disassemble(&dp, endian);
	       cons_printf("  %s %s", opcode, operands);
		}
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
	case ARCH_MSIL: {
		int n;
		DISASMSIL_OFFSET CodeBase = seek;
		ILOPCODE_STRUCT ilopar[8]; // XXX only uses 1
		DisasMSIL(b,bytes,CodeBase,ilopar, 8, &n);
		cons_printf("%s", ilopar[0].Mnemonic);
		ret = ilopar[0].Size;
		} break;
	case ARCH_BF:
		ret = arch_bf_dis(b, seek, 1024);
		break;
	default:
		cons_printf("Unknown architecture");
		break;
	}
	C cons_printf(C_RESET);
	return ret;
}

extern int color;
void udis_arch(int arch, int len, int rows)
{
	udis_arch_buf(arch, config.block, len, rows);
}

void udis_arch_buf(int arch, const u8 *block, int len, int rows)
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
	flag_t *f = NULL;
	int rrows = rows; /* rrows is in reality the num of bytes to be disassembled */
	int endian;
	int show_size, show_bytes, show_offset,show_splits,show_comments,show_lines,
	show_traces,show_nbytes, show_flags, show_reladdr, show_flagsline, show_functions;
	int folder = 0; // folder level

	cmd_asm       = config_get("cmd.asm");
	show_size     = (int) config_get("asm.size");
	show_bytes    = (int) config_get("asm.bytes");
	show_offset   = (int) config_get("asm.offset");
	show_functions= (int) config_get("asm.functions");
	show_splits   = (int) config_get("asm.split");
	show_flags    = (int) config_get("asm.flags");
	show_flagsline= (int) config_get("asm.flagsline");
	show_lines    = (int) config_get_i("asm.lines");
	show_reladdr  = (int) config_get("asm.reladdr");
	show_traces   = (int) config_get("asm.trace");
	show_comments = (int) config_get("asm.comments");
	show_nbytes   = (int) config_get_i("asm.nbytes");
	endian        = (int) config_get("cfg.bigendian");
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

	data_reflines_init();
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
		seek = config.seek+bytes + config.baddr;
		sk   = config.seek+bytes;
		CHECK_LINES

		D {
			if (show_comments)
				data_printd(bytes);
			if (reflines && show_lines) // does nothing if not data
				code_lines_print(reflines, sk, 0);
		}
		if (show_offset)
			print_addr(seek);
		if (show_reladdr) {
			if (bytes==0) cons_printf("%08llX ", seek);
			else cons_printf("+%7d ", bytes);
		}
		/* size */
		if (show_size)
			cons_printf("%d ", aop.length);
		/* trac information */
		if (show_traces)
			cons_printf("%04x %02x ", trace_count(seek), trace_times(seek));

		struct data_t *foo = data_get_range(sk);
		funline[0]='\0';
		/* handle data type block */
		//	print_function_line(foo, sk);
		if (show_functions) {
			if (foo != NULL && foo->type == DATA_FUN) {
				if(show_functions) {
//eprintf("%llx %llx\n", (u64)foo->to-aop.length,(u64)sk);
//eprintf("%llx %llx\n", (u64)foo->to,(u64)sk);
					if (foo->from == sk)
						strcpy(funline,"/");
					else
					//if (foo->to-aop.length== sk)
					//if (foo->to-aop.length== sk)
					if ((foo->to-aop.length+1 == sk) // XXX UGLY HACK
					|| (foo->to-1 == sk))
						strcpy(funline,"\\");
					else
						strcpy(funline,"|");
					cons_strcat(funline);
				}
				foo = NULL;
			} else
				cons_strcat(" ");
		}
		if (foo != NULL) {
			int dt = foo->type;
			idata = foo->to-sk;
			myinc = idata;

			flag = flag_name_by_offset(seek);
			if (flag == NULL && config.baddr)
				flag = flag_name_by_offset(seek-config.baddr);
			if (!strnull(flag))
				cons_printf("%s: ", flag);

			//if (foo->from != sk)
			//	cons_printf("<< %d <<", sk-foo->from);

			switch(foo->type) {
			case DATA_FOLD_C: 
				cons_printf("  { 0x%llx-0x%llx %lld(%d) }", foo->from, foo->to, foo->size, myinc);
				break;
			case DATA_FOLD_O:
				if (foo->from == sk) {
					for(i=0;i<folder;i++)
						cons_strcat("  ");
					cons_strcat("  {\n");
						if (show_lines && reflines) 
							code_lines_print(reflines, sk+i, 1);
						if (show_reladdr)
							cons_printf("        ");
						if (show_offset)
							print_addr(sk+i);
					CHECK_LINES
					folder++;
				}
				myinc = 0;
				idata = 0;
				goto __outofme;
			case DATA_STR:
				cons_strcat("  .string \"");
				for(i=0;i<idata;i++) {
					print_color_byte_i(bytes+i, "%c",
						is_printable(block[bytes+i])?block[bytes+i]:'.');
				}
				cons_strcat("\"");
				break;
			case DATA_STRUCT:
				if (*foo->arg=='\0') {
					cons_printf("(struct: undefined memory format)\n");
				} else {
					int ofmt = last_print_format;
					cons_printf("(pm %s)\n", foo->arg);
					/* TODO: implement scrolling delta */
					radare_cmdf("pm %s @ 0x%08llx", foo->arg, foo->from);
					last_print_format = ofmt;
				}
				break;
			case DATA_HEX:
			default:
				cons_strcat("  .db  ");
				int w = 0;
				for(i=0;i<idata;i++) {
					print_color_byte_i(bytes+i,"%02x ", block[bytes+i]);
					w+=4;
					if (w >= config.height) {
						cons_printf("\n");
						if (show_lines && reflines) 
							code_lines_print(reflines, sk+i, 1);
						if (show_reladdr)
							cons_printf("        ");
						if (show_offset)
							print_addr(sk+i);
						cons_strcat("  .db  ");
						w = 0;
					}
				}
				break;
			}
			if (foo->type!=DATA_STRUCT)
				cons_newline();
			CHECK_LINES
			bytes+=idata;
			ud_idx+=idata;
			myinc = 0;
			continue;
		}
		__outofme:
		if (data_end(sk) == DATA_FOLD_O) {
			folder--;
			for(i=0;i<folder;i++) cons_strcat("  ");
			cons_strcat("  }\n");
				if (reflines && show_lines)
					code_lines_print(reflines, sk+i, 1);
				if (show_reladdr)
					cons_printf("        ");
				if (show_offset)
					print_addr(sk+i);
			CHECK_LINES
		}
		__outofme2:

		switch(arch) {
		case ARCH_X86:
			memcpy(b, block+bytes, 32);
			break;
		case ARCH_CSR:
			memcpy(b, block+bytes, 2);
			break;
		case ARCH_ARM16:
			memcpy(b, block+bytes, 2);
			break;
		case ARCH_ARM:
			memcpy(b, block+bytes, 4);
			break;
		case ARCH_PPC:
			endian_memcpy_e(b, block+bytes, 4, !endian);
			break;
		default:
			//memcpy(b, config.block+bytes, 4);
			endian_memcpy_e(b, block+bytes, 4, endian);
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
#if 1
		switch(arch) {
			case ARCH_X86:
				if (ud_disassemble(&ud_obj)<1)
					return;
				//arch_x86_aop((unsigned long)ud_insn_off(&ud_obj), (const unsigned char *)config.block+bytes, &aop);
				//ud_set_pc(&ud_obj, seek);
				//arch_x86_aop((unsigned long)ud_insn_off(&ud_obj), (const unsigned char *)b, &aop);
				arch_x86_aop((u64)seek, (const u8*)block+bytes, &aop);
				myinc += ud_insn_len(&ud_obj);
				break;
			case ARCH_ARM16:
				arm_mode = 16;
				myinc += 2;
				arch_arm_aop(seek, (const u8 *)block+bytes, &aop);
				break;
			case ARCH_ARM:
				// endian stuff here
				myinc += arch_arm_aop(seek, (const u8 *)b+bytes, &aop);
				break;
			case ARCH_MIPS:
				arch_mips_aop(seek, (const u8 *)block+bytes, &aop);
				myinc += aop.length;
				break;
			case ARCH_SPARC:
				arch_sparc_aop(seek, (const u8 *)b+bytes, &aop);
				myinc += aop.length;
				break;
			case ARCH_JAVA:
				arch_java_aop(seek, (const u8 *)block+bytes, &aop);
				myinc += aop.length;
				break;
			case ARCH_PPC:
				arch_ppc_aop(seek, (const u8 *)block+bytes, &aop);
				myinc += aop.length;
				break;
			case ARCH_CSR:
				arch_csr_aop(seek, (const u8 *)block+bytes, &aop);
				myinc += 2;
				break;
			case ARCH_MSIL:
				arch_msil_aop(seek, (const u8 *)block+bytes, &aop);
				myinc += aop.length+1;
				break;
			case ARCH_BF:
				arch_bf_aop(seek, (const u8 *)block+bytes, &aop);
				myinc += aop.length;
				break;
			case ARCH_OBJD:
				radare_dump_and_process(DUMP_DISASM, len);
				return 0;
			default:
				// Uh?
				myinc += 4;
				// XXX clear aop or so
				break;
		}
#endif
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
			if (show_flags && !show_flagsline) {
				char buf[1024];
				f = flag_by_offset( seek );
				
				const char *flag = nullstr;
				if (f != NULL)
					flag = f->name;
				//cons_printf("(%08x) ", seek-config.baddr);
				if (flag == NULL || !flag[0])
					flag = flag_name_by_offset(seek -config.baddr);
				//if (flag == NULL)
				//	flag = flag_name_by_offset(seek +config.baddr);
				//config.baddr?(config.seek+bytes-myinc-myinc):seek);
				if (flag && flag[0]) {
					sprintf(buf, " %%%ds:", show_nbytes);
					if (strlen(flag)>show_nbytes) {
						cons_printf(buf, flag);
						NEWLINE;
						CHECK_LINES
						if (show_lines && reflines)
							code_lines_print(reflines, sk, 0);
						if (show_offset) {
							C cons_printf(C_GREEN"0x%08llX  "C_RESET, (unsigned long long)(seek));
							else cons_printf("0x%08llX  ", (unsigned long long)(seek));
						}
						if (show_reladdr)
							cons_strcat("        ");
						if (funline[0] != '\0')
							cons_strcat("|");
							//cons_strcat(funline);
						sprintf(buf, " %%%ds ", show_nbytes+((funline[0]!='\0')?0:1)); //show_nbytes);
						cons_printf(buf,"");
					} else {
						cons_printf(buf, flag);
					}
				} else {
					sprintf(buf, " %%%ds ", show_nbytes);
					cons_printf(buf, flag);
				}
			}

			/* cursor and bytes */
			D switch(is_cursor(bytes,myinc)) {
			case 0:
 				cons_strcat(" ");
				break;
			case 1:
 				cons_strcat("*");
				break;
			case 2:
 				cons_strcat(">");
				break;
			}
			if (show_bytes) {
				int max = show_nbytes;
				int cur = myinc;
				if (cur + 1> show_nbytes)
					cur = show_nbytes - 1;

				for(i=0;i<cur; i++)
					print_color_byte_i(bytes+i, "%02x", block[bytes+i]);
				if (cur !=myinc)
					max--;
				for(i=(max-cur)*2;i>0;i--)
					cons_printf(" ");
				if (cur != myinc)
					cons_printf(". ");
			}
			C switch(aop.type) {
			case AOP_TYPE_CMP: // cons_strcat("\x1b[36m");
				cons_strcat(cons_palette[PAL_CMP]);
				break;
			case AOP_TYPE_REP:
			case AOP_TYPE_JMP:
			case AOP_TYPE_CJMP:
				cons_strcat(cons_palette[PAL_JUMP]);
				break;
			case AOP_TYPE_CALL: // green cons_strcat("\x1b[32m");
				cons_strcat(cons_palette[PAL_CALL]);
				break;
			case AOP_TYPE_NOP: // cons_strcat("\x1b[35m");
				cons_strcat(cons_palette[PAL_NOP]);
				break;
			case AOP_TYPE_UPUSH:
			case AOP_TYPE_PUSH:
			case AOP_TYPE_POP: // yellow cons_strcat("\x1b[33m");
				cons_strcat(cons_palette[PAL_PUSH]);
				break;
			case AOP_TYPE_SWI:
			//case AOP_TYPE_UNK:
			case AOP_TYPE_TRAP:
			case AOP_TYPE_UJMP: //red cons_strcat("\x1b[31m");
				cons_strcat(cons_palette[PAL_TRAP]);
				break;
			case AOP_TYPE_RET: // blue cons_strcat("\x1b[34m");
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
			udis_arch_opcode(arch, endian, seek, bytes, myinc); //seek+myinc, bytes, myinc);

			/* show references */
			D if (aop.ref) {
				if (string_flag_offset(buf, aop.ref-config.baddr));
					cons_printf(" ; %s",buf);
			}

			/* show comments and jump keys */
			D if (show_comments) {
				if (aop.jump) {
					if (++jump_n<10) {
						jumps[jump_n-1] = aop.jump;
						if (string_flag_offset(buf, aop.jump) || (config.baddr && string_flag_offset(buf, aop.jump-config.baddr)))
							cons_printf("  ; %d = %s", jump_n,buf);
						else cons_printf("  ; %d = 0x%08llx", jump_n, aop.jump);
					} else {
						if (string_flag_offset(buf, aop.jump) || (config.baddr && string_flag_offset(buf, aop.jump-config.baddr)))
							cons_printf("  ; %s", buf);
						else cons_printf("  ; 0x%08llx", aop.jump);
					}
				}
			}

			/* show splits at end of code blocks */
			D if (show_splits) {
				char buf[1024];
				if (aop.jump||aop.eob) {
					if (config_get("asm.splitall") || aop.type == AOP_TYPE_RET) {
						NEWLINE;
						if (show_lines && reflines)
							code_lines_print(reflines, sk, 0);
						if (show_offset) {
							C cons_printf(C_GREEN"0x%08llX "C_RESET, (unsigned long long)(seek));
							else cons_printf("0x%08llX ", (unsigned long long)(seek));
						}
						sprintf(buf, "%%%ds ", show_nbytes);
						cons_printf(buf, "");
						cons_strcat("; ------------------------------------ ");
						CHECK_LINES
					}
				}
			}
		} else {
			udis_arch_opcode(arch, endian, sk, bytes, myinc);
		}

		cons_newline();
		if (f && f->cmd != NULL)
			radare_cmd(f->cmd, 0);

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
		cons_strcat("nop");
		break;
	case AOP_TYPE_RET:
		cons_strcat("ret");
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
	int (*fun)(u64 addr, const u8 *bytes, struct aop_t *aop);
} radis_arches [] = {
	{ "bf"      , ARCH_BF    , &arch_bf_aop  }   ,
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
	{ "msil"    , ARCH_MSIL  , &arch_msil_aop }   , 
	{ "objdump" , ARCH_OBJD  , NULL }   , 
	{ NULL      , -1         , NULL }
};

void radis_update_i(int id)
{
	int i;
	for(i=0;radis_arches[i].name;i++) {
		if (radis_arches[i].id == id) {
			config.arch = radis_arches[i].id;
			if (radis_arches[i].fun != NULL)
				arch_aop = radis_arches[i].fun;
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
			if (radis_arches[i].fun != NULL)
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
