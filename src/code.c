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
#include "arch/x86/ollyasm/disasm.h"
#include "list.h"

//#define CHECK_LINES if ( config.visual && len != config.block_size && (cons_lines > config.height) ) break;
#define CHECK_LINES if ( config.visual && (cons_lines > config.height) ) break;

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

static char *udis_mem = NULL;
static int udis_mem_ptr = 0;
static int input_hook_x(ud_t* u)
{
	if (ud_idx>length)
		return -1;
	if (ud_idx>=config.block_size)
		return -1;
	//return udis_mem[udis_mem_ptr++]; //config.block[ud_idx++];
	return config.block[ud_idx++];
}

void udis_init()
{
	const char *syn;
	const char *ptr;
	// TODO: check if already init

	ptr = config_get("asm.arch");
	syn = config_get("asm.syntax");

	//udis_mem_ptr = 0;
	udis_mem = config.block;
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
//ud_idx=0;
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

int udis_arch_string(int arch, char *string, const u8 *buf, int endian, u64 seek, int bytes, int myinc)
{
	//unsigned char *b = config.block + bytes;
	const u8 *b = buf; //config.block + bytes;
	int ret = 0;

	//ud_idx = bytes;
udis_mem= buf;
udis_mem_ptr= 0;

	if (bytes>63)
		bytes=63;
//	radare_read_at(seek, b, bytes);

	//b = config.block + bytes;
	string[0]='\0';
	switch(arch) {
	case ARCH_ARM:
		arm_mode = 32;
		force_thumb = 0;
		ret = gnu_disarm_str(string, b, seek);
		break;
	case ARCH_ARM16:
		arm_mode = 16;
		force_thumb = 1;
		ret = gnu_disarm_str(string, b, seek);
		break;
	case ARCH_CSR:
		//if (bytes<config.block_size) {
		arch_csr_disasm(string, (const unsigned char *)b, (u64)seek);
		break;
	case ARCH_X86:
		if (ollyasm_enable) {
			t_disasm da;
			ret = Disasm(b, seek, seek, &da, DISASM_CODE);
			sprintf(string, "%s", da.result);
		} else {
//			udis_init();
			ud_obj.insn_offset = seek;//;+myinc; //+bytes;
			ud_obj.pc = seek; //+myinc;
			//ud_idx = myinc;
			ud_disassemble(&ud_obj);
			ret = ud_insn_len(&ud_obj);
			//ud_idx+=ret;
			sprintf(string, "%s", ud_insn_asm(&ud_obj));
		}
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
		char bof[1024];
		endian_memcpy(bof, b, 4);
		dp.instr = bof;
	      // dp.instr = b; //config.block + i;
	       PPC_Disassemble(&dp, endian);
		sprintf(string, "%s %s", opcode, operands);
	       //cons_printf("  %s %s", opcode, operands);
		}
	       } break;
	case ARCH_JAVA:
	//	endian=1;
		if (java_disasm(b, string)==-1)
			strcpy(string, "???");
		break;
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
		sprintf(string, "%s %s", opcode, operands);
		} break;
	case ARCH_MSIL: {
		int n;
		DISASMSIL_OFFSET CodeBase = seek;
		ILOPCODE_STRUCT ilopar[8]; // XXX only uses 1
		DisasMSIL(b,bytes,CodeBase,ilopar, 8, &n);
		sprintf(string, "%s", ilopar[0].Mnemonic);
		//cons_printf("%s", ilopar[0].Mnemonic);
		ret = ilopar[0].Size;
		} break;
	}
	return ret;
}

