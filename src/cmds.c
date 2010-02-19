/*
 * Copyright (C) 2006, 2007, 2008, 2009
 *       pancake <youterm.com>
 * Copyright (C) 2006 Lluis Vilanova <xscript@gmx.net>
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
#include "radare.h"
#include "rdb.h"
#include "rasm/rasm.h"
#if __UNIX__
#include <sys/ioctl.h>
#include <regex.h>
#include <termios.h>
#include <netdb.h>
#endif
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "search.h"
#include "utils.h"
#include "plugin.h"
#include "cmds.h"
#include "readline.h"
#include "print.h"
#include "flags.h"
#include "undo.h"
#include "rtr.h"

print_fmt_t last_print_format = FMT_HEXB;
//int fixed_width = 0;
extern char **environ;
extern command_t commands[];
extern void show_help_message();

/* Call a command from the command table */
int commands_parse (const char *_cmdline)
{
	// XXX making non-const a const ptr??
	char *cmdline = (char *)_cmdline;
	command_t *cmd = NULL;

	for(;cmdline[0]&&(cmdline[0]=='\x1b'||cmdline[0]==' ');cmdline=cmdline+1);

	// null string or comment
	if (cmdline[0]==0||cmdline[0]=='#'||cmdline[0]==';')
		return 0;

	for (cmd = commands; cmd->sname != 0; cmd++) {
		if (cmd->sname == cmdline[0])
		    return cmd->hook(cmdline+1);
	}
	return cmd?cmd->hook(cmdline):-1;
}

void show_help_message()
{
	command_t *cmd;
	char *cmdaux;
	char cmdstr[128];

	for (cmd = &commands[0]; cmd->sname != 0; cmd++) {
		cmdaux = cmdstr;
		cmdaux[0] = cmd->sname;
		switch(cmdaux[0]) {
		case '+': case '-': case '0': case '?':
		    continue;
		}
		cmdaux++;
		cmdaux += sprintf(cmdaux, "%s", cmd->options);
		cmdaux[0] = '\0';
		cons_printf(" %-17s %s\n", cmdstr, cmd->help);
	}
	cons_printf(" ?[?]<expr>        calc     evaluate math expression\n");
}

command_t commands[] = {
	COMMAND('a', "[ocdgv?]",       "analyze  perform code/data analysis", analyze),
	COMMAND('b', " [blocksize]",   "bsize    change the block size", blocksize),
	COMMAND('c', "[f file]|[hex]", "cmp      compare block with given data", compare),
	COMMAND('C', "[op] [arg]",     "Code     Comments, data type conversions, ..", code),
	COMMAND('d', "[rsc..]",        "debug    Debug command (backport from r2)", dbg),
	COMMAND('e', "[m?] key=value", "eval     evaluates a configuration expression", config_eval),
	COMMAND('f', "[crogd|-][name]","flag     flag the current offset (f? for help)", flag),
	COMMAND('H', " [cmd]",         "Hack     performs a hack", hack),
	COMMAND('i', "",               "info     prints status information", info),
	COMMAND('o', " [file]",        "open     open file", open),
	COMMAND('p', "[fmt] [len]",    "print    print data block", print),
	COMMAND('q', "[!]",            "quit     close radare shell", quit),
	COMMAND('P', "[so][i [file]]", "Project  project Open, Save, Info", project),
	COMMAND('r', " [size|-strip]", "resize   resize or query the file size", resize),
	COMMAND('g', "[act] ([arg])",  "graph    graph analysis operations", graph),
	COMMAND('s', " [[+,-]pos]",    "seek     seek to absolute/relative expression", seek),
	COMMAND('S', "[len] [vaddr]",  "Section  manage io.vaddr sections", sections),
	COMMAND('u', "[[+,-]idx]",     "undo     undo/redo indexed write change", undowrite),
	COMMAND('V', "",               "Visual   enter visual mode", visual),
	COMMAND('w', "[?aAdwxfF] [str]","write    write ascii/hexpair string here", write),
	COMMAND('x', " [length]",      "examine  the same as px", examine),
	COMMAND('y', "[yt] [len] [to]","yank     copy/paste bytes ('yy' to paste)", yank),
	COMMAND('.', "[!cmd]|[ file]", ".script  interpret a script (radare, .py, .rb, .lua, .pl)", interpret),
	COMMAND('-', "[size]",         "prev     go to previous block (-= block_size)", prev),
	COMMAND('+', "[size]",         "next     go to next block (+= block_size)", next),
	COMMAND('(', "name\\n...\\n)", "macro    record macros (use .(foo) to call it)", macro),
	COMMAND('<', "",               "preva    go previous aligned block", prev_align),
	COMMAND('>', "",               "nexta    go next aligned block", next_align),
	COMMAND('/', "[?] [str]",      "search   find matching strings", search),
	COMMAND('!', "[[!]command]",   "system   execute a !iosystem or !!shell command", shell), 
	COMMAND('h', "[hash|!lang]",   "hash     hash current block (#? or #!perl)", hash),
	COMMAND('=', "[?] [cmd]",      "remote   send cmd to remote radare", rtr),
	COMMAND('?', "",               "help     show the help message", help),
	COMMAND( 0, NULL, NULL, default)
};

CMD_DECL(macro)
{
	switch (input[0]) {
	case ')':
		radare_macro_break(input+1);
		break;
	case '-':
		radare_macro_rm(input+1);
		break;
	case '\0':
		radare_macro_list();
		break;
	case '?':
		eprintf(
		"Usage: (foo\\n..cmds..\\n)\n"
		" Record macros grouping commands\n"
		" (foo args\\n ..)  ; define a macro\n"
		" (-foo)            ; remove a macro\n"
		" .(foo)            ; to call it\n"
		" ()                ; break inside macro\n"
		"Argument support:\n"
		" (foo x y\\n$1 @ $2) ; define fun with args\n"
		" .(foo 128 0x804800) ; call it with args\n"
		"Iterations:\n"
		" .(foo\\n() $@)      ; define iterator returning iter index\n"
		" x @@ .(foo)         ; iterate over them\n"
		);
		break;
	default:
		radare_macro_add(input);
		break;
	}
	return 0;
}

CMD_DECL(config_eval)
{
	int i = 0;
	for(i=0;input[i]&&input[i]!=' ';i++);
	switch(input[0]) {
	case 'r':
		config_init(0);
		break;
	case 'm':
		CMD_CALL(menu, input);
		break;
	case '?':
		cons_printf( "Usage: e[m] key=value\n"
		" ereset           ; reset configuration\n"
		" emenu            ; opens menu for eval\n"
		" e scr.color = 1  ; sets color for terminal\n");
		break;
	default:
		config_eval(input+i);
	}
	return 0;
}

CMD_DECL(analyze)
{
	struct aop_t aop;
	struct program_t *prg;
	struct block_t *b0;
	struct list_head *head;
	struct list_head *head2;
	struct xrefs_t *c0;
	char *file;
	const char *ptr;
	int i, sz, n_calls=0;
	int depth_i;
	int delta = 0;
	int depth = input[0]?atoi(input+1):0;
	ut64 oseek = config.seek;

	if (input[0]=='c'&&depth<1) // only for 'ac'
		depth = config_get_i("graph.depth");
	if (depth<1) depth=1;
	//eprintf("depth = %d\n", depth);
	memset(&aop, '\0', sizeof(struct aop_t));

	switch(input[0]) {
	case 't':
		switch(input[1]) {
		case '?':
			cons_strcat("Usage: at[*] [addr]\n"
			" at?                ; show help message\n"
			" at                 ; list all traced opcode ranges\n"
			" at-                ; reset the tracing information\n"
			" at*                ; list all traced opcode offsets\n"
			" at+ [addr] [times] ; add trace for address N times\n"
			" at [addr]          ; show trace info at address\n"
			" att [tag]          ; select trace tag (no arg unsets)\n"
			" at%                ; TODO\n"
			" atr                ; show traces as range commands (ar+)\n"
			" atd                ; show disassembly trace\n"
			" atD                ; show dwarf trace (at*|rsc dwarf-traces $FILE)\n");
			eprintf("Current Tag: %d\n", trace_tag_get());
			break;
		case 't':
			sz = atoi(input+2);
			if (sz>0) trace_tag_set(sz-1);
			else trace_tag_set(-1);
			break;
		case 'd':
			trace_show(2, trace_tag_get());
			break;
		case 'D':
			radare_cmd("at*|rsc dwarf-traces $FILE", 0);
			break;
		case '+':
			ptr = input+3;
			ut64 addr = get_offset(input+3);
			ptr = strchr(ptr, ' ');
			if (ptr != NULL) {
				//eprintf("at(0x%08llx)=%d (%s)\n", addr, atoi(ptr+1), ptr+1);
				trace_add(addr);
				trace_set_times(addr, atoi(ptr+1));
			}
			break;
		case '-':
			trace_reset();
			break;
		case ' ': {
			struct trace_t *t = (struct trace_t *)trace_get(get_math (input+1), trace_tag_get());
			if (t != NULL) {
				cons_printf("offset = 0x%llx\n", t->addr);
				cons_printf("opsize = %d\n", t->opsize);
				cons_printf("times = %d\n", t->times);
				cons_printf("count = %d\n", t->count);
				//TODO cons_printf("time = %d\n", t->tm);
			} }
			break;
		case '*':
			trace_show(1, trace_tag_get());
			break;
		case 'r':
			trace_show(-1, trace_tag_get());
			break;
		default:
			trace_show(0, trace_tag_get());
		}
		break;
	case 'd':
		// XXX do not uses depth...ignore analdepth?
		radare_analyze(config.seek, config.block_size, config_get_i("anal.depth"), input[1]=='*');
		break;
#if 0
	/* TODO: reset analyze engine ? - remove rdbs, xrefs, etc...  reset level as argument? maybe cool 0 for just vm, 1 for rdbs, 2 for xrefs */
	case 'r':
		break;
#endif
	case 'b':
		prg = code_analyze(config.seek, depth+1);
		if (prg) {
			//cons_printf("name = %s\n", prg->name);
			list_for_each_prev(head, &(prg->blocks)) {
				struct block_t *b0 = list_entry(head, struct block_t, list);
				cons_printf("offset = 0x%08llx\n", b0->addr);
				cons_printf("type = %s\n", block_type_names[b0->type]);
				cons_printf("size = %d\n", b0->n_bytes);
				list_for_each(head2, &(b0->calls)) {
					c0 = list_entry(head2, struct xrefs_t, list);
					cons_printf("call%d = 0x%08llx\n", n_calls++, c0->addr);
				}
				cons_printf("n_calls = %d\n", b0->n_calls);

				if (b0->tnext)
					cons_printf("true = 0x%08llx\n", b0->tnext);
				if (b0->fnext)
					cons_printf("false = 0x%08llx\n", b0->fnext);
				cons_printf("bytes = ");
				for (i=0;i<b0->n_bytes;i++)
					cons_printf("%02x", b0->bytes[i]);
				cons_newline();
			}
		} else eprintf("oops\n");
		break;
	case 'f':
		switch(input[1]) {
		case '?':
			eprintf("Usage: af[*] @ addr\n");
			eprintf(" af         - show function report (fun metrics)\n");
			eprintf(" afr @ addr - analyze function recursively at address\n");
			eprintf(" afu @ addr - undefine function at address\n");
			eprintf(" .af*       - import function analysis (same as Vdf)\n");
			break;
		case 'r':
			analyze_function(config.seek, config_get_i("anal.depth"), 0);
			break;
		case 'u':
			analyze_function(config.seek, config_get_i("anal.depth"), 2);
			break;
		case '*':
			analyze_function(config.seek, config_get_i("anal.depth"), 0);
			break;
		default:
			analyze_function(config.seek, 0, 1);
		}
		break;
	case 'g':
		depth = config_get_i("graph.depth");
		switch(input[1]) {
		case '?':
			eprintf("Usage: ag[.]\n");
			eprintf(" ag          - open graph window\n");
			eprintf(" agd [file]  - outputs dot graph format for code analysis\n");
			eprintf(" agd*        - like above but only addresses (without body)\n");
			eprintf(" agdv [file] - use agd > file.dot && !!dot -Tpng -ofile.png file.dot && rsc view file.png\n");
			break;
		case 'd':
			file = strchr(input+2, ' ');
			if (input[2]=='v') {
				char *oprof = strdup(config_get("asm.profile"));
				int ocolor = config_get_i("scr.color");
				config_set("asm.profile", "simple");
				config_set_i("scr.color", 0);
				radare_cmd_raw("agd > .file.dot", 0);
				config_set("asm.profile", oprof);
				config_set_i("scr.color", ocolor);
				if (oprof) free(oprof);
				if (file && file[1]) {
					char buf[1024];
					radare_cmd("agd .file.dot", 0);
					radare_cmd_raw("!!dot -Tpng -o .file.png .file.dot", 0);
				} else {
					radare_cmd_raw("!!dot -Tpng -o .file.png .file.dot", 0);
					radare_cmd_raw("!!rsc view .file.png", 0);
				}
				radare_cmd_raw("!!rm -f .file.png .file.dot", 0);
			} else {
				if (file && file[1]) {
					int ofd, fd = open(file+1, O_RDWR|O_CREAT, 0644);
					if (fd!=-1) {
						ofd = dup(1);
						dup2(fd, 1);
						radare_cmd("agd", 0);
						stdout_close();
						dup2(ofd, 1);
						close(fd);
						close(ofd);
					} else {
						eprintf("Cannot open file '%s'\n", file+1);
					}
				} else {
					prg = code_analyze(config.seek, depth ); //config_get_i("graph.depth"));
					list_add_tail(&prg->list, &config.rdbs);
					graph_viz(prg, input[2]!='*');
				}
			}
			break;
		default:
#if HAVE_GUI
			// use graph.depth by default if not set
			config.graph = 1;
			prg = code_analyze(config.vaddr + config.seek, depth ); //config_get_i("graph.depth"));
			list_add_tail(&prg->list, &config.rdbs);
			grava_program_graph(prg, NULL);
			config.graph = 0;
#else
			eprintf("Compiled without gui. Try with ag*\n");
#endif
		}
		break;
	case 'C':
		{
			char *b, *str;
			ut64 seek = config.seek;
			ut32 len, i = 0;
			if (input[1]=='f'){
 				// TODO: use get_math(input+2) ???
				struct data_t *data = data_get(config.seek);
				if (data) len = data->size;
				else len = 0;
			} else len = (ut32)get_math(input+1);
			if (len<1) len = config.block_size;
			b = malloc(len);
			radare_read_at(seek, b, len);
			while (i<len) {
				if (arch_aop(seek+i, b+i, &aop)<1)
					break;
				if (aop.type == AOP_TYPE_CALL) {
					const char *str = flag_name_by_offset(aop.jump);
					cons_printf("call 0x%08llx", aop.jump);
					if (str&&*str)
						cons_printf(" ; %s\n", str);
					cons_printf("\n");
				}
				i += aop.length;
			}
			free(b);
		}
		break;
	case 'c': {
		int c = config.verbose;
		char cmd[1024];
		config.verbose = 1;
		prg = code_analyze(config.seek+config.vaddr, depth); //config_get_i("graph.depth"));
		list_add_tail(&prg->list, &config.rdbs);
		list_for_each(head, &(prg->blocks)) {
			b0 = list_entry(head, struct block_t, list);
			D {
				cons_printf("0x%08x (%d) -> ", b0->addr, b0->n_bytes);
				if (b0->tnext)
					cons_printf("0x%08llx", b0->tnext);
				if (b0->fnext)
					cons_printf(", 0x%08llx", b0->fnext);
				cons_printf("\n");
			// TODO eval asm.lines=0
				sprintf(cmd, "pD %d @ 0x%08x", b0->n_bytes, (unsigned int)b0->addr);
			// TODO restore eval
				radare_cmd(cmd, 0);
				cons_printf("\n\n");
			} else {
				cons_printf("b %d\n", b0->n_bytes);
				cons_printf("f blk_%08X @ 0x%08x\n", b0->addr, b0->addr);
			}
			i++;
		}
		config.verbose=c;
		} break;
	case 'm':
		input = input+1;
		while(input[0]==' ') input=input+1;
		if (strchr(input,'-')) {
			print_mem_del(input+1);
		} else
		if (strchr(input,'?')) {
			cons_printf("Usage: am [-][name] [format]\n"
			" am          ; list all memory formats registered\n"
			" am foo xxd  ; add a named memory format\n"
			" am -foo     ; delete named memory foo\n"
			" pm x[foo]x  ; use the 'foo' named memory format from 'pm'\n"
			" pm x[*foo]x ; use the 'foo' (follow pointer) named memory format from 'pm'\n");
		} else
		if (input[0]=='\0') {
			print_mem_list();
		} else {
			char *s = strchr(input,' ');
			if (s) {
				*s='\0';
				print_mem_add(input, s+1);
			} else {
				const char *fmt = print_mem_get(input);
				if (fmt) print_mem(config.seek, config.block,
					config.block_size, fmt, config.endian);
			}
		}
		break;
	case 'o':
		if (input[1]=='s') {
			int i, count = get_math(input+3);
			sz = 0;
			if (count<1) count = 1;
			radare_read(0);
			for(i=0;i<count;i++) {
				sz += arch_aop(config.vaddr + config.seek+i,
					config.block-i, &aop);
			}
			cons_printf("%d\n", sz);
			break;
		}

		for(depth_i=0;depth_i<depth;depth_i++) {
			char food[64];
			food[0] = '\0';
			radare_read(0);
			udis_init();
			pas_aop(config.arch, config.seek, config.block, 16, NULL, food, 0);
			sz = arch_aop(config.vaddr + config.seek, config.block, &aop);

			cons_printf("opcode = %s\n", food);
			cons_printf("index = %d\n", depth_i);
#if 0
			cons_printf("opcode = ");
			j = config.verbose;
			config.verbose = 0;
			radare_cmd("pd 1", 0);
			config.verbose = j;
#endif
#if 1
			cons_printf("size = %d\n", sz);
			// TODO: implement aop-type-to-string
			cons_printf("stackop = ");
			if (aop.stackop == AOP_STACK_INCSTACK) {
				if (aop.value > 0)
					cons_printf("inc %d\n", aop.value);
				else cons_printf("dec %d\n", -aop.value);
			} else {
				switch(aop.type) {
				case AOP_TYPE_PUSH:  cons_printf("push 8\n"); break;
				case AOP_TYPE_UPUSH:  cons_printf("push 8\n"); break;
				case AOP_TYPE_POP:  cons_printf("pop 8\n"); break;
				default: 
					switch(aop.stackop) {
					case AOP_STACK_LOCAL_GET:
						cons_printf("local-var-get\n");
						break;
					case AOP_STACK_LOCAL_SET:
						cons_printf("local-var-set\n");
						break;
					case AOP_STACK_ARG_GET:
						cons_printf("function-arg-get\n");
						break;
					case AOP_STACK_ARG_SET:
						cons_printf("function-arg-set\n");
						break;
					default:
						cons_printf("unknown(%d)\n", aop.stackop);
					}
				}
			}
#endif
			cons_printf("type = ");
			switch(aop.type) {
			case AOP_TYPE_CALL:  cons_printf("call\n"); break;
			case AOP_TYPE_CJMP:  cons_printf("conditional-jump\n"); break;
			case AOP_TYPE_JMP:   cons_printf("jump\n"); break;
			case AOP_TYPE_UJMP:  cons_printf("jump-unknown\n"); break;
			case AOP_TYPE_TRAP:  cons_printf("trap\n"); break;
			case AOP_TYPE_SWI:   cons_printf("interrupt\n"); break;
			case AOP_TYPE_UPUSH: cons_printf("push-unknown\n"); break;
			case AOP_TYPE_PUSH:  cons_printf("push\n"); break;
			case AOP_TYPE_POP:   cons_printf("pop\n"); break;
			case AOP_TYPE_ADD:   cons_printf("add\n"); break;
			case AOP_TYPE_SUB:   cons_printf("sub\n"); break;
			case AOP_TYPE_AND:   cons_printf("and\n"); break;
			case AOP_TYPE_OR:    cons_printf("or\n"); break;
			case AOP_TYPE_XOR:   cons_printf("xor\n"); break;
			case AOP_TYPE_NOT:   cons_printf("not\n"); break;
			case AOP_TYPE_MUL:   cons_printf("mul\n"); break;
			case AOP_TYPE_DIV:   cons_printf("div\n"); break;
			case AOP_TYPE_SHL:   cons_printf("shift-left\n"); break;
			case AOP_TYPE_SHR:   cons_printf("shift-right\n"); break;
			case AOP_TYPE_REP:   cons_printf("rep\n"); break;
			case AOP_TYPE_RET:   cons_printf("ret\n"); break;
			case AOP_TYPE_ILL:   cons_printf("illegal-instruction\n"); break;
			case AOP_TYPE_MOV:   cons_printf("move\n"); break;
			case AOP_TYPE_LOAD:  cons_printf("load\n"); break;
			case AOP_TYPE_STORE: cons_printf("store\n"); break;
			default: cons_printf("unknown(%d)\n", aop.type);
			}
			cons_printf("bytes = ");
			for (i=0;i<sz;i++) cons_printf("%02x", config.block[i]);
			cons_printf("\n");
			cons_printf("offset = 0x%08llx\n", config.vaddr+config.seek);
			cons_printf("ref = 0x%08llx\n", aop.ref);
			cons_printf("jump = 0x%08llx\n", aop.jump);
			cons_printf("fail = 0x%08llx\n", aop.fail);
			cons_newline();
			delta += sz;
			config.seek += sz;
		}
		break;
	case 'p':
		analyze_preludes(input+1);
		break;
	case 'v':
		switch(input[1]) {
		case 'e':
			if (input[2]=='\0')
				cons_printf("Usage: \"ave [expression]\n"
				"Note: The prefix '\"' quotes the command and does not parses pipes and so\n");
			else vm_eval(input+2);
			break;
		case 'f':
			if (input[2]=='\0')
				cons_printf("Usage: avf [file]\n");
			else vm_eval_file(input+2);
			break;
		case 'r':
			if (input[2]=='?')
				cons_printf(
				"Usage: avr [reg|type]\n"
				" avr+ eax int32  ; add register\n"
				" avr- eax        ; remove register\n"
				" \"avra al al=eax&0xff al=al&0xff,eax=eax>16,eax=eax<16,eax=eax|al\n"
				"                 ; set register alias\n"
				" avr eax         ; view register\n"
				" avr eax=33      ; set register value\n"
				" avrt            ; list valid register types\n"
				"Note: The prefix '\"' quotes the command and does not parses pipes and so\n");
			else vm_cmd_reg(input+2);
			break;
		case 'I':
			vm_import(1);
			break;
		case 'i':
			vm_import(0);
			break;
		case '-':
			vm_init(1);
			break;
		case 'o':
			if (input[2]=='\0')
				vm_op_list();
			else if (input[2]=='?')
				vm_cmd_op_help();
			else vm_cmd_op(input+2);
			break;
		case '\0':
		case '?':
			cons_printf("Usage: av[ier] [arg]\n"
			" ave eax=33   ; evaluate expression in vm\n"
			" avf file     ; evaluate expressions from file\n"
			" avi          ; import register values from flags (eax, ..)\n"
			" avI          ; import register values from vm flags (vm.eax, ..)\n"
			" avm          ; select MMU (default current one)\n"
			" avo op expr  ; define new opcode (avo? for help)\n"
			" avr          ; show registers\n"
			" avx N        ; execute N instructions from cur seek\n"
			" av-          ; restart vm using asm.arch\n"
			" avr eax      ; show register eax\n"
			" avra         ; show register aliases\n"
			" avra al eax=0xff ; define 'get' alias for register 'al'\n"
			" avrt         ; list valid register types\n"
			" e vm.realio  ; if true enables real write changes\n"
			"Note: The prefix '\"' quotes the command and does not parses pipes and so\n");
			break;
		case 'm':
			eprintf("TODO\n");
			break;
		case 'x':
			vm_emulate(atoi(input+2));
			break;
		case '*':
			vm_print(-2);
			break;
		default:
			vm_print(0);
			break;
		}
		break;
	case 'r':
		ranges_cmd(input+1);
		break;
	case 's':
		analyze_spcc(input+1);
		break;
	case 'x': {
		char buf[4096];
		char file[1024];
		ut64 seek = config.seek;
		ut64 base = 0;
		strcpy(file, config.file);
#if DEBUGGER
		if (config.debug) {
			/* dump current section */
			if (radare_dump_section(file))
				return 1;
			base = 0x8048000; // XXX must be section base
		}
#endif
		{
		char buf[4096];
		sprintf(buf,"xrefs -a %s -b %lld %s %lld",
			config_get("asm.arch"), base, file, seek);
		radare_system(buf);
		}
#if DEBUGGER
		if (config.debug)
			unlink(file);
#endif
		} break;
	default:
		cons_printf("Usage: a[ocdg] [depth]\n");
		cons_printf(" ao [num]     analyze N opcodes\n");
		cons_printf(" aos [num]    show size of N opcodes\n");
		cons_printf(" am [n] [fmt] analyze memory format manager (am? and pm? fmi)\n");
		cons_printf(" ab [num]     analyze N code blocks\n");
		cons_printf(" ap [size]    analyze function preludes from here to <size> (default filesize)\n");
		cons_printf(" af [size]    analyze function\n");
		//cons_printf(" aF [size]    analyze function (recursively)\n");
		cons_printf(" ac [num]     disasm and analyze N code blocks\n");
		cons_printf(" aC[f] [num]  analyze calls of N bytes or 'f'unction size\n");
		cons_printf(" ad [num]     analyze N data blocks \n");
		cons_printf(" ag [depth]   graph analyzed code (agd = dot format) (check 'gu?')\n");
		cons_printf(" ar [args]    analyze ranges\n");
		cons_printf(" as [name]    analyze spcc structure (uses dir.spcc)\n");
		cons_printf(" at [args]    analyze opcode traces\n");
		cons_printf(" av[?] [arg]  analyze code with virtual machine\n");
		cons_printf(" ax           analyze xrefs\n");
		break;
	}
	config.seek = oseek;
	return 0;
}