int udis_arch_opcode(int arch, const u8 *b, int endian, u64 seek, int bytes, int myinc)
{
	char buf[128];
	//unsigned char *b = config.block + bytes; //(seek-config.seek); //+ bytes;
	int  ret = 0;
	ud_idx = bytes; //+myinc;

	buf[0]='\0';
	if (config_get_i("asm.pseudo")) {
		int tmp = ud_idx;
		pas_aop(arch, seek, b, bytes, NULL, buf);
		ud_idx = tmp;
	} else
	switch(arch) {
	case ARCH_X86:
	case ARCH_PPC:
	case ARCH_JAVA:
	case ARCH_M68K:
	case ARCH_CSR:
	case ARCH_MSIL:
		ret = udis_arch_string(arch, buf, b, endian, seek+myinc, bytes, myinc);
		break;
	case ARCH_ARM16:
	case ARCH_ARM:
		endian_memcpy(&buf, b, 4); //, endian);
		ret = gnu_disarm((u8*)buf, (u64)seek+myinc);
		break;
	case ARCH_MIPS:
		//unsigned long ins = (b[0]<<24)+(b[1]<<16)+(b[2]<<8)+(b[3]);
		gnu_dismips((u8*)b, (unsigned int)seek+myinc);
		break;
	case ARCH_SPARC:
		gnu_disparc((u8*)b, (unsigned int)seek+myinc);
		break;
	case ARCH_BF:
		ret = arch_bf_dis(b, seek+myinc, 1024);
		break;
	default:
		strcpy(buf, "Unknown architecture");
		break;
	}
	if (buf[0]!='\0')
		cons_strcat(buf);

	C cons_printf(C_RESET);
	return ret;
}