CMD_DECL(project)
{
	char *arg = input + 2;
	char* str;
	switch (input[0]) {
	case 'o':
		if (input[1])
			project_open(arg);
		else {
			str = strdup ( config_get("file.project") );
			project_open(str);
			free ( str );
		}
		break;
	case 's':
		if (input[1])
			project_save(arg);
		else {
			str = strdup(config_get("file.project") );
			project_save(str);
			free(str);
		}
		break;
	case 'i':
		if (input[1]) {
			project_info(arg);
		} else {
			str = strdup(config_get("file.project"));
			project_info(str);
			free(str);
		}
		break;
	default:
		cons_strcat(
		"Usage: P[osi] [file]\n"
		" Po [file]  open project\n"
		" Ps [file]  save project\n"
		" Pi [file]  info\n");
		break;
	}
	return 0;
}

CMD_DECL(dbg)
{
	switch(input[0]) {
	case 'r':
		return radare_cmd("!regs", 0);
	case 's':
		return radare_cmd("!step", 0);
	case 'S':
		return radare_cmd("!stepo", 0);
	case 'c':
		return radare_cmd("!cont", 0);
	default:
		cons_printf(
		"Usage: d[rsc] [arg]\n"
		" dr   ; show registers\n"
		" dc   ; continue\n"
		" ds   ; step into\n"
		" dS   ; step over\n");
		break;
	}
	return 0;
}

/* XXX: DEPREACTED BY Ve */
#if 1
CMD_DECL(menu)
{
	char buf[128];
	char buf2[128];
	char pfx[128];

	pfx[0]='\0';
	while (1) {
		cons_printf("Menu: (q to quit)\n");
		if (pfx[0]) {
			cons_printf("* %s\n", pfx);
			sprintf(buf2, "e %s. | cut -d . -f 2 | sort | awk '{print \" - \"$1\"   \t\"$2\"\t\"$3$4$5$6}'", pfx);
			radare_cmd(buf2, 0);
		} else  radare_cmd("e | sort | cut -d . -f 1 | uniq | awk '{print \" - \"$1}'", 0);
	noprint:
		cons_printf("> ");
		cons_flush();
		fgets(buf, 128, stdin);
		if (buf[0])
			buf[strlen(buf)-1]='\0';
		if (strstr(buf, "..")) {
			pfx[0] = '\0';
			continue;
		}
		if (buf[0]=='q')
			break;

		if (strchr(buf, '=')) {
			sprintf(buf2, "e %s.%s", pfx, buf);
			radare_cmd(buf2, 0);
		} else
		if (pfx[0]) {
			cons_printf("%s.%s = ", pfx, buf);
			sprintf(buf2, "e %s.%s", pfx, buf);
			radare_cmd(buf2, 0);
			goto noprint;
		} else strcpy(pfx, buf);
	}
	return 0;
}
#endif

CMD_DECL(hack)
{
	if (strnull(input))
		return radare_hack_help();
	return radare_hack(input);
}

CMD_DECL(graph)
{
	char *text = input;
	char *eof = input + strlen(input)-1;

	for(;*text&&!iswhitespace(*text);text=text+1);
	for(;*text&&iswhitespace(*text);text=text+1);
	for(;iswhitespace(eof[0]);eof=eof-1) eof[0]='\0';

	if (input[0]=='?') {
		rdb_help();
		return 0;
	}
	switch(input[0]) {
	case 'u':
		switch(input[1]) {
		case 'r':
			ugraph_reset();
			break;
		case 'n':
			{
			char *p0, *p1, *p2;
			p0 = input+3;
			if (!p0) {
				eprintf("Usage: gun [addr] [size] [rcmd]\n");
				break;
			}
			p1 = strchr(p0, ' ');
			if (!p1) {
				eprintf("Usage: gun [addr] [size] [rcmd]\n");
				break;
			} *p1 = '\0';
			p2 = strchr(p1+1, ' ');
			if (!p2) {
				eprintf("Usage: gun [addr] [size] [rcmd]\n");
				break;
			} *p2 = '\0';
			ugraph_node(get_math(p0), get_math(p1+1), p2+1);
			}
			break;
		case 'e':
			{
			char *p0, *p1;
			p0 = input+3;
			if (!p0) {
				eprintf("Usage: gue [from] [to]\n");
				break;
			}
			p1 = strchr(p0, ' ');
			if (!p1) {
				eprintf("Usage: gue [from] [to]\n");
				break;
			} *p1 = '\0';
			ugraph_edge(get_math(p0), get_math(p1+1));
			}
			break;
		case 'd':
			if (input[2]=='v') {
				radare_cmd_raw("gud > .file.dot", 0);
				radare_cmd_raw("!!dot -T png -o .file.png .file.dot ", 0);
				radare_cmd_raw("!!rsc view .file.png", 0);
				radare_cmd_raw("!!rm -f .file.png .file.dot", 0);
			} else ugraph_print_dot();
			break;
		case 'k':
			// TODO
			//ugraph_key(input[0]
			break;
		case 'v':
			graph_set_user(1);
			grava_program_graph(NULL, NULL); //, NULL);
			graph_set_user(0);
			break;
		default:
			eprintf("Usage: gu[dnerv] [args]\n"
			" gur              user graph reset\n"
			" gun $$ $$b pd    add node\n"
			" gue $$F $$t      add edge\n"
			" gud              generate graph in dot format\n"
			" gudv             view graph in dot format (like agdv does)\n"
			//" guk s !step      add keybinding\n"
			//" gum Step !step   add contextual menu entry\n"
			" guv              visualize user defined graph\n");
		}
		break;
	case ' ': {
		struct program_t *prg = program_open(input+1); // XXX FIX stripstring and so
		if (prg != NULL)
			list_add_tail(&prg->list, &config.rdbs);
		return 0;
		}
	case 'a':
		return radare_cmdf("ac %d@%s", (int)config_get_i("graph.depth"), input+1);
	case 's': {
		int num = atoi(text+1);
		int i = 0;
		struct list_head *pos;
		list_for_each(pos, &config.rdbs) {
			struct program_t *prg = list_entry(pos, struct program_t, list);
			if (i ==  num) {
				program_save(prg);
				return 0;
			}
			i++;
		}
		eprintf("Not found\n");
		return 0;
		}
	case 'c': {
		/* XXX segfaults ?? */
		struct list_head *pos, *head;
		list_for_each(pos, &config.rdbs) {
			struct program_t *prg = list_entry(pos, struct program_t, list);
			list_for_each_prev(head, &(prg->blocks)) {
				char str[128];
				struct block_t *b0 = list_entry(head, struct block_t, list);
				string_flag_offset(NULL, str, b0->addr, -1);
				cons_printf("0x%08llx : %s ",
					b0->addr, str);
				if (b0->tnext != 0)
					cons_printf(": 0x%08llx -> 0x%08llx",  b0->tnext, b0->fnext);
				cons_printf("\n");
				//udis_arch_buf(config.arch, b0->bytes, b0->n_bytes, 0);
				radis_str_e(config.arch, b0->bytes, b0->n_bytes, 0);
			}
		}
		}
		break;
	case 'g': {
#if HAVE_GUI
		int num = atoi(text);
		int i = 0;
		struct list_head *pos;
		list_for_each(pos, &config.rdbs) {
			struct program_t *mr = list_entry(pos, struct program_t, list);
			if (i ==  num) {
				grava_program_graph(mr, NULL);
				return 0;
			}
			i++;
		}
		eprintf("Not found\n");
#else
		eprintf("Compiled without vala-gtk\n");
#endif
		return 0;
		}
	case 'r':
		switch(input[1]) {
		case '*': {
			/* SHOW LIST IN ar+ format */
			struct list_head *pos, *head;
			list_for_each(pos, &config.rdbs) {
				struct program_t *prg = list_entry(pos, struct program_t, list);
				list_for_each_prev(head, &(prg->blocks)) {
					struct block_t *b0 = list_entry(head, struct block_t, list);
					cons_printf("ar+ 0x%08llx 0x%08llx\n",
						b0->addr, b0->addr+b0->n_bytes);
				}
			} }
			break;
		default: {
			/* SHOW LIST IN ar+ format */
			struct list_head *pos, *head;
			list_for_each(pos, &config.rdbs) {
				struct program_t *prg = list_entry(pos, struct program_t, list);
				list_for_each_prev(head, &(prg->blocks)) {
					struct block_t *b0 = list_entry(head, struct block_t, list);
					cons_printf("0x%08llx 0x%08llx branch 0x%08llx, 0x%08llx\n",
						b0->addr, b0->addr+b0->n_bytes, b0->tnext, b0->fnext);
				}
			}
			}
		}
		break;
	case 'd': {
		int a, b;
		int i = 0;
		struct list_head *pos;
		struct program_t *mr0 = NULL;
		struct program_t *mr1 = NULL;

		sscanf(text, "%d %d", &a, &b);

		eprintf("RDB diffing %d vs %d\n", a, b);
		list_for_each(pos, &config.rdbs) {
			struct program_t *mr = list_entry(pos, struct program_t, list);
			if (i ==  a) mr0 = mr;
			if (i ==  b) mr1 = mr;
			i++;
		}

		if (mr0 != NULL && mr1 != NULL)
			rdb_diff(mr0, mr1, 0);
		else eprintf("!mr0 || !mr1 ??\n");
		return 0;
		}
		break;
	default:
		/* list */
		switch(text[0]) {
		case '-': { /* remove */
			int num = atoi(text+1);
			int i = 0;
			struct list_head *pos;
			list_for_each(pos, &config.rdbs) {
				struct program_t *mr = list_entry(pos, struct program_t, list);
				if (i ==  num) {
					list_del(&(mr->list));
					return 0;
				}
				i++;
			}
			eprintf("Not found\n");
			return 0;
			}
		default: {
			int i = 0;
			char offstr[245];
			struct list_head *pos;
			list_for_each(pos, &config.rdbs) {
				struct program_t *mr = list_entry(pos, struct program_t, list);
				fflush(stdout);
				offstr[0]='\0';
				string_flag_offset(NULL, offstr, mr->entry, 0);

				cons_printf("%02d 0x%08llx %s\n", i, (ut64)mr->entry, offstr);
				i++;
			}
			return 0;
			}
		}
	}

	return 0;
}

CMD_DECL(vaddr)
{
	int i;
	for(i=0;input[i]&&input[i]!=' ';i++);
	if (input[i]!='\0')
		config.vaddr = get_math(input+i+1);
	D { cons_printf("0x%08llx\n", config.vaddr); }

	return 0;
}

CMD_DECL(hash)
{
	int i;
	char buf[1024];
	char *str;
	ut64 bs = config.block_size;
	ut64 obs = config.block_size;

	str = strchr (input, ' ');
	if (str)
		bs = get_math (str+1);

	for(i=0;input[i];i++) if (input[i]==' ') { input[i]='\0'; break; }

	if (config.debug) {
		radare_set_block_size_i(bs);
		snprintf(buf, 1000, "!rahash -fa '%s' $BLOCK", input);
		radare_cmd_raw(buf, 0);
		radare_set_block_size_i(obs);
	} else {
		snprintf(buf, 1000, "rahash -a '%s' -S %lld -L %lld '%s'", // | head -n 1", 
			input, (ut64)config.seek, (ut64)bs, config.file);
		io_system(buf);
	}
	return 0;
}

CMD_DECL(interpret)
{
	char *ptro = strdup(input);
	char *ptr = ptro;

	clear_string(ptr);

	if (ptr[0] == '\0')
		eprintf("Usage: . [filename]\n");
	else
	if (!radare_interpret( ptr ))
		eprintf("error: Cannot open file '%s'\n", ptr);

	free(ptro);

	return 0;
}

CMD_DECL(open)
{
	char *arg, *ptr = strdup(input);
//	clear_string(ptr);

	switch(ptr[0]) {
	case '\0':
		cons_printf("0x%08llx 0x%08llx %s\n", 0LL, config.size, config.file);
		io_map_list();
		break;
	case '?':
		cons_printf("Usage: o[-] [file] [offset] [delta] [size]\n");
		cons_printf(" o                         ; list opened files\n");
		cons_printf(" o /bin/ls                 ; open file\n");
 		cons_printf("; map file at offset skiping 100 bytes until 200 of the file\n");
		cons_printf(" o /lib/libc.so 0xC848000 100 200\n");
		cons_printf(" o- /lib/libc.so           ; unmap\n");
		break;
	case ' ':
		arg = strchr(ptr+1, ' ');
		if (arg != NULL) {
			char *arg2, *arg3;
			ut64 size = 0LL;
			ut64 delta = 0LL;
			arg[0]='\0';
			arg2 = strchr(arg+1, ' ');
			if (arg2) {
				*arg2='\0';
				arg3 = strchr(arg2+1, ' ');
				if (arg3) {
					*arg3='\0';
					size = get_math(arg3+1);
				}
				delta = get_math(arg2+1);
			}
			io_map(ptr+1, get_math(arg+1)-config.vaddr, delta, size);
		} else {
			/* XXX */
			config.file = ptr+1;
			radare_open(0);
		}
	}
	free(ptr);
	return 0;
}

// XXX should be removed
CMD_DECL(envvar)
{
	char *text2;
	char *text = input;
	char *ptr, *ptro;

	if (text[0]=='\0') {
		env_prepare("");
		printf("%%FILE        %s\n", getenv("FILE"));
		if (config.debug)
		printf("%%DPID        %s\n", getenv("DPID"));
		printf("%%BLOCK       /tmp/.radare.???\n");
		printf("%%SIZE        %s\n", getenv("SIZE"));
		printf("%%OFFSET      %s\n", getenv("OFFSET"));
		printf("%%CURSOR      %s\n", getenv("CURSOR"));
		printf("%%BSIZE       %s\n", getenv("BSIZE"));
		return 0;
	} else
	if (text[0]=='?') {
		printf("Get and set environment variables.\n");
		printf("To undefine use '-' or '(null)' as argument: f.ex: %%COLUMNS (null)\n");
		return 0;
	}

	/* Show only one variable */
	text = text2 = strdup(input);

	for(;*text&&!iswhitespace(*text);text=text+1);
	if ((text[0]=='\0') || text[1]=='\0') {
		char *tmp = getenv(text2);
		if (tmp) {
			text[0]='\0';
			cons_printf("%s\n", tmp);
			free(text2);
		} else 	{ cons_newline(); }
		return 0;
	}

	/* Set variable value */
	ptro = ptr = text;
	ptr[0]='\0';
	for(;*text&&iswhitespace(*text);text=text+1);
	for(ptr=ptr+1;*ptr&&iswhitespace(*ptr);ptr=ptr+1);
	if ((!memcmp(ptr,"-", 2)) || (!memcmp(ptr,"(null)", 5)))
		unsetenv(text2);
	else setenv(text2, ptr, 1);
	ptro[0]=' ';
	env_update();
	free(text2);

	return 0;
}

CMD_DECL(blocksize)
{
	flag_t *flag;
	switch(input[0]) {
	case 't': // bf = block flag size
		flag = flag_get(input+2);
		if (flag) {
			radare_set_block_size_i(flag->offset-config.seek);
			printf("block size = %d\n", flag->length);
		} else eprintf("Unknown flag '%s'\n", input+2);
		break;
	case 'f': // bf = block flag size
		flag = flag_get(input+2);
		if (flag) {
			radare_set_block_size_i(flag->length);
			printf("block size = %d\n", flag->length);
		} else eprintf("Unknown flag '%s'\n", input+2);
		break;
	case '?':
		cons_printf("Usage: b[t,f flag]|[size]  ; Change block size\n"
		" b 200                ; set block size to 200\n"
		" bt next @ here       ; block size = next-here\n"
		" bf sym.main          ; block size = flag size\n");
		break;
	case '\0':
		cons_printf("%d\n", config.block_size);
		break;
	default:
		radare_set_block_size(input);
	}
	return 0;
}

CMD_DECL(code)
{
	char *text = input;

	switch(text[0]) {
	case 'r':
		switch(text[1]) {
		case ' ':
			data_del_range(config.seek, config.seek+get_math(text+2));
			break;
		default:
			cons_printf("Usage: Cr [length] @ addr\n");
			break;
		}
		break;
	case 'C':
		/* comment */
		text = text+1;
		while(text[0]==' ')
			text = text + 1;
		cons_flush();
		if (text[0]=='\0') {
			data_comment_list("");
		} else if (text[0]=='?') {
			cons_printf("Usage: CC [-addr|comment] @ address\n"
				"adds or removes comments for disassembly\n");
		} else
		if (text[0]) {
			if (text[0]=='-')
				data_comment_del(config.seek, text+1);
			else	data_comment_add(config.seek, text);
		}
		break;
	case 'x': // code xref
		switch(text[1]) {
		case '\0':
			data_xrefs_list("Cx");
			break;
		case '-':
			data_xrefs_del(get_math(text+2)-config.vaddr, config.seek+config.vaddr, 0);
			break;
		default:
			{
			ut64 from = get_math(text+1);
			if (from >= config.vaddr*2)
				from -= config.vaddr;
			data_xrefs_add(from, config.seek+config.vaddr, 0);
			}
			break;
		}
		break;
	case 'X': // data xref
		switch(text[1]) {
		case '\0':
			data_xrefs_list("CX");
			break;
		case '-':
			data_xrefs_del(get_math(text+2)-config.vaddr,config.seek+config.vaddr, 1);
			break;
		default:
			data_xrefs_add(get_math(text+1)-config.vaddr,config.seek+config.vaddr, 1);
			break;
		}
		break;
	case 'i':
		data_info();
		break;
	case 'v': /* var types */
		// Cv int 4 d
		// Cv float 4 f
		data_var_cmd(text+1);
		break;
	case 'F':
		/* do code analysis here */
		switch(text[1]) {
		case 'V':
		case 'v': // function var
		case 'a': // function args
		case 'A':
			var_cmd(text+1);
			break;
#if 0
		case 's':
		case 'g': // set/get
			break;
#endif
		case 'r': // function ranges
			eprintf("TODO\n");
			break;
		case 'f': // function frame size
			eprintf("TODO\n");
			break;
		case '\0':
			radare_cmdf("C*C%c", text[0]);
			break;
		case '*':
			/* ranges */
			data_list_ranges();
			break;
		case ' ':
			if (text[2]=='\0') {
				eprintf("Oops. missing argument?\n");
			} else {
				ut64 len, tmp = config.block_size;
				len = get_math(text+2);
				radare_set_block_size_i(len);
				data_add_arg(config.seek+(config.cursor_mode?config.cursor:0), DATA_FUN, text+2);
				radare_set_block_size_i(tmp);
			}
			break;
		case '-':
			data_del(config.seek+(config.cursor_mode?config.cursor:0), DATA_FUN, atoi(text+2));
			break;
		case '?': // function help
		default:
			cons_printf(
			"Usage: CF[afrv.][gs] [args]\n"
			" CF 20 @ eip                   ; create new function of 50 byte length at eip\n"
			" CF- @ eip                     ; remove function definition at eip\n"
			" CFv.                          ; show variables for current function\n"
			" CFv 20 int                    ; define local var\n"
			" CFvg 20 @ 0x8048000           ; access 'get' to delta 20 var (creates var if not exist)\n"
			" CFvs 20 @ 0x8049000           ; access 'set' to delta 20 var (\"\")\n"
			" CFV @ 0x8049000               ; Show local variables and arg values at function\n"
			" CFa 0 int argc                ; define arg[0]\n"
			" CFa 4 char** argv             ; define arg[1]\n"
			" CFA 0 char** argv             ; define arg[1] (fastcall)\n"
			" CFf 320 @ fun.8058400         ; set frame size for function\n"
			" CFr 10-20,3040 @ fun.8048300  ; define ranges for non-linear function\n"
			" CFr 10-20,3040 @ fun.8048300  ; define ranges for non-linear function\n"
			);
			break;
		}
		break;
	case 'm':
	case 'c':
	case 'd':
	case 's':
	case 'f':
	case 'u': {
		struct data_t *d;
		int fmt, len;
		char *arg = text + 1;
		ut64 tmp = config.block_size;

		for(; *arg==' ';arg=arg+1);
		if (arg[0]=='\0') {
			radare_cmdf("C*C%c", text[0]);
		} else
		if (arg[0]=='*') {
			/* ranges */
			data_list_ranges();
		} else {
			len = get_math(arg);
			switch(text[0]) {
				case 'm': fmt = DATA_STRUCT; break;
				case 'c': fmt = DATA_CODE; break;
				case 'd': fmt = DATA_HEX; break;
				case 's': fmt = DATA_STR; break;
				case 'F': fmt = DATA_FUN; break;
				case 'f': fmt = DATA_FOLD_C; break;
				case 'u': fmt = DATA_FOLD_O; break;
				default:  fmt = DATA_HEX; break;
			}
			//arg = strchr(arg, ' ');
			if (arg != NULL)
				arg = arg + 1;
			tmp = config.block_size;
			radare_set_block_size_i(len);
			d = data_add_arg(config.seek+(config.cursor_mode?config.cursor:0), fmt, arg);
			radare_set_block_size_i(tmp);
		}
		} break;
	case 'L':
		{
		char buf[1024];
		snprintf(buf, 1023, "rsc dwarf-at %s 0x%08llx",
			config.file, config.seek);
		system(buf);
		}
		break;
	case '*':
		data_comment_list(input+1);
		data_xrefs_list(input+1);
		data_list(input+1);
		break;
	default:
		cons_printf(
		"Usage: C[op] [arg] <@ offset>\n"
		" Ci                     ; show info about metadata\n"
		" CL                     ; show debug information at $$\n"
		" Cr [length] @ addr     ; removes all metadata from here to length\n"
		" CC [-][comment] @ here ; add/rm comment\n"
		" CF [-][len]  @ here    ; add/rm linear function\n"
		" Cx [-][addr] @ here    ; add/rm code xref\n"
		" CX [-][addr] @ here    ; add/rm data xref\n"
		" Cm [num] [expr]        ; define memory format (pm?)\n"
		" Cc [num]               ; converts num bytes to code\n"
		" Cd [num]               ; converts to data bytes\n"
		" Cs [num]               ; converts to string\n"
		" Cf [num]               ; folds num bytes\n"
		" Cu [num]               ; unfolds num bytes\n"
		" Cv [type] [sz] [fmt]   ; define var type (Cv?)\n"
		" CF[afrv] [args] @ addr ; function-level local variable\n"
		" CF*                    ; list function ranges as ar cmds\n"
		" C*                     ; list metadata database\n");
	}
	return 0;
}