extern int color;
void udis_arch(int arch, int len, int rows)
{
	int foo = ud_idx;
	radis_str_e(arch, config.block, len,rows);
	//udis_arch_buf(arch, config.block, len, rows);
	ud_idx = foo;
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
	//{ "aop"     , ARCH_AOP   , &arch_aop_aop }   , 
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

/* ------------------------------  refactor ----------------------------------*/

#define RADIS_SIZE  0x00001
#define RADIS_BYTES 0x00002
#define RADIS_OFFSET 0x00004
#define RADIS_FUNCTIONS 0x00008
#define RADIS_SECTION 0x00010
#define RADIS_SPLITS 0x00040
#define RADIS_FLAGS 0x00080
#define RADIS_FLAGSLINE 0x00100
#define RADIS_LINES 0x00200
#define RADIS_RELADDR 0x00400
#define RADIS_TRACES 0x00800
#define RADIS_COMMENTS 0x01000
#define RADIS_COLOR 0x02000
#define RADIS_STACKPTR 0x04000


static int stack_ptr = 0;
static void print_stackptr(struct aop_t *aop, int zero)
{
	int ostack_ptr = stack_ptr;
	char changed = ' ';
	if (zero)
		stack_ptr = 0;

	if (aop->stackop == AOP_STACK_INCSTACK) {
		stack_ptr += aop->value;
	}
	switch(aop->type) {
	case AOP_TYPE_UPUSH:
	case AOP_TYPE_PUSH:
		stack_ptr += 8;
		break;
	case AOP_TYPE_POP:
		stack_ptr -= 8;
		break;
	}
	if (stack_ptr != ostack_ptr)
		changed = '_';
	cons_printf("%3d%c", stack_ptr, changed);
}

void radis_str(int arch, const u8 *block, int len, int rows,char *cmd_asm, int flags)
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
	flag_t *f = NULL;
	int rrows = rows; /* rrows is in reality the num of bytes to be disassembled */
	int endian;
	int show_nbytes;
	int folder = 0; // folder level

	// XXX
	endian        = (int) config_get("cfg.bigendian");
	show_nbytes   = (int) config_get_i("asm.nbytes");
	if (show_nbytes>16 ||show_nbytes<0)
		show_nbytes = 16;

	jump_n = 0;
	length = len;
	ud_idx = 0;
	delta = 0;
	inc = 0;
	stack_ptr = 0;

	if (config.visual && rows<1)
		rows = config.height - ((last_print_format==FMT_VISUAL)?11:0) - config.lines;

#if 1
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
#endif

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
			if (flags & RADIS_COMMENTS)
				data_printd(bytes);
			if ((reflines) &&  (flags & RADIS_LINES))
				code_lines_print(reflines, sk, 0);
		}
		if (flags & RADIS_SECTION) {
			if (config.baddr)
				cons_printf("%s:", flag_get_here_filter(seek - config.baddr, "section_"));
			else cons_printf("%s:", flag_get_here_filter(seek, "section_"));
		}
		if (flags & RADIS_OFFSET)
			print_addr(seek);
		if (flags & RADIS_RELADDR) {
			if (bytes==0) cons_printf("%08llX ", seek);
			else cons_printf("+%7d ", bytes);
		}
		/* size */
		if (flags & RADIS_STACKPTR)
			print_stackptr(&aop, 0);
		if (flags & RADIS_SIZE)
			cons_printf("%d ", aop.length);
		/* trac information */
		if (flags & RADIS_TRACES)
			cons_printf("%04x %02x ", trace_count(seek), trace_times(seek));

		struct data_t *foo = data_get_range(sk);
		funline[0]='\0';
		/* handle data type block */
		//	print_function_line(foo, sk);
		if (flags & RADIS_FUNCTIONS ) {
			if (foo != NULL && foo->type == DATA_FUN) {
				stack_ptr = 0;
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
				foo = NULL;
			} else
				cons_strcat(" ");
		}

		if (foo != NULL) {
			//int dt = foo->type;
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
						if (reflines && flags & RADIS_LINES)
							code_lines_print(reflines, sk+i, 1);
						if (flags & RADIS_RELADDR)
							cons_printf("        ");
						if (flags & RADIS_SECTION ) {
							if (config.baddr)
								cons_printf("%s:", flag_get_here_filter(seek - config.baddr, "section_"));
							else cons_printf("%s:", flag_get_here_filter(seek, "section_"));
						}
						if (flags & RADIS_OFFSET)
							print_addr(sk+i);
						if (flags & RADIS_STACKPTR)
							print_stackptr(&aop, 0);
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
						if (reflines && flags & RADIS_LINES)
							code_lines_print(reflines, sk+i, 1);
						if (flags & RADIS_RELADDR)
							cons_printf("        ");
						if (flags & RADIS_SECTION) {
							if (config.baddr)
								cons_printf("%s:", flag_get_here_filter(seek - config.baddr, "section_"));
							else cons_printf("%s:", flag_get_here_filter(seek, "section_"));
						}
						if (flags & RADIS_OFFSET)
							print_addr(sk+i);
						if (flags & RADIS_STACKPTR)
							print_stackptr(&aop, 0);
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
				if (reflines && flags & RADIS_LINES)
					code_lines_print(reflines, sk+i, 1);
				if (flags & RADIS_RELADDR)
					cons_printf("        ");
				if (flags & RADIS_SECTION) {
					if (config.baddr)
						cons_printf("%s:", flag_get_here_filter(seek - config.baddr, "section_"));
					else cons_printf("%s:", flag_get_here_filter(seek, "section_"));
				}
				if (flags & RADIS_OFFSET)
					print_addr(sk+i);
				if (flags & RADIS_STACKPTR)
					print_stackptr(&aop, 0);
			CHECK_LINES
		}

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
			endian_memcpy_e((u8*)b, (u8*)block+bytes, 4, !endian);
			break;
		default:
			//memcpy(b, config.block+bytes, 4);
			endian_memcpy_e((u8*)b, (u8*)block+bytes, 4, endian);
		}

		if (cmd_asm && cmd_asm[0]) {
			char buf[128];
			sprintf(buf, "%lld", seek);
			setenv("HERE", buf, 1);
			radare_cmd(cmd_asm, 0);
		}
		
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
				myinc += arch_arm_aop(seek, (const u8 *)block+bytes, &aop);
				break;
			case ARCH_MIPS:
				arch_mips_aop(seek, (const u8 *)block+bytes, &aop);
				myinc += aop.length;
				break;
			case ARCH_SPARC:
				arch_sparc_aop(seek, (const u8 *)block+bytes, &aop);
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
				return;
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
			if (flags & RADIS_FLAGS && !(flags&RADIS_FLAGSLINE)) {
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
						if (reflines && flags & RADIS_LINES)
							code_lines_print(reflines, sk, 0);
						if (flags & RADIS_SECTION){
							if (config.baddr)
								cons_printf("%s:", flag_get_here_filter(seek - config.baddr, "section_"));
							else cons_printf("%s:", flag_get_here_filter(seek, "section_"));
						}
						if (flags & RADIS_OFFSET)
							print_addr(seek);
						if (flags & RADIS_STACKPTR)
							print_stackptr(&aop, 0);
						if (flags & RADIS_RELADDR)
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
			if (flags & RADIS_BYTES) {
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
			/* XXX: The seek+myinc is weird but MANDATORY, the first opcode must
			 * be used to get the 'base incremental' address for calculating the
			 * rest of opcodes, or the jumps will be addr-myinc and only the *first*
			 * opcode will be disassmbled correctly.
			 *
			 * I know that this has no sense, but computer science is not an exact one.
			 */
			udis_arch_opcode(arch, block+bytes, endian, seek, bytes, myinc); //seek+myinc, bytes, myinc);

			/* show references */
			D if (aop.ref) {
				if (string_flag_offset(buf, aop.ref-config.baddr));
					cons_printf(" ; %s",buf);
			}

			/* show comments and jump keys */
			D if (flags & RADIS_COMMENTS) {
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
			D if (flags & RADIS_SPLITS) {
				char buf[1024];
				if (aop.jump||aop.eob) {
					if (config_get("asm.splitall") || aop.type == AOP_TYPE_RET) {
						NEWLINE;
						if (reflines && flags & RADIS_LINES)
							code_lines_print(reflines, sk, 0);
						if (flags & RADIS_SECTION) {
							if (config.baddr)
								cons_printf("%s:", flag_get_here_filter(seek - config.baddr, "section_"));
							else cons_printf("%s:", flag_get_here_filter(seek, "section_"));
						}
						if (flags & RADIS_OFFSET)
							print_addr(seek);
						if (flags & RADIS_STACKPTR)
							print_stackptr(&aop, 0);
						sprintf(buf, "%%%ds ", show_nbytes);
						cons_printf(buf, "");
						cons_strcat("; ------------------------------------ ");
						CHECK_LINES
					}
				}
			}
		} else {
			udis_arch_opcode(arch,block+bytes, endian, sk, bytes, myinc);
		}

		cons_newline();
		if (f && f->cmd != NULL)
			radare_cmd(f->cmd, 0);

		bytes+=myinc;
		myinc = 0;
	}

	radare_controlc_end();
}