#if 0
CMD_DECL(endianess)
{
	char *text = input;
	for(;*text&&!iswhitespace(*text);text=text+1);
	for(;*text&&iswhitespace(*text);text=text+1);

	if (text[0]!='\0'&&text[0]!=' ')
		config.endian = atoi(text)?1:0;

	D eprintf("endian = %d %s\n", config.endian, (config.endian)?"big":"little");
	return 0;
}
#endif

static void radare_set_limit(char *arg)
{
	if ( arg[0] != '\0' )
		config.limit = get_math(arg);
	D eprintf("limit = %lld\n", config.limit);
}

CMD_DECL(limit)
{
	char *text = input;
	for(;*text&&!iswhitespace(*text);text=text+1);
	for(;*text&&iswhitespace(*text);text=text+1);

	radare_set_limit(text);

	return 0;
}

#if 0
CMD_DECL(move)
{
	char *text = input;
	for(;*text&&!iswhitespace(*text);text=text+1);
	for(;*text&&iswhitespace(*text);text=text+1);
	radare_move(text);

	return 0;
}
#endif

CMD_DECL(print)
{
	print_fmt_t fmt = MD_BLOCK;
	int bs = -1;

	switch(input[0]) {
	case '\0':
	case '?':
		format_show_help( fmt );
		break;
	case 'I':
		fmt = format_get(input[1]); //, fmt);
		if (fmt == FMT_ERR)
			format_show_help(MD_BLOCK|MD_ALWAYS|MD_EXTRA);
		else	radare_print(input+1, fmt);
		break;
	default:
		if (input[1]=='f') {
			struct data_t *data = data_get(config.seek); // TODO: use get_math(input+2) ???
			if (data) {
				bs = config.block_size;
				radare_set_block_size_i(data->size);
			}
		} else {
			if (strchr(input, ' ')) {
				ut64 num = get_math (input+1);
				bs = config.block_size;
				if (input[0]!='d')
					radare_set_block_size_i(num);
			}
		}
		fmt = format_get(input[0]); //, fmt);
		if (fmt == FMT_ERR)
			format_show_help(MD_BLOCK|MD_ALWAYS|MD_EXTRA);
		else	radare_print(input+1, fmt);
	}
	if (bs != -1)
		radare_set_block_size_i(bs);

	return 0;
}

CMD_DECL(quit)
{
	switch(input[0]) {
	case '!':
		exit(atoi(input+1));
	case '?':
		cons_strcat(
		"Usage: q[!]\n"
		" q      ; quit radare\n"
		" q!     ; quit radare NOW!\n");
		break;
	default:
		radare_exit(atoi(input+1));
	}
	return 0;
}

CMD_DECL(resize)
{
	char *text = input;

	if (strchr(input, '*')) {
		printf("0x"OFF_FMTx"\n", config.size);
		return 0;
	}
	/* XXX not needed ? */
	for(;*text&&!iswhitespace(*text);text=text+1);
	for(;*text&&iswhitespace(*text);text=text+1);

	radare_resize(text);

	return 0;
}

static int flag_ctr = 0;
CMD_DECL(flag)
{
	int ret = 0;
	char *text = input;
	char *text2;
	char *eof = input + strlen(input)-1;

	if (input[0]!='-')
	for(;*text&&!iswhitespace(*text);text=text+1);
	for(;*text&&iswhitespace(*text);text=text+1);
	for(;iswhitespace(eof[0]);eof=eof-1) eof[0]='\0';

	switch(input[0]) {
	case '*': flag_set("*",0,0); break;
	case 'o': radare_fortunes(); break;
	case '?': flag_help(); break;
	case 'h': cons_printf("%s\n", flag_get_here_filter(config.seek, input+2)); break;
	case 'g': 
		switch(input[1]) {
		case 'n':
			flag_grep_np(text, config.seek, 0);
			break;
		case 'p':
			flag_grep_np(text, config.seek, 1);
			break;
		case ' ':
			flag_grep(text);
			break;
		case '?':
			flag_grep_help();
			break;
		}
		break;
	case 'b': flag_rebase(text); break;
	case 'c': flag_cmd(text); break;
	case 'f': flag_from(text); break;
	case 'r': flag_rename_str(text); break;
	case 's': flag_space(input+1); break;
	case 'u': flag_set_undef(input+2, config.seek, 0); break;
	case 'm': flag_space_move(text); break;
	case 'd': print_flag_offset(text, config.seek); cons_newline(); break;
	case 'i':
		text2=strchr(text,' ');
		if (text2) {
			text2[0]='\0';
			text2 = text2+1;
			flag_interpolation(text, text2); 
		} else eprintf("Usage: fi hit0_ hit1_\n");
		break;
	case 'N':
		if (input[1]!='\0') {
			char bu[128];
			snprintf(bu, 127, "%s_%d",text, flag_ctr++); 
			while(bu[0]==' ')strcpy(bu, bu+1);
			eprintf("0x%08llx %s\n", config.seek, bu);
			flag_set(bu, config.seek,0);
		} else flag_ctr = 0;
		break;
	case '\0': flag_list(text); break;
	//case '*': flag_set("*",0,0); break;
	case '-': flag_remove(text+1); break;
	case ' ':
		if (input[1]!='-') {
			ut64 here = config.seek;//+config.vaddr;
			ut64 size = config.block_size;
			char *s = strchr(text, ' ');
			char *s2 = NULL;
			if (s) {
				*s='\0';
				s2 = strchr(s+1, ' ');
				if (s2) {
					*s2 = '\0';
					here = get_math(s2+1);
				}
				radare_set_block_size_i(get_math(s+1));
			}
			ret = flag_set(text, here, input[0]=='n');
			D { if (!ret) { flags_setenv(); } }
			if (s) *s=' ';
			if (s2) *s2=' ';
			if (size != config.block_size)
				radare_set_block_size_i(size);
		} else flag_remove(text+1);
	}
	return ret;
}

CMD_DECL(undowrite)
{
	switch(input[0]) {
	case '-':
		undo_write_clear();
		break;
	case 'a':
		undo_write_set_all(0);
		break;
	case 'r':
		undo_write_set_all(1);
		break;
	case 'w':
		input = input + 1;
	case '*':
		undo_write_list(1);
		break;
	case ' ':
	case '\0':
		if (input[0] != '\0') {
			if (input[1]=='-')
				undo_write_set(atoi(input+2), 1);
			else undo_write_set(atoi(input+1), 0);
		} else undo_write_list(0);
		break;
	case '?':
	default:
		cons_strcat(
		"Usage: u[-ar] [arg]\n"
		" u 3   ; undo write change at index 3\n"
		" u -3  ; redo write change at index 3\n"
		" u     ; list all write changes\n"
		" u*    ; list all write changes (in radare commands)\n"
		" u-    ; clear write history\n"
		" ua    ; undo all write changes\n"
		" ur    ; redo all write changes\n");
		break;
	}

	return 0;
}

CMD_DECL(seek)
{
	ut64 new_off = 0;
	struct aop_t aop;
	char *input2, *text;
	int whence    = SEEK_SET;
	int len,sign  = 1;
	
	len = strlen(input)+1;
	text = input2 = alloca(len);
	memcpy(input2, input, len);

	for(;*text&&!iswhitespace(*text);text=text+1);
	for(;*text&&iswhitespace(*text);text=text+1);

	if (strchr(input, '?')) {
		cons_strcat("Usage: s[nbcxXS-+*!] [arg]\n"
		" s 0x128 ; absolute seek\n"
		" s +33   ; relative seek\n"
		" s/ lib  ; seek to first hit of search\n"
		" s/x 00  ; seek to first hit of search\n"
		" s\\ lib  ; backward seek to the previous string hit\n"
		" s\\x 23  ; backward seek to the previous hex hit\n"
		" sn      ; seek to next opcode\n"
		" sb      ; seek to opcode branch\n"
		" sc      ; seek to call index (pd)\n"
		" sx N    ; seek to code xref N\n"
		" sX N    ; seek to data reference N\n"
		" sS N    ; seek to section N (fmi: 'S?')\n"
		" s-      ; undo seek\n"
		" s+      ; redo seek\n"
		" s*      ; show seek history\n"
		" .s*     ; flag them all\n"
		" s!      ; reset seek history\n");
		return 0;
	}

	text = input;
#if 0
	if (input[0]=='n'||input[0]=='b'||input[0]=='c'||input[0]=='!'||input[0]=='*'||input[0]=='+'||input[0]=='-'||(input[0]>='0'&&input[0]<'9'))
		text = input;
#endif

	if (text[0] == '\0') {
		D printf(OFF_FMT"\n", (ut64)config.seek);
		return 0;
	}
	for(;text[0]==' ';text = text +1);

	switch(text[0]) {
	case '\\':
		radare_seek_search_backward(text+1);
		return;
	case '/':
		radare_seek_search(text+1);
		return;
	case 'x': 
		new_off = data_seek_to(config.seek, 0, atoi(text+1));
		break;
	case 'X': 
		new_off = data_seek_to(config.seek, 1, atoi(text+1));
		break;
	case 'S': {
		struct section_t *s = section_get_i(atoi(text+1));
		if (s != NULL) 
			radare_seek(s->from, SEEK_SET);
		}
		break;
	case 'c': 
		sign = atoi (text+1);
		if (sign > 0)
			udis_jump(sign-1);
		return 0;
	case 'n': 
		radare_read(0);
		arch_aop(config.seek, config.block, &aop);
		new_off = config.seek + aop.length;
		break;
	case 'b': 
		radare_read(0);
		arch_aop(config.seek, config.block, &aop);
		if (aop.jump != 0)
			new_off = aop.jump;
		break;
	case '!': sign = -2; text++; break;
	case '*': sign = 0; text++; break;
	case '-': sign = -1; text++; whence = SEEK_CUR; break;
	case '+': sign = 1;  text++; whence = SEEK_CUR; break; 
	}
	if (input[text-input]=='\0') {
		switch(sign) {
		case 0:
			undo_list();
			break;
		case 1:
			eprintf("redo seek\n");
			undo_redo();
			break;
		case -1:
			eprintf("undo seek\n");
			undo_seek();
			break;
		case -2:
			eprintf("undo history reset\n");
			undo_reset();
			break;
		}
		return 0;
	}
	if (new_off == 0)
		new_off = get_math( text );

	if (whence == SEEK_CUR) {
		new_off *= sign;
		new_off += config.seek;
		whence   = SEEK_SET; // stupid twice
	}

	//if (new_off<0) new_off = 0;
	if (1 || (text[0]=='0'&&new_off==0) || new_off!=0) {
		if (radare_seek(new_off, whence) < 0)
			eprintf("Couldn't seek: %s\n", strerror(errno));
	}
	undo_push();
	radare_read(0);

	return 0;
}

CMD_DECL(info)
{
#if 0
	if (strchr(input, '*')) {
		printf("b %d\n", config.block_size);
		printf("e %d\n", config.endian);
		printf("s 0x"OFF_FMTx"\n", config.seek);
		printf("B 0x"OFF_FMTx"\n", config.vaddr);
		printf("l 0x"OFF_FMTx"\n", config.limit);
		return 0;
	}
#endif
	cons_printf(" file    %s",   strget(config.file)); cons_newline();
	cons_printf(" rdb     %s",   strget(config_get("file.rdb"))); cons_newline();
	cons_printf(" project %s",   strget(config_get("file.project"))); cons_newline();
	cons_printf(" mode    %s",   config_get("file.write")?"read-write":"read-only"); cons_newline();
	cons_printf(" debug   %d",   config.debug); cons_newline();
	cons_printf(" endian  %d  ( %s )",   config.endian, config.endian?"big":"little"); cons_newline();
	cons_printf(" vaddr   "OFF_FMTd"  ( 0x"OFF_FMTx" )", config.vaddr, config.vaddr); cons_newline();
	cons_printf(" bsize   %d  ( 0x%x )", config.block_size, config.block_size); cons_newline();
	cons_printf(" seek    "OFF_FMTd" 0x"OFF_FMTx,
		(ut64)config.seek, (ut64)config.seek); cons_newline();
	cons_printf(" delta   %lld\n", config_get_i("cfg.delta")); 
	cons_printf(" count   %lld\n", config_get_i("cfg.count")); 
	//fflush(stdout);
	//print_flag_offset(config.seek);
	cons_printf(" size    "OFF_FMTd" \t 0x"OFF_FMTx,
		(ut64)config.size, (ut64)config.size); cons_newline();
	cons_printf(" limit   "OFF_FMTd" \t 0x"OFF_FMTx,
		(ut64)config.limit, (ut64)config.limit); cons_newline();

	if (config.debug) {
		cons_printf("Debugger:\n");
		io_system("info");
	}

	return 0;
}

CMD_DECL(compare)
{
	int ret;
	FILE *fd;
	unsigned int off;
	unsigned char *buf;

	radare_read(0);
	switch (input[0]) {
	case 'c':
		radare_compare_code(get_math(input+1), config.block, config.block_size);
		break;
	case 'd':
		off = (unsigned int) get_offset(input+1);
		radare_compare((u8*)&off, config.block, 4);
		break;
	case 'f':
		if (input[1]!=' ') {
			eprintf ("Please. use 'cf [file]'\n");
			return 0;
		}
		fd = fopen(input+2, "r");
		if (fd == NULL) {
			eprintf("Cannot open file '%s'\n",input+2);
			return 0;
		}
		buf = (unsigned char *)malloc(config.block_size);
		fread(buf, 1, config.block_size, fd);
		fclose(fd);
		radare_compare(buf, config.block, config.block_size);
		free(buf);
		break;
	case 'x':
		if (input[1]!=' ') {
			eprintf("Please. use 'cx 00 11 22'\n");
			return 0;
		}
		buf = (unsigned char *)malloc(strlen(input+2));
		ret = hexstr2binstr(input+2, buf);
		radare_compare(buf, config.block, ret);
		free(buf);
		break;
	case 'X':
		{
		u8 *buf = malloc(config.block_size);
		radare_read_at(get_math(input+1), buf, config.block_size);
		radare_compare_hex(config.seek+config.vaddr, buf, config.block, config.block_size);
		free(buf);
		}
		break;
	case ' ':
		radare_compare((unsigned char*)input+1,config.block, strlen(input+1)+1);
		break;
	case 'D':
		{
		char cmd[1024];
		sprintf(cmd, "radiff -b %s %s", ".curblock", input+2);
		file_dump(".curblock", config.block, config.block_size);
		radare_system(cmd);
		unlink(".curblock");
		}
		break;
	case '?':
		cons_strcat(
		"Usage: c[?cdfx] [argument]\n"
		" c  [string]   - compares a plain with escaped chars string\n"
		" cc [offset]   - code bindiff current block against offset\n"
		" cd [value]    - compare a doubleword from a math expression\n"
		" cx [hexpair]  - compare hexpair string\n"
		" cX [addr]     - like 'cc' but using hexdiff output\n"
		" cf [file]     - compare contents of file at current seek\n"
		" cD [file]     - like above, but using radiff -b\n");
		break;
	default:
		eprintf("Usage: c[?Ddxf] [argument]\n");
		return 0;
	}

	return 0;
}

CMD_DECL(write)
{
	int ret;
	ut64 delta = 0;
	ut64 off;
	ut64 here = config.seek + (config.cursor_mode?config.cursor:0);
	ut64 back = config.seek;

	radare_seek(here, SEEK_SET);
	switch (input[0]) {
	case 'F':
		if (input[1]!=' ') {
			eprintf("Please. use 'wF [hexpair-file]'\n");
			return 0;
		} else {
			/* TODO: move to radare_poke_hexpairs() */
			int n, commented=0;
			u8 c;
			FILE *fd = fopen(input+2, "r");
			if (fd) {
				while(!feof(fd)) {
					if (!commented && fscanf(fd, "%02x", &n)){
						if (feof(fd))
						        break;

						c = n;

						radare_write_at(config.seek+delta, &c, 1);
						delta++;
					} else {
						fscanf(fd, "%c", &n);
						if (feof(fd))
							break;
						c = n;
						if (c == '#') commented = 1;
						else if (c == '\n' || c == '\r') commented = 0;
					}
				}
				fclose(fd);
			} else {
				eprintf("Cannot open file '%s'\n", input+2);
				return 0;
			}
		}
		break;
	case 'v':
		off = get_math(input+1);
		if (off&0xFFFFFFFF00000000LL) {
			/* 8 byte addr */
			unsigned long long addr8;
			endian_memcpy((u8*)&addr8, (u8*)&off, 8);
			undo_write_new(config.seek, (const u8*) &addr8, 8);
			io_write(config.fd, &addr8, 8);
		} else {
			/* 4 byte addr */
			unsigned long addr4_, addr4 = (unsigned long)off;
			drop_endian((u8*)&addr4_, (u8*)&addr4, 4); /* addr4_ = addr4 */
			endian_memcpy((u8*)&addr4, (u8*)&addr4_, 4); /* addr4 = addr4_ */
			undo_write_new(config.seek, (const u8*) &addr4, 4);
			//eprintf("VALUE: %x\n", addr4);
			io_write(config.fd, &addr4, 4);
		}
		break;
	case 'b': {
		char *tmp;
		u8 out[9999]; // XXX
		int size, osize = hexstr2binstr(input+1, out);
		if (osize>0) {
			tmp = (char *)malloc(config.block_size);
			memcpy_loop(tmp, out, config.block_size, osize);
			undo_write_new(config.seek, tmp, config.block_size);
			io_write(config.fd, tmp, config.block_size);
			free(tmp);
		} else {
			eprintf("Usage: wb 90 90\n");
		} }
		break;
	case 'f':
		if (input[1]!=' ') {
			eprintf("Please. use 'wf [file]'\n");
			return 0;
		}
		radare_poke(input+2);
		break;
	case 'A': {
		char data[1024];
		snprintf(data, 1023, "wx `!!rsc asm '%s'", input + 2);
		radare_cmd(data, 0);
		} break;
	case 'a': {
			int ret, delta = 0;
			unsigned char data[256];
			char *oinput, *ptr, *aux = strdup ( config_get("asm.arch") );
			ut64 seek = config.seek + config.vaddr;
			oinput= input = strdup(input+2);
			while(input) {
				ptr = strchr(input, ';');
				if (ptr) ptr[0]='\0';
//eprintf("assembling(%s)\n", input);
				ret = rasm_asm(aux, &seek, input, data);
				if (ret<1)
					eprintf("Invalid opcode for asm.arch. Try 'wa?'\n");
				else {
					undo_write_new(config.seek+delta, data, ret);
					radare_write_at(config.seek+delta, data, ret);
					delta+=ret;
				}
				if (ptr ) input = ptr+1;
				else input = NULL;
			}
			free ( aux );
			free ( oinput );
		} break;
	case 'x':
		if (input[1]!=' ') {
			eprintf("Usage: 'wx 00 11 22'\n");
			return 0;
		}
		ret = radare_write(input+2, WMODE_HEX);
		break;
	case 'o':
		switch(input[1]) {
		case 'a':
		case 's':
		case 'A':
		case 'x':
		case 'r':
		case 'l':
		case 'm':
		case 'd':
		case 'o':
			if (input[2]!=' ') {
				eprintf("Usage: 'wo%c 00 11 22'\n",input[1]);
				return 0;
			}
		case '2':
		case '4':
			ret = radare_write_op(input+3, input[1]);
			break;
		case 'n':
			ret = radare_write_op ("ff", 'x');
			break;
		case '\0':
		case '?':
		default:
			eprintf(
			"Usage: wo[xrlasmd] [hexpairs]\n"
			"Example: wox 90    ; xor cur block with 90\n"
			"Example: woa 02 03 ; add 2, 3 to all bytes of cur block\n"
			"Supported operations:\n"
			"  woa  addition            +=\n"
			"  wos  substraction        -=\n"
			"  wom  multiply            *=\n"
			"  won  negate all bits (same as wox ff)\n"
			"  wod  divide              /=\n"
			"  wox  xor                 ^=\n"
			"  woo  or                  |=\n"
			"  woA  and                 &=\n"
			"  wor  shift right         >>=\n"
			"  wol  shift left          <<=\n"
			"  wo2  2 byte endian swap  2=\n"
			"  wo4  4 byte endian swap  4=\n"
			);
			break;
		}
		break;
	case 'm':
		radare_write_mask_str(input+1);
		break;
	case 'w':
		if (input[1]!=' ') {
			eprintf("Please. use 'ww string-with-scaped-hex'. (%s)\n", input);
			return 0;
		}
		ret = radare_write(input+2, WMODE_WSTRING);
		break;
	case 'T':
		/* TODO: Use libr_io here */
		if (input[1]!=' ') {
			eprintf("Please. use 'wT prefix-file [size]'.\n");
			return 0;
		} else {
			ut64 bsize = config.block_size;
			static counter = 0;
			char file[1024];
			char *off = strchr(input+2, ' ');
			int fd;
			if (off) {
				*off = '\0';
				off = off + 1;
				radare_set_block_size_i(get_math(off));
			}
			sprintf(file, "%s-%d", input+2, counter);
			fd = open(file, O_RDWR|O_TRUNC|O_CREAT, 0644);
			if (fd == -1) {
				eprintf("Cannot open file '%s'\n", input+2);
			} else {
				radare_read(0);
				write(fd, config.block, config.block_size);
				counter++;
				close(fd);
			}
			if (bsize != config.block_size)
				radare_set_block_size_i(bsize);
		}
		break;
	case 't':
		/* TODO: Use libr_io here */
		if (input[1]!=' ') {
			eprintf("Please. use 'wt file [offset]'.\n");
			return 0;
		} else {
			char *off = strchr(input+2, ' ');
			int fd;
			if (off) {
				*off = '\0';
				off = off + 1;
			}
			fd = open(input+2, off?O_RDWR|O_CREAT:O_RDWR|O_TRUNC|O_CREAT, 0644);
			if (fd == -1) {
				eprintf("Cannot open file '%s'\n", input+2);
			} else {
				if (off)
					lseek(fd, get_math(off), SEEK_SET);
radare_read(0);
				write(fd, config.block, config.block_size);
				close(fd);
			}
		}
		break;
	case ' ':
		ret = radare_write(input+1, WMODE_STRING);
		break;
	case '?':
	default:
		eprintf(
		"Usage: w[?|*] [argument]\n"
		" w  [string]        ; write plain with escaped chars string\n"
		" wa [opcode]        ; write assembly using asm.arch and rasm\n"
		" wA '[opcode]'      ; write assembly using asm.arch and rsc asm\n"
		" wb [hexpair]       ; circulary fill the block with these bytes\n"
		" wm [hexpair]       ; write mask (default: FF FF FF..)\n"
		" wv [expr]          ; writes 4-8 byte value of expr (use cfg.bigendian)\n"
		" ww [string]        ; write wide chars (interlace 00s in string)\n"
		" wt [file] ([off])  ; write current block to file at offset\n"
		" wT [pfx] ([size])  ; write current block to file with size\n"
		" wf [file]          ; write contents of file at current seek\n"
		" wF [hexfile]       ; write hexpair contents of file\n"
		" wo[xrlaAsmd] [hex] ; operates with hexpairs xor,shiftright,left,add,sub,mul,div\n"
		"NOTE: Use 'yt' (yank to) command to copy data from one place to other. y? fmi\n");
		return 0;
	}
	radare_seek(back, SEEK_SET);

	return 0;
}