void radis_str_e(int arch, const u8 *block, int len, int rows)
{
	const char *cmd_asm;
	int size, bytes, offset, splits, comments, lines, section,
	traces, flags, reladdr, flagsline, functions, stackptr;

	// cache here!
	cmd_asm       = config_get("cmd.asm");
	size     = (int) config_get("asm.size");
	bytes    = (int) config_get("asm.bytes");
	offset   = (int) config_get("asm.offset");
	functions= (int) config_get("asm.functions");
	section  = (int) config_get("asm.section");
	splits   = (int) config_get("asm.split");
	flags    = (int) config_get("asm.flags");
	flagsline= (int) config_get("asm.flagsline");
	lines    = (int) config_get_i("asm.lines");
	reladdr  = (int) config_get("asm.reladdr");
	stackptr = (int) config_get("asm.stackptr");
	traces   = (int) config_get("asm.trace");
	comments = (int) config_get("asm.comments");
	color    = (int) config_get("scr.color");

	flags = 0;
	if (color) flags |= RADIS_COLOR;
	if (size) flags |= RADIS_SIZE;
	if (bytes) flags |= RADIS_BYTES;
	if (offset) flags |= RADIS_OFFSET;
	if (functions) flags |= RADIS_FUNCTIONS;
	if (section) flags |= RADIS_SECTION;
	if (splits) flags |= RADIS_SPLITS;
	if (flags) flags |= RADIS_FLAGS;
	if (stackptr) flags |= RADIS_STACKPTR;
	if (flagsline) flags |= RADIS_FLAGSLINE;
	if (lines) flags |= RADIS_LINES;
	if (reladdr) flags |= RADIS_RELADDR;
	if (traces) flags |= RADIS_TRACES;
	if (comments) flags |= RADIS_COMMENTS;
	if (color) flags |= RADIS_COLOR;
	
	radis_str(arch, block, len, rows, cmd_asm, flags);
}