CMD_DECL(sections)
{
	switch(input[0]) {
	case '?':
		eprintf("Usage: S[cbtf=*] len [base [comment]] @ address\n"
		" S                ; list sections\n"
		" S*               ; list sections (in radare commands)\n"
		" S=               ; list sections (in nice ascii-art bars)\n"
		" Sa @ 0x804840    ; show section at address\n"
		" S 4096 0x80000 rwx section_text @ 0x8048000 ; adds new section\n"
		" S 4096 0x80000   ; 4KB of section at current seek with base 0x.\n"
		" S 10K @ 0x300    ; create 10K section at 0x300\n"
		" S -0x300         ; remove this section definition\n"
		" Sd 0x400 @ here  ; set ondisk start address for current section\n"
		" Sc rwx _text     ; add comment to the current section\n"
		" Sb 0x100000      ; change base address\n"
		" St 0x500         ; set end of section at this address\n"
		" Sf 0x100         ; set from address of the current section\n"
		" Sp 7             ; set rwx (r=4 + w=2 + x=1)\n");
		break;
	case ' ':
		switch(input[1]) {
		case '-': // remove
			section_rm(atoi(input+1));
			break;
		default:
			{
			int i;
			char *ptr = strdup(input+1);
			const char *comment = NULL;
			ut64 from = config.seek;
			ut64 to   = config.seek + config.block_size;
			ut64 base = config.vaddr; //config_get_i("io.vaddr");
			ut64 ondisk = 0LL;
			
			i = set0word(ptr);
			switch(i) {
			case 3: // get comment
				comment = get0word(ptr, 2);
			case 2: // get base address
				ondisk = get_math(get0word(ptr, 1));
			case 1: // get length
				to = from + get_math(get0word(ptr,0));
			}
			section_add(from, to, base, ondisk, 7, comment);
			free(ptr);
			}
			break;
		}
		break;
	case '=':
		section_list_visual(config.seek, config.block_size);
		break;
	case 'a':
		section_list(config.seek, 2);
		break;
	case '\0':
		section_list(config.seek, 0);
		break;
	case '*':
		section_list(config.seek, 1);
		break;
	case 'd':
		section_set(config.seek, -1, -1, get_math(input+1), -1, NULL);
		break;
	case 'c':
		section_set(config.seek, -1, -1, -1, -1, input+(input[1]==' '?2:1));
		break;
	case 'b':
		section_set(config.seek, -1, get_math(input+1), -1, -1, NULL);
		break;
	case 't':
		section_set(config.seek, get_math(input+1), -1, -1,-1, NULL);
		break;
	case 'p':
		section_set(config.seek, -1, -1, -1, atoi(input+1), NULL);
		break;
	case 'f':
		eprintf("TODO\n");
		break;
	}
	return 0;
}

CMD_DECL(examine)
{
	switch(input[0]) {
	case '\0':
		radare_print(input, FMT_HEXB);
		break;
	case ' ':
		radare_print(input+1, FMT_HEXB);
		break;
	default:
		eprintf("Error parsing command.\n");
		break;
	}

	return 0;
}

CMD_DECL(prev)
{
	ut64 off;
	if (input[0] == '\0')
		off = config.block_size;
	else	off = get_math(input);

	if (off > config.seek)
		config.seek = 0;
	else 	config.seek -= off;

	if (config.limit && config.seek > config.limit)
		config.seek = config.limit;
	radare_seek(config.seek, SEEK_SET);

	D printf(OFF_FMTd"\n", config.seek);

	return 0;
}

CMD_DECL(next)
{
	ut64 off;
	if (input[0] == '\0')
		off = config.block_size;
	else	off = get_math(input);

	config.seek += off;
	if (config.limit && config.seek > config.limit)
		config.seek = config.limit;
	radare_seek(config.seek, SEEK_SET);

	D printf(OFF_FMTd"\n", config.seek);

	return 0;
}

CMD_DECL(prev_align)
{
	int times;
	for(times=1; input[times-1]=='<'; times++);
	for(;times;times--) {
		config.seek -= config.block_size;
		if (config.seek<0) {
			config.seek = 0;
			break;
		}
		if ( config.seek % config.block_size )
			CMD_CALL(next_align, "")
	}
	undo_push();

	return 0;
}

CMD_DECL(next_align)
{
	int times;
	for(times=1; input[times-1]=='>'; times++);
	for(;times;times--) {
		if (!config.unksize)
		if (config.seek > config.size) {
			config.seek = config.size;
			break;
		}
		config.seek += config.block_size - (config.seek % config.block_size);
	}
	undo_push();

	return 0;
}

print_fmt_t last_search_fmt = FMT_ASC;

// TODO: show /k && /m together
CMD_DECL(search) {
	char buf[128];
	char *input2 = (char *)strdup(input);
	char *text   = input2;
	int  len,i = 0,j = 0;
	ut64 seek = config.seek;
	char *ptr;

	switch(text[0]) {
	case '\0':
	case '?':
		cons_printf(
		" / \\x7FELF       ; plain string search (supports \\x).\n"
		" /i str-no-case  ; case insensitive text search\n"
		" /. [file]       ; search using the token file rules (/.? for help)\n"
		" /: [file] [...] ; search using the 'keyname keyword' file format\n"
		" /-              ; unsetenv SEARCH[N] and MASK[N] environ vars( deprecated )\n"
		" //              ; repeat last search\n"
		" /a [opcode]     ; look for a string in disasembly\n"
		" /A              ; find expanded AES keys from current seek(*)\n"
		" /k# keyword     ; keyword # to search\n"
		" /m# FF 0F       ; Binary mask for search '#' (optional)\n"
		" /n[-]           ; seek to hit index N (/n : next, /n- : prev)\n"
		" /r 0,2-10       ; launch range searches 0-10\n"
		" /s [str] [str]  ; replace first string with the second one\n"
		" /S [hex] [hex]  ; replace first hexpair string with the second one\n"
		" /p len          ; search pattern of length = len\n"
		" /P count        ; search patterns of size $$b matching at least N bytes of curblock\n"
		" /v numexpr      ; search a value (32 or 64 bit size) uses cfg.bigendian\n"
		" /w foobar       ; search a widechar string (f\\0o\\0o\\0b\\0..)\n"
		" /x A0 B0 43        ; hex byte pair binary search. (space between bytes are optional)\n"
		" /z [str,-max,+min] ; find ascii/widechar strings matching 'str' (or size limit)\n"
		" /0, /1, /2..       ; launch search using keyword number\n"
		"NOTE: See 'e search.' for extended options for these commands\n"
		"NOTE: This command will run from current seek unless we had previosly defined\n"
		"      The initial and end offsets in the search.from and search.to eval vars.\n");
		break;
	case 'e':
		//regexp
		eprintf("TODO\n");
		break;
	case 'P':
		if (text[1]=='?') {
			eprintf("Usage: /P [num]\n"
			" Searchs for a block of blocksize bytes matching at least"
			" 'num' bytes of the blocksize bytes\n");
		} else {
			search_similar_pattern(atoi(text+1));
			radare_seek(seek, SEEK_SET);
		}
		break;
	case 'p':
		do_byte_pat(atoi(text+1));
		radare_seek(seek, SEEK_SET);
		break;
	case 'm':
		if (text[1]=='?') {
			eprintf("/m [binary-search-mask]\n");
			break;
		}
		i = atoi(text+1);
		ptr = strchr(text, ' ');
		if (ptr) {
			sprintf(buf, "MASK[%d]", i);
			setenv(buf, ptr+1, 1);
		} else {
			extern char **environ;
			for(i=0;environ[i];i++) {
				if (!memcmp(environ[i], "MASK[", 5)) {
					int j = atoi(environ[i]+5);
					sprintf(buf, "MASK[%d]", j);
					ptr = getenv(buf);
					if (ptr)
						cons_printf("%d %s\n", j, ptr);
					else    cons_printf("%d (no mask)\n", i);
				}
			}
		}
		break;
	case 'n':
		switch(text[1]) {
		case '\0':
			radare_search_seek_hit(+1);
			break;
		case '+':
		case '-':
			i = atoi(text+1);
			if (i ==0)
				i = (text[1]=='+')?1:-1;
			radare_search_seek_hit(i);
			break;
		case '?':
			eprintf("/n[+,-,#]      ; seek to search hit next, previous or number N\n");
			break;
		default:
			radare_search_seek_hit(atoi(text+1));
			break;
		}
		break;
	case 'k':
		if (text[1]=='?') {
			eprintf("/k[number] [keyword]\n");
			break;
		}
		i = atoi(text+1);
		ptr = strchr(text, ' ');
		if (ptr) {
			sprintf(buf, "SEARCH[%d]", i);
			setenv(buf, ptr+1, 1);
		} else {
			for(i=0;environ[i];i++) {
				if (!memcmp(environ[i], "SEARCH[", 7)) {
					int j = atoi(environ[i]+7);
					sprintf(buf, "SEARCH[%d]", j);
					ptr = getenv(buf);
					if (ptr) cons_printf("%02d %s\n", j, ptr);
					else cons_printf("%02d (no keyword)\n", i);
				}
			}
		}
		break;
	case 'a':
		radare_search_asm(text+2);
		break;
	case 'A':
		radare_search_aes();
		break;
	case 'b':
		eprintf("Sorry. This is not 4chan =)\n");
		break;
	case 's':
		radare_search_replace(text+2, 0);
		break;
	case 'S':
		radare_search_replace(text+2, 1);
		break;
	case 'z':
		radare_strsearch(strchr(text, ' '));
		break;
	case 'r':
		search_range(text+2);
		break;
	case 'w':
		free(input2);
		len = strlen(input+2);
		input2 = (char *)malloc(len*6);
		for(i=0;i<len;i++,j+=5) {
			input2[j]=input[i+2];
			input2[j+1]='\0';
			strcat(input2,"\\x00");
		}
		radare_cmd("f -hit0*", 0);
		setenv("SEARCH[0]", input2, 1);
		search_range("0");
		break;
	case '/':
		search_range("0");
		break;
	case 'i':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case ' ': {
		char buf[128];
		char *ptr = strchr(text, ' ');
		int idx = atoi(text);
		sprintf(buf, "SEARCH[%d]", idx);
		if (ptr == NULL)
			ptr = text+1;
		else ptr = ptr +1;
		setenv(buf, ptr, 1);
		sprintf(buf, "%d", idx);
		if (text[0]=='i')
			search_nocase = 1;
		else search_nocase = 0;
		search_range(buf);
		} break;
	case 'v':
		{
		char buf[64];
		ut8 *b;
		ut64 n, _n;
		ut32 n32, _n32;
		radare_cmd("f -hit0*", 0);
		_n = n = get_math(input+2);
		if (n & 0xffffffff00000000LL) {
			endian_memcpy_e((ut8*)&n, (const ut8*)&_n, 8, !config_get_i("cfg.bigendian"));
			b=(ut8*)&n;
			sprintf(buf,
			"\\x%02x\\x%02x\\x%02x\\x%02x"
			"\\x%02x\\x%02x\\x%02x\\x%02x",
			b[0],b[1],b[2],b[3],
			b[4],b[5],b[6],b[7]);
		} else {
			_n32 = n32=(ut32)n;
			endian_memcpy_e((ut8*)&n32, (const ut8*)&_n32, 4, !config_get_i("cfg.bigendian"));
			b=(ut8*)&n32;
			sprintf(buf,
			"\\x%02x\\x%02x\\x%02x\\x%02x",
			b[0],b[1],b[2],b[3]);
		}
		setenv("SEARCH[0]", buf, 1);
		printf("Searching: '%s' (%s)=0x%llx\n", buf, input2, n);
		search_range("0");
		break;
		}
	case '-':
		for(i=0;environ[i];i++) {
			char *eq = strchr(environ[i], '=');
			if (!eq) continue;
			eq[0]='\0';
			if ((strstr(environ[i], "SEARCH"))
			||  (strstr(environ[i], "MASK")))
				unsetenv(environ[i]);
			else	eq[0]='=';
		} break;
	case ':':
		for(text=text+1;*text==' ';text++);
		search_from_simple_file(text);
		break;
	case '.':
		for(text=text+1;*text==' ';text++);
		search_from_file(text);
		break;
	case 'x':
		free(input2);
		len = strlen(input+2);
		if (len<2) {
			eprintf("Too short len\n");
			return;
		}
		input2 = (char *)malloc((len<<1)+10);
		input2[0] = '\0';
		for(i=0;i<len;i+=j) {
			char buf[10];
			int ch;
			ch = hexpair2bin(input+2+i);
			if (i+4<len && input[2+i+2]==' ')
				j = 3;
			else j = 2;
			if (ch == -1) {
				eprintf ("Invalid hexadecimal string\n");
				goto __beach;
			}
			if (is_printable(ch)) {
				buf[0] = ch;
				buf[1] = '\0';
				strcat(input2, buf);
			} else {
				sprintf(buf, "\\x%02x", ch);
				strcat(input2, buf);
			}
		}
		radare_cmd("f -hit0*", 0);
		setenv("SEARCH[0]", input2, 1);
		search_range("0");
		break;
	default:
		setenv("SEARCH[0]", text, 1);
		search_range("0");
		break;
	}
__beach:
	free(input2);

	return 0;
}

CMD_DECL(shell)
{
	int ret = 0;
	env_prepare(input);
	if (input[0]=='!')
		ret = radare_system(input+1);
	else ret = io_system(input);
	env_destroy(input);
	return ret;
}

CMD_DECL(rtr)
{
	switch (input[0]) {
	case '\0':
		rtr_list();
		break;
	case '?':
		rtr_help();
		break;
	case '+':
		rtr_add(input+1);
		break;
	case '-':
		rtr_remove(input+1);
		break;
	case '=':
		rtr_session(input+1);
		break;
	case '<':
		rtr_pushout(input+1);
		break;
	default:
		rtr_cmd(input);
	}
	return 0;
}

CMD_DECL(help)
{
	if (strlen(input)>0) {
		// XXX switch/case this!
		switch(input[0]) {
		case '!':
			if (config.last_cmp != 0)
				radare_cmd(input+1, 0);
			break;
		case 's':
			{
			ut64 from, to, step;
			char *p, *p2;
			for(input=input+1;input[0]==' ';input=input+1);
			p = strchr(input, ' ');
			if (p) {
				*p='\0';
				from = get_math(input);
				p2 = strchr(p+1, ' ');
				if (p2) {
					*p2='\0';
					step = get_math(p2+1);
				} else step = 1;
				to = get_math(p+1);
				for(;from<=to;from+=step)
					cons_printf("%lld ", from);
				cons_newline();
			}
			}
			break;
		case 'q':
			config.last_cmp = get_math(input+1);
			break;
		case 'v':
			{
			ut64 res = get_math(input+1);
			config.last_cmp = res;
			cons_printf("0x%08llx\n", res);
			}
			break;
		case '?':
			if (input[1]=='?') {
				cons_printf("0x%llx\n", config.last_cmp);
			} else
			if (input[1]=='\0') {
				cons_strcat(
				"Usage: ?[?[?]] <expr>\n"
				" ? eip             ; get value of eip flag (or any math expression)\n"
				" ?i offset A:      ; user input for ut64 numeric values, result in $$?\n"
				" ?f 3.4545         ; calculate hex, double and float value for hex or float num\n"
				" ?v eip+33+[esp]   ; show hex value of math expression\n"
				" ?q eip+33+[esp]   ; quite math expression evaluation (no output) changes $$?\n"
				" ?z`str            ; sets false if string is zero length\n"
				" ?x 303132         ; show hexpair as a printable string\n"
				" ?x 0x804800       ; show hexpair ruled by asm.bits and cfg.bigendian\n"
				" ?s from to step   ; print numeric sequence for(;from<=to;from+=step)\n"
				" ?X eip            ; show hex result of math expression\n"
				" ? 0x80+44         ; calc math expression\n"
				" ? eip-23          ; ops with flags and numbers\n"
				" ? eip==sym.main   ; compare flags\n"
				"The ?? is used for conditional executions after a comparision\n"
				" ? [foo] == 0x44   ; compare memory read with byte\n"
				" ? eip != oeip     ; compare memory read with byte\n"
				" ???               ; show result of comparision\n"
				" ?? s +3           ; seek current seek + 3 if equal\n"
				" ?! s +3           ; seek current seek + 3 if not equal\n"
				"Special $$ variables that can be used in expressions\n"
				" $$  = current seek\n"
				" $$$ = size of opcode\n"
				" $$b = block size\n"
				" $$s = file size\n"
				" $$? = last compare (? 1==1) or read (?<) value\n"
				" $$j = jump branch of opcode\n"
				" $$f = failover continuation of opcode\n"
				" $$r = pointer reference of opcode\n"
				" $$e = end of basic block\n"
				" $$F = beggining of the function\n"
				" $$S = size of the function\n"
				" $$l = last seek done\n"
				" $$w = size of last write operation\n"
				" $${io.vaddr} = get eval value\n"
				"Note: Prefix the command with '\"' to skip pipes ('|','>'..)\n"
				);
			} else
			if (config.last_cmp == 0)
				radare_cmd(input+1, 0);
			break;
		case '<':
			{
			char buf[1024];
			cons_printf("%s?\n", input+1);
			cons_flush();
			cons_fgets(buf, 1023, 0, NULL);
			if (buf[0]=='\0') {
				config.last_cmp = 1; // true
			} else
			if (!strcmp(buf, "y")) {
				config.last_cmp = 1;
			} else
			if (!strcmp(buf, "n")) {
				config.last_cmp = 0;
			} else config.last_cmp = get_math(buf);
			}
			break;
		case 't':
			{
			struct r_prof_t prof;
			r_prof_start(&prof);
			radare_cmd(input+1, 0);
			r_prof_end(&prof);
			config.last_cmp = (ut64)prof.result;
			eprintf("%lf\n", prof.result);
			}
			break;
		case 'e':
			{
			char *str = input+1;
			if (str[0]==' ')
				str = str+1;
			if (str[0]) {
				cons_strcat(str);
				cons_newline();
			} }
			break;
		case 'f':
			{
			//double n1, n2;
			/* TODO: Add support for doubles 'F'? */
			char *str = input+((input[1]==' ')?2:1);
			char buf[64];
			ut32 _ut32, __ut32;
			float _f;
			ut8 *p = (ut8*) &_f;
			if (str[0]=='0'&&str[1]=='x') {
				sscanf(str, "0x%x", &_ut32);
				memcpy(&_f, &_ut32, sizeof(float));
				sprintf(buf, "%f", _f);
				/* consistency check */
				//n1 = (double)_f;
				//sscanf(buf, "%g", (double *)&n2);
			} else {
				sscanf(str, "%f", &_f);
				memcpy(&_ut32, &_f, sizeof(ut32));
				sprintf(buf, "0x%x", _ut32);
				/* consistency check */
				//sscanf(str, "%lf", &n1);
				//n2 = (double)_f;
			}
			cons_printf("%s\n", buf);
			/* it is consistent ? */
			//if (n1 != n2) cons_printf("%s,\n", buf);
			//else cons_printf("%s\n", buf);
			}
			break;
		case 'i':
			{
			char str[128];
			eprintf("%s: ", input+2);
			fflush(stderr);
			fgets(str, 127, stdin);
			config.last_cmp = get_math(str);
			}
			break;
		case 'z':
			{
			char *ptr;
			for(ptr=input+1;*ptr==' ';ptr=ptr+1);
			config.last_cmp = strlen(ptr);
			}
			break;
		case 'x':
			if (input[2]=='0' && input[3]=='x') {
	// TODO: endian memcpy!!
				switch(config_get_i("asm.bits")) {
				case 64:
					{
					ut64 n = get_math(input+2);
					u8 np[8];
					endian_memcpy_e((u8*)&np, (u8*)&n, 8, !config.endian);
					cons_printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",
						np[0], np[1], np[2], np[3], np[4], np[5], np[6], np[7]);
					}
					break;
				case 32:
					{ ut32 n = get_math(input+2);
					u8 np[4];
					endian_memcpy_e((u8*)&np, (u8*)&n, 4, !config.endian);
					cons_printf("%02x %02x %02x %02x\n",
						np[0], np[1], np[2], np[3]);
					}
					break;
				case 16:
					{ ut16 n = get_math(input+2);
					u8 np[2];
					endian_memcpy_e((u8*)&np, (u8*)&n, 2, !config.endian);
					cons_printf("%02x %02x\n", np[0], np[1]);
					}
					break;
				}
			} else {
				char buf[1024];
				int len;
				len = hexstr2binstr(input+1, buf);
				if (len>0)
					print_data(0, "", buf, len, FMT_ASC);
			}
			break;
		case 'X':
			{
			ut64 res = get_math(input+1);
			if (res > 0xffffffff)
				cons_printf("0x%llx\n", -res);
			else cons_printf("0x%llx\n", res);
			}
			break;
		default:
			{
			ut64 res = get_math(input);
			config.last_cmp = res;
		//	if (!strchr(input, '!') && !strchr(input, '=')) {
			D { 
				char str[64];
				char ch=' ';
				double resf = res;
				cons_printf("0x%llx %lldd %lloo ", res, res, res); 
				PRINT_BIN(res); 
				if (res>1024*1024*1024) {
					resf = res/(1024*1024*1024);
					ch='G';
				} else
				if (res>1024*1024) {
					resf = res/(1024*1024);
					ch='M';
				} else
				if (res>1024) {
					resf = res/(1024);
					ch='K';
				}
				sprintf(str, "%.1f%c", resf, ch);
				cons_printf(" %s", str);
				cons_newline();
			}// else cons_printf("0x%llx\n", res);
			//}
			}
		}
	} else show_help_message();
	return 0;
}

CMD_DECL(default)
{
	D eprintf("Invalid command '%s'. Try '?'.\n", input);
	return 0;
}
