/*
 * Copyright (C) 2006, 2007, 2008
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

print_fmt_t last_print_format = FMT_HEXB;
int fixed_width = 0;
extern char **environ;
extern command_t commands[];
extern void show_help_message();

/* Call a command from the command table */
int commands_parse (const char *_cmdline)
{
	char *cmdline = (char *)_cmdline;
	command_t *cmd;

	for(;cmdline[0]&&(cmdline[0]=='\x1b'||cmdline[0]==' ');
	cmdline=cmdline+1);

	// null string or comment
	if (cmdline[0]==0||cmdline[0]==';')
		return 0;

	if (cmdline[0]=='h') {
		(void)show_help_message();
		return 0;
	}

	for (cmd = commands; cmd->sname != 0; cmd++)
		if (cmd->sname == cmdline[0])
		    return cmd->hook(cmdline+1);

	/* default hook */
	return cmd->hook(cmdline);
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
		case '+': case '-': case '0': case '=': case '?':
		    continue;
		}
		cmdaux++;
		cmdaux += sprintf(cmdaux, "%s", cmd->options);
		cmdaux[0] = '\0';
		cons_printf(" %-17s %s\n", cmdstr, cmd->help);
	}
	cons_printf(" ? <expr>          calc     math expr and show result in hex,oct,dec,bin\n");
}

	//COMMAND('c', " [times]",       "count   limit of search hits and 'w'rite loops", count),
	//COMMAND('e', " [0|1]",       "endian  change endian mode (0=little, 1=big)", endianess),
	//COMMAND('g', " [x,y]",         "gotoxy  move screen cursor to x,y", gotoxy),
	//COMMAND('l', " [offset]",      "limit   set the top limit for searches", limit),
	//COMMAND('Y', " [length]",      "Ypaste  copy n bytes from clipboard to cursor", yank_paste),
	//COMMAND('_', " [oneliner]",    "perl/py interpret a perl/python script (use #!perl)", interpret_perl),
	//COMMAND('%', "ENVVAR [value]", "setenv  gets or sets a environment variable", envvar),

command_t commands[] = {
	COMMAND('a', "[ocdgv?]",       "analyze  perform code/data analysis", analyze),
	COMMAND('b', " [blocksize]",   "bsize    change the block size", blocksize),
	COMMAND('c', "[f file]|[hex]", "cmp      compare block with given data", compare),
	COMMAND('C', "[op] [arg]",     "Code     Comments, data type conversions, ..", code),
	COMMAND('e', "[m?] key=value", "eval     evaluates a configuration expression", config_eval),
	COMMAND('f', "[crogd|-][name]","flag     flag the current offset (f? for help)", flag),
	COMMAND('H', " [cmd]",         "Hack     performs a hack", hack),
	COMMAND('i', "",               "info     prints status information", info),
	COMMAND('m', " [size] [dst]",  "move     copy size bytes from here to dst", move),
	COMMAND('o', " [file]",        "open     open file", open),
	COMMAND('p', "[fmt] [len]",    "print    print data block", print),
	COMMAND('q', "[!]",            "quit     close radare shell", quit),
	COMMAND('P', "[so][i [file]]", "Project  project Open, Save, Info", project),
	COMMAND('r', " [size|-strip]", "resize   resize or query the file size", resize),
	COMMAND('R', "[act] ([arg])",  "RDB      rdb operations", rdb),
	COMMAND('s', " [[+,-]pos]",    "seek     seek to absolute/relative expression", seek),
	COMMAND('u', "[[+,-]idx]",     "undo     undo/redo indexed write change", undowrite),
	COMMAND('V', "",               "Visual   enter visual mode", visual),
	COMMAND('w', "[?aAdwxfF] [str]","write    write ascii/hexpair string here", write),
	COMMAND('x', " [length]",      "examine  the same as p/x", examine),
	COMMAND('y', "[y] [length]",   "yank     copy n bytes from cursor ('yy' to paste)", yank),
	COMMAND('.', "[!cmd]|[ file]", ".script  interpret a script (radare, .py, .lua, .pl)", interpret),
	COMMAND('-', "[size]",         "prev     go to previous block (-= block_size)", prev),
	COMMAND('+', "[size]",         "next     go to next block (+= block_size)", next),
	COMMAND('<', "",               "preva    go previous aligned block", prev_align),
	COMMAND('>', "",               "nexta    go next aligned block", next_align),
	COMMAND('/', "[?] [str]",      "search   find matching strings", search),
	COMMAND('!', "[[!]command]",   "system   execute a !iosystem or !!shell command", shell), 
	COMMAND('#', "[hash|!lang]",   "hash     hash current block (#? or #!perl)", hash),
	COMMAND('?', "",               "help     show the help message", help),
	COMMAND( 0, NULL, NULL, default)
};

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
		cons_printf("Usage: e[m] key=value\n");
		cons_printf("   > ereset           ; reset configuration\n");
		cons_printf("   > emenu            ; opens menu for eval\n");
		cons_printf("   > e scr.color = 1  ; sets color for terminal\n");
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
	int i, j, sz, n_calls=0;
	int depth_i;
	int delta = 0;
	int depth = input[0]?atoi(input+1):0;
	u64 oseek = config.seek;

	if (depth<1)depth=1;
	//eprintf("depth = %d\n", depth);
	memset(&aop, '\0', sizeof(struct aop_t));

	switch(input[0]) {
	case 't':
		switch(input[1]) {
		case '?':
			cons_printf("Usage: at[*] [addr]\n");
			cons_printf("   > at         ; list all traced opcode ranges\n");
			cons_printf("   > at-        ; reset the tracing information\n");
			cons_printf("   > at*        ; list all traced opcode offsets\n");
			cons_printf("   > at [addr]  ; show trace info at address\n");
			break;
		case '-':
			trace_reset();
			break;
		case ' ': {
			u64 foo = get_math(input+1);
			struct trace_t *t = trace_get(foo);
			if (t != NULL) {
				cons_printf("offset = 0x%llx\n", t->addr);
				cons_printf("opsize = %d\n", t->opsize);
				cons_printf("times = %d\n", t->times);
				cons_printf("count = %d\n", t->count);
				//TODO cons_printf("time = %d\n", t->tm);
			} } break;
		case '*':
			trace_show(1);
			break;
		default:
			trace_show(0);
		}
		break;
	case 'd':
		// XXX do not uses depth...ignore analdepth?
		radare_analyze(config.seek, config.block_size, config_get_i("cfg.analdepth"));
		break;
#if 0
	/* TODO: reset analyze engine ? - remove rdbs, xrefs, etc...  reset level as argument? maybe cool 0 for just vm, 1 for rdbs, 2 for xrefs */
	case 'r':
		break;
#endif
	case 'b':
		prg = code_analyze(config.seek, depth);
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
					cons_printf("%02x ", b0->bytes[i]);
				cons_newline();
				cons_newline();
			}
		} else {
			eprintf("oops\n");
		}
		break;
	case 'f': {
		u64 end = 0;
		/* Analyze function */
		/* XXX ensure this is ok */
		config_set("graph.jmpblocks", "true");
		config_set("graph.callblocks", "false");
		prg = code_analyze(config.baddr + config.seek, 1024);
		list_add_tail(&prg->list, &config.rdbs);
		list_for_each(head, &(prg->blocks)) {
			b0 = list_entry(head, struct block_t, list);
			//if ((b0->type == BLK_TYPE_HEAD)
			//if ((b0->type == BLK_TYPE_LAST)
			//|| (b0->type == BLK_TYPE_FOOT))
			if (b0->addr +b0->n_bytes > end)
				end = b0->addr + b0->n_bytes;
		}
		{
			char *bytes;
			struct aop_t aop;
			u64 from = config.baddr+ config.seek;
			u64 seek = from; // to place comments
			u64 to   = end+1;
			u64 len  = to-from;
			int inc  = 0;

			bytes = (char *)malloc(len);
			radare_read_at(from, bytes, len);

			cons_printf("; from = 0x%08llx\n", from);
			cons_printf("; to   = 0x%08llx\n", end);
			cons_printf("CF %lld @ 0x%08llx\n", len, from); // XXX can be recursive
			for(;seek< to; seek+=inc) {
				inc = arch_aop(seek, bytes+(seek-from), &aop);
				if (inc<1) {
					inc = 1;
					continue;
				}
				switch(aop.stackop) {
				case AOP_STACK_LOCAL_SET:
					{
					char buf[1024];
					int ref = (int)aop.ref;
					if (ref<0)
						sprintf(buf, "CC Set arg%d @ 0x%08llx\n", -ref, seek);
					else sprintf(buf, "CC Set var%d @ 0x%08llx\n", ref, seek);
					cons_strcat(buf);
#if 0
					radare_cmd(buf, 0);
					radare_seek(config.seek, SEEK_SET);
					radare_read(0);
#endif
					}
					break;
				case AOP_STACK_ARG_SET:
					{
					int ref = (int)aop.ref;
					char buf[1024];
					sprintf(buf, "CC Set arg%d @ 0x%08llx\n", ref, seek);
					cons_strcat(buf);
#if 0
					radare_cmd(buf, 0);
					radare_seek(config.seek, SEEK_SET);
					radare_read(0);
#endif
					}
					break;
				case AOP_STACK_ARG_GET:
					{
					char buf[1024];
					int ref = (int)aop.ref;
					sprintf(buf, "CC Get arg%d @ 0x%08llx\n", ref, seek);
					cons_strcat(buf);
#if 0
					radare_cmd(buf, 0);
					radare_seek(config.seek, SEEK_SET);
					radare_read(0);
#endif
					}
					break;
				case AOP_STACK_LOCAL_GET:
					{
					char buf[1024];
					int ref = (int)aop.ref;
					if (ref<0)
						sprintf(buf, "CC Get arg%d @ 0x%08llx\n", -ref, seek);
					else sprintf(buf, "CC Get var%d @ 0x%08llx\n", ref, seek);
					cons_strcat(buf);
#if 0
					radare_cmd(buf, 0);
					radare_seek(config.seek, SEEK_SET);
					radare_read(0);
#endif
					}
					break;
				case AOP_STACK_INCSTACK:
					{
					char buf[1024];
					sprintf(buf, "CC Stack size +%d @ 0x%08llx\n", (int)aop.ref, seek);
					cons_strcat(buf);
#if 0
					radare_cmd(buf, 0);
					radare_seek(config.seek, SEEK_SET); radare_read(0);
#endif
					}
					break;
				}
			}
			free(bytes);
			/* add final report here */
			/* N local vars...*/
		}}
		break;
	case 'g':
#if HAVE_VALAC
		// use graph.depth by default if not set
		prg = code_analyze(config.baddr + config.seek, depth ); //config_get_i("graph.depth"));
		list_add_tail(&prg->list, &config.rdbs);
		grava_program_graph(prg, NULL);
#else
		eprintf("Compiled without valac/gtk/cairo\n");
#endif
		break;
	case 'c': {
		int c = config.verbose;
		char cmd[1024];
		config.verbose = 1;
		prg = code_analyze(config.seek+config.baddr, depth); //config_get_i("graph.depth"));
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
	case 'o':
		udis_init();
		for(depth_i=0;depth_i<depth;depth_i++) {
			radare_read(0);
			sz = arch_aop(config.baddr + config.seek, config.block, &aop);
			cons_printf("index = %d\n", depth_i);
#if 0
			cons_printf("opcode = ");
			j = config.verbose;
			config.verbose = 0;
			radare_cmd("pd 1", 0);
			config.verbose = j;
#endif
			cons_printf("size = %d\n", sz);
			cons_printf("type = ");
			// TODO: implement aop-type-to-string
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
			default: cons_printf("unknown(%d)\n", aop.type);
			}
			cons_printf("bytes = ");
			for (i=0;i<sz;i++) cons_printf("%02x ", config.block[i]);
			cons_printf("\n");
			cons_printf("offset = 0x%08llx\n", config.baddr+config.seek);
			cons_printf("ref = 0x%08llx\n", aop.ref);
			cons_printf("jump = 0x%08llx\n", aop.jump);
			cons_printf("fail = 0x%08llx\n", aop.fail);
			cons_newline();
			delta += sz;
			config.seek += sz;
		}
		break;
	case 'v':
		eprintf("WIP: av . analyze N opcodes using code emulation\n");
		vm_emulate(depth);
		break;
	case 's':
		analyze_spcc(input+1);
		break;
	case 'x': {
		char buf[4096];
		char file[1024];
		u64 seek = config.seek;
		u64 base = 0;
		strcpy(file, config.file);
#if DEBUGGER
		if (config.debug) {
			/* dump current section */
			if (radare_dump_section(file))
				return 1;
			base = 0x8048000; // XXX must be section base
		}
#endif
		snprintf(buf, 4095, "%s -a %s -b %lld %s %lld",
			config_get("asm.xrefs"), config_get("asm.arch"), base, file, seek);
		eprintf("system(%s)\n", buf);
		radare_system(buf);
#if DEBUGGER
		if (config.debug) {
			unlink(file);
		}
#endif
		} break;
	default:
		cons_printf("Usage: a[ocdg] [depth]\n");
		cons_printf(" ao [nops]    analyze N opcodes\n");
		cons_printf(" ab [num]     analyze N code blocks\n");
		cons_printf(" af [size]    analyze function\n");
		cons_printf(" aF [size]    analyze function (recursively)\n");
		cons_printf(" ac [num]     disasm and analyze N code blocks\n");
		cons_printf(" ad [num]     analyze N data blocks \n");
		cons_printf(" ag [depth]   graph analyzed code\n");
		cons_printf(" as [name]    analyze spcc structure (uses dir.spcc)\n");
		cons_printf(" at [args]    analyze opcode traces\n");
		cons_printf(" av [nops]    analyze virtual machine (negative resets before)\n");
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
	switch(input[0]) {
	case 'o':
		if (input[1])
			project_open(arg);
		else{
			str = strdup ( config_get("file.project") );
			project_open(str);
			free ( str );
		}
		break;
	case 's':
		if (input[1])
			project_save(arg);
		else {
			str = strdup ( config_get("file.project") );
			project_save(str);
			free ( str );
		}
		break;
	case 'i':
		if (input[1]) {
			project_info(arg);
		} else {
			str = strdup ( config_get("file.project"));
			project_info(str);
			free ( str );
		}
		break;
	default:
		cons_printf(
		" Po [file]  open project\n"
		" Ps [file]  save project\n"
		" Pi [file]  info\n");
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
		} else {
			radare_cmd("e | sort | cut -d . -f 1 | uniq | awk '{print \" - \"$1}'", 0);
		}
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
			if (!pfx[0]) {
				strcpy(pfx, buf);
			} else {
				cons_printf("%s.%s = ", pfx, buf);
				sprintf(buf2, "e %s.%s", pfx, buf);
				radare_cmd(buf2, 0);
				goto noprint;
			}
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

CMD_DECL(rdb)
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
	case 'm':
		break;
	case 'g':
		break;
	case 'd':
		break;
	}

	/* list */
	if (text[0]=='\0') {
		int i = 0;
		struct list_head *pos;
		list_for_each(pos, &config.rdbs) {
			struct program_t *mr = list_entry(pos, struct program_t, list);
			fflush(stdout);
			cons_printf("%d 0x%08x %s\n", i, (unsigned long long)mr->entry, mr->name);
			i++;
		}
		return 0;
	}
	/* remove */
	if (text[0]=='-') {
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
	/* open */
	if (input[0] ==' ') {
		struct program_t *prg = program_open(text+1); // XXX FIX stripstring and so
		list_add_tail(&prg->list, &config.rdbs);
		return 0;
	}
	/* grefusa diffing */
	if (input[0] =='d') {
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
	/* draw graph */
	if (input[0] =='G') {
#if HAVE_VALAC
		int num = atoi(text);
		int i = 0;
		struct list_head *pos;
		list_for_each(pos, &config.rdbs) {
			struct program_t *mr = list_entry(pos, struct program_t, list);
			if (i ==  num) {
				grava_program_graph(mr);
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
#if 0

	ret = flag_set(text, config.seek, input[0]=='n');
	D if (ret) {
		if (!config.debug)
			eprintf("flag '%s' redefined to "OFF_FMTs"\n", text, config.seek);
	} else {
		flags_setenv();
		eprintf("flag '%s' at "OFF_FMT" and size %d\n",
			text, config.seek, config.block_size);
	}
#endif
	return 0;
}

CMD_DECL(baddr)
{
	int i;
	for(i=0;input[i]&&input[i]!=' ';i++);
	if (input[i]!='\0')
		config.baddr = get_math(input+i+1);
	D { printf("0x"OFF_FMTx, config.baddr); NEWLINE; }

	return 0;
}

CMD_DECL(hash)
{
	int i;
	char buf[1024];
	char *str;
	u64 bs = config.block_size;
	u64 obs = config.block_size;

	str = strchr(input, ' ');
	if (str)
		bs = get_math(str+1);

	for(i=0;input[i];i++) if (input[i]==' ') { input[i]='\0'; break; }

	/* #!perl hashbang! */
	if (input[0]=='!') {
#if HAVE_PERL
		if (strstr(input+1, "perl"))
			config.lang = LANG_PERL;
		else
#endif
#if HAVE_PYTHON
		if (strstr(input+1, "python"))
			config.lang = LANG_PYTHON;
		else
#endif
			eprintf("Invalid interpreter. Try (perl or python).\n");
		// TODO: check perl|python build and show proper msg
		return 0;
	}

	// XXX doesnt works with dbg:///
	// XXX use real temporal file instead of /tmp/xx
	if (config.debug) {
		radare_set_block_size_i(bs);
		radare_cmd("pr > /tmp/xx", 0);
		snprintf(buf, 1000, "rahash -fa '%s' '/tmp/xx'", input);
		radare_set_block_size_i(obs);
	} else
	snprintf(buf, 1000, "rahash -a '%s' -S %lld -L %lld '%s'", // | head -n 1", 
		input, (u64)config.seek, (u64)bs, config.file);

	io_system(buf);

	if (config.debug)
		unlink("/tmp/xx");

	return 0;
}

CMD_DECL(interpret_perl)
{
	#if HAVE_PERL
	char *ptr;
	char *cmd[] = { "perl", "-e", ptr, 0 };
	#endif

	if (input==NULL || *input== '\0' || strchr(input,'?')) {
	#if HAVE_PERL
		eprintf("Usage: > #!perl\n");
		eprintf("       > _print \"hello world\";\n");
		eprintf("       > _require \"my-camel.pl\";\n");
	#endif
	#if HAVE_PYTHON
		eprintf("Usage: > #!python\n");
		eprintf("       > _print \"hello world\\n\"\n");
		eprintf("       > _import my-snake.py\n");
	#endif
	#if !HAVE_PERL && !HAVE_PYTHON
		eprintf("Build with no scripting support.\n");
	#endif
		return 0;
	}

	switch(config.lang) {
	#if HAVE_PERL
	case LANG_PERL:
		ptr = strdup(input);
		cmd[2] = ptr;
		eperl_init();
		perl_parse(my_perl, xs_init, 3, cmd, (char **)NULL);
		perl_run(my_perl);
		eperl_destroy();
		free(ptr);
		break;
	#endif
	#if HAVE_PYTHON
	case LANG_PYTHON:
		epython_init();
		epython_eval(input);
		epython_destroy(); // really??? why not keep the dream on?
		break;
	#endif
	default:
		eprintf("No scripting support.\n");
		break;
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
	char *ptr = strdup(input);
	clear_string(ptr);
	config.file = ptr;
	radare_open(0);
	//radare_go();
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
		prepare_environment("");
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
		} else 	{ NEWLINE; }
		return 0;
	}

	/* Set variable value */
	ptro = ptr = text;
	ptr[0]='\0';
	for(;*text&&iswhitespace(*text);text=text+1);
	for(ptr=ptr+1;*ptr&&iswhitespace(*ptr);ptr=ptr+1);

	if ((!memcmp(ptr,"-", 2))
	|| (!memcmp(ptr,"(null)", 5))) {
		unsetenv(text2);
		//D cons_printf("%%%s=(null)\n", text2);
	} else {
		setenv(text2, ptr, 1);
		//D cons_printf("%%%s='%s'\n", text2, ptr);
	}
	ptro[0]=' ';

	update_environment();
	free(text2);

	return 0;
}

CMD_DECL(blocksize)
{
	flag_t *flag;
	switch(input[0]) {
	case 'f': // bf = block flag size
		flag = flag_get(input+2);
		if (flag) {
			radare_set_block_size_i(flag->length);
			printf("block size = %d\n", flag->length);
		} else {
			eprintf("Unknown flag '%s'\n", input+2);
		}
		break;
	case '?':
		cons_printf("Usage: b[f flag]|[size]     ; Change block size\n");
		cons_printf("  > b 200 ; set block size to 200\n");
		cons_printf("  > bf sym_main && s sym_main\n");
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
	case 'C':
		/* comment */
		text = text+1;
		while(text[0]==' ')
			text = text + 1;
		cons_flush();
		if (text[0]=='\0'  || text[0]=='?')
			cons_printf("Usage: CC [-addr|comment] @ address\n"
				"adds or removes comments for disassembly\n");
		else
		if (text[0]) {
			if (text[0]=='-')
				metadata_comment_del(config.seek, text+1);
			else	metadata_comment_add(config.seek, text);
		}
		break;
	case 'x': // code xref
		if (text[1]=='-')
			metadata_xrefs_del(config.seek, get_math(text+2), 0);
		else	metadata_xrefs_add(config.seek, get_math(text+1), 0);
		break;
	case 'X': // data xref
		if (text[1]=='-')
			metadata_xrefs_del(config.seek, get_math(text+2), 1);
		else	metadata_xrefs_add(config.seek, get_math(text+1), 1);
		break;
	case 'F':
		/* do code analysis here */
	case 'c':
	case 'd':
	case 's':
	case 'f':
	case 'u': {
		u64 tmp = config.block_size;
		int fmt = FMT_HEXB;
		int len = get_math(text +2);
		switch(text[0]) {
			case 'c': fmt = DATA_CODE; break;
			case 'd': fmt = DATA_HEX; break;
			case 's': fmt = DATA_STR; break;
			case 'F': fmt = DATA_FUN; break;
			case 'f': fmt = DATA_FOLD_C; break;
			case 'u': fmt = DATA_FOLD_O; break;
		}
		//if (len>config.block_size)
		//	len = config.block_size;
		tmp = config.block_size;
		radare_set_block_size_i(len);
		data_add(config.seek+(config.cursor_mode?config.cursor:0), fmt);
		radare_set_block_size_i(tmp);
		} break;
	case '*':
		metadata_comment_list();
		data_list();
		break;
	default:
		cons_printf("Usage: C[op] [arg] <@ offset>\n"
		"  CC [-][comment] @ here - add/rm comment\n"
		"  CF [-][len]  @ here    - add/rm function\n"
		"  Cx [-][addr] @ here    - add/rm code xref\n"
		"  CX [-][addr] @ here    - add/rm data xref\n"
		"  Cc [num]     - converts num bytes to code\n"
		"  Cd [num]     - converts to data bytes\n"
		"  Cs [num]     - converts to string\n"
		"  Cf [num]     - folds num bytes\n"
		"  Cu [num]     - unfolds num bytes\n"
		"  C*           - list metadata database\n");
	}

	return 0;
}

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


static void radare_set_limit(char *arg)
{
	if ( arg[0] != '\0' )
		config.limit = get_math(arg);

	D eprintf("limit = "OFF_FMTd"\n", config.limit);
}

CMD_DECL(limit)
{
	char *text = input;
	for(;*text&&!iswhitespace(*text);text=text+1);
	for(;*text&&iswhitespace(*text);text=text+1);

	radare_set_limit(text);

	return 0;
}

CMD_DECL(move)
{
	char *text = input;
	for(;*text&&!iswhitespace(*text);text=text+1);
	for(;*text&&iswhitespace(*text);text=text+1);
	radare_move(text);

	return 0;
}

CMD_DECL(print)
{
	print_fmt_t fmt = MD_BLOCK;

	switch(input[0]) {
	case '\0':
	case '?':
		format_show_help( fmt );
		break;
	case 'I':
		fmt = format_get(input[1]); //, fmt);
		if (fmt == FMT_ERR)
			format_show_help(MD_BLOCK|MD_ALWAYS|MD_EXTRA);
		else	radare_print(input+2, fmt);
		break;
	default:
		fmt = format_get(input[0]); //, fmt);
		if (fmt == FMT_ERR)
			format_show_help(MD_BLOCK|MD_ALWAYS|MD_EXTRA);
		else	radare_print(input+2, fmt);
	}

	return 0;
}

CMD_DECL(quit)
{
	if (input[0]=='!')
		exit(1);
	radare_exit();
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

CMD_DECL(flag)
{
	int ret = 0;
	char *text = input;
	char *eof = input + strlen(input)-1;

	for(;*text&&!iswhitespace(*text);text=text+1);
	for(;*text&&iswhitespace(*text);text=text+1);
	for(;iswhitespace(eof[0]);eof=eof-1) eof[0]='\0';

	switch(input[0]) {
	case 'o': radare_fortunes(); break;
	case '?': flag_help(); break;
	case 'g': flag_grep(text); break;
	case 'c': flag_cmd(text); break;
	case 'r': flag_rename_str(text); break;
	case 's': flag_space(input+1); break;
	case 'm': flag_space_move(text); break;
	case 'd': print_flag_offset(config.seek); NEWLINE; break;
	default:
		switch(text[0]) {
		case '\0': flag_list(text); break;
		case '*': flag_set("*",0,0); break;
		case '-': flag_clear(text+1); break;
		default:
			ret = flag_set(text, config.seek, input[0]=='n');
			D { if (!ret) { flags_setenv(); } } }
	}

	return ret;
}

CMD_DECL(undowrite)
{
	switch(input[0]) {
	case 'w':
		input = input +1;
	case ' ':
	case '\0':
		if (input[0] == '\0')
			undo_write_list();
		else {
			if (input[1]=='-')
				undo_write_set(atoi(input+2), 1);
			else
				undo_write_set(atoi(input+1), 0);
		}
		break;
	case '?':
	default:
		cons_printf(
		"Usage: > u 3   ; undo write change at index 3\n"
		"       > u -3  ; redo write change at index 3\n"
		"       > u     ; list all write changes\n");
		break;
	}

	return 0;
}

CMD_DECL(seek)
{
	u64 new_off = 0;
	char *input2  = strdup(input);
	char *text    = input2;
	int whence    = SEEK_SET;
	int sign      = 1;

	for(;*text&&!iswhitespace(*text);text=text+1);
	for(;*text&&iswhitespace(*text);text=text+1);

	if (strchr(input, '?')) {
		cons_printf("Usage: > s 0x128 ; absolute seek\n");
		cons_printf("       > s +33   ; relative seek\n");
		cons_printf("       > s-     ; undo seek\n");
		cons_printf("       > s+     ; redo seek\n");
		cons_printf("       > s*     ; show seek history\n");
		cons_printf("       > .s*    ; flag them all\n");
		cons_printf("       > s!     ; reset seek history\n");
		return;
	}

	if (input[0]=='!'||input[0]=='*'||input[0]=='+'||input[0]=='-'||(input[0]>='0'&&input[0]<'9'))
		text = input;

	if (text[0] != '\0') {
		switch(text[0]) {
		case '!': sign = -2; text++; break;
		case '*': sign = 0; text++; break;
		case '-': sign = -1; text++; whence = SEEK_CUR; break;
		case '+': sign = 1;  text++; whence = SEEK_CUR; break; }

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
			return;
		}

		new_off = get_math( text );

		if (whence == SEEK_CUR) {
			new_off *= sign;
			new_off += config.seek;
			whence   = SEEK_SET; // stupid twice
		}

		if (new_off<0) new_off = 0;
		if (radare_seek(new_off, whence) < 0)
			eprintf("Couldn't seek: %s\n", strerror(errno));
		undo_push();
	} else {
		if (text[0]!='\0')
			eprintf("Whitespace expected after 's'.\n");
		else	D printf(OFF_FMT"\n", (u64)config.seek);
	}

	radare_read(0);

	free(input2);

	return 0;
}

CMD_DECL(info)
{
#if 0
	if (strchr(input, '*')) {
		printf("b %d\n", config.block_size);
		printf("e %d\n", config.endian);
		printf("s 0x"OFF_FMTx"\n", config.seek);
		printf("B 0x"OFF_FMTx"\n", config.baddr);
		printf("l 0x"OFF_FMTx"\n", config.limit);
		return 0;
	}
#endif

	INILINE;
	cons_printf(" file    %s",   strget(config.file)); NEWLINE;
	cons_printf(" rdb     %s",   strget(config_get("file.rdb"))); NEWLINE;
	cons_printf(" project %s",   strget(config_get("file.project"))); NEWLINE;
	cons_printf(" mode    %s",   config_get("file.write")?"read-write":"read-only"); NEWLINE;
	cons_printf(" debug   %d",   config.debug); NEWLINE;
	cons_printf(" endian  %d  ( %s )",   config.endian, config.endian?"big":"little"); NEWLINE;
	cons_printf(" baddr   "OFF_FMTd"  ( 0x"OFF_FMTx" )", config.baddr, config.baddr); NEWLINE;
	cons_printf(" bsize   %d  ( 0x%x )", config.block_size, config.block_size); NEWLINE;
	cons_printf(" seek    "OFF_FMTd" 0x"OFF_FMTx,
		(u64)config.seek, (u64)config.seek); NEWLINE;
	cons_printf(" delta   %lld\n", config_get_i("cfg.delta")); 
	cons_printf(" count   %lld\n", config_get_i("cfg.count")); 
	//fflush(stdout);
	//print_flag_offset(config.seek);
	cons_printf(" size    "OFF_FMTd" \t 0x"OFF_FMTx,
		(u64)config.size, (u64)config.size); NEWLINE;
	cons_printf(" limit   "OFF_FMTd" \t 0x"OFF_FMTx,
		(u64)config.limit, (u64)config.limit); NEWLINE;

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

	//radare_read(0);
	switch (input[0]) {
	case 'd':
		off = (unsigned int) get_offset(input+1);
		radare_compare((unsigned char*)&off, config.block, 4);
		break;
	case 'f':
		if (input[1]!=' ') {
			eprintf("Please. use 'wf [file]'\n");
			return 0;
		}
		fd = fopen(input+2,"r");
		if (fd == NULL) {
			eprintf("Cannot open file '%s'\n",input+2);
			return 0;
		}
		buf = (unsigned char *)malloc(config.block_size);
		fread(buf, config.block_size, 1, fd);
		fclose(fd);
		radare_compare(buf, config.block, config.block_size);
		free(buf);
		break;
	case 'x':
		if (input[1]!=' ') {
			eprintf("Please. use 'wx 00 11 22'\n");
			return 0;
		}
		buf = (unsigned char *)malloc(strlen(input+2));
		ret = hexstr2binstr(input+2, buf);
		radare_compare(buf, config.block, ret);
		free(buf);
		break;
	case ' ':
		radare_compare((unsigned char*)input+1,config.block, strlen(input+1)+1);
		break;
	case '?':
		eprintf(
		"Usage: c[?|d|x|f] [argument]\n"
		"  c  [string]   - compares a plain with escaped chars string\n"
		"  cd [offset]   - compare a doubleword from a math expression\n"
		"  cx [hexpair]  - compare hexpair string\n"
		"  cf [file]     - compare contents of file at current seek\n");
		break;
	default:
		eprintf("Usage: c[?|d|x|f] [argument]\n");
		return 0;
	}

	return 0;
}

CMD_DECL(write)
{
	int ret;
	u64 delta = 0;
	u64 off;
	u64 here = config.seek + (config.cursor_mode?config.cursor:0);
	u64 back = config.seek;

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

						if (c == '#')
							commented = 1;
						else if (c == '\n' || c == '\r')
							commented = 0;
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
			io_write(config.fd, &addr8, 8);
		} else {
			unsigned long addr4;
			unsigned long addr4_= (unsigned long)off;
			endian_memcpy((u8*)&addr4, (u8*)&addr4, 4);
			/* 4 byte addr */
			undo_write_new(config.seek, (u8*)&addr4, 4);
			io_write(config.fd, &addr4, 4);
		}
		break;
	case 'b': {
		char *tmp;
		char out[9999]; // XXX
		int size, osize = hexstr2binstr(input+1, out);
		if (osize>0) {
			tmp = (char *)malloc(config.block_size);
			memcpy_loop(tmp, out, config.block_size, osize);
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
		unsigned char data[256];
		char* aux = strdup ( config_get("asm.arch") );
		u64 seek = config.seek;
		int ret = rasm_asm(aux, &seek, input+2, data);
		free ( aux );
		if (ret<1)
			eprintf("Invalid opcode for asm.arch. Try 'wa?'\n");
		else {
			undo_write_new(config.seek, data, ret);
			io_write(config.fd, data, ret);
		}
		
		} break;
	case 'x':
		if (input[1]!=' ') {
			eprintf("Please. use 'wx 00 11 22'\n");
			return 0;
		}
		ret = radare_write(input+2, WMODE_HEX);
		break;
	case 'w':
		if (input[1]!=' ') {
			eprintf("Please. use 'ww string-with-scaped-hex'.\n");
			return 0;
		}
		ret = radare_write(input+2, WMODE_WSTRING);
		break;
	case ' ':
		ret = radare_write(input+1, WMODE_STRING);
		break;
	case '?':
		eprintf(
		"Usage: w[?|w|x|f] [argument]\n"
		"  w  [string]   - write plain with escaped chars string\n"
		"  wa [opcode]   - write assembly using asm.arch and rasm\n"
		"  wA '[opcode]' - write assembly using asm.arch and rsc asm\n"
		"  wb [hexpair]  - circulary fill the block with these bytes\n"
		"  wv [expr]     - writes 4-8 byte value of expr (use cfg.bigendian)\n"
		"  ww [string]   - write wide chars (interlace 00s in string)\n"
		"  wx [hexpair]  - write hexpair string\n"
		"  wf [file]     - write contents of file at current seek\n"
		"  wF [hexfile]  - write hexpair contents of file\n");
		break;
	default:
		eprintf("Usage: w[?|a|A|d|w|x|f|F] [argument]\n");
		return 0;
	}
	radare_seek(back, SEEK_SET);

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
	u64 off;
	if (input[0] == '\0')
		off = config.block_size;
	else
		off = get_math(input);

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
	u64 off;
	if (input[0] == '\0')
		off = config.block_size;
	else
		off = get_math(input);

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

	return 0;
}

print_fmt_t last_search_fmt = FMT_ASC;

// TODO: show /k && /m together
CMD_DECL(search) {
	char buf[128];
	char *input2 = (char *)strdup(input);
	char *text   = input2;
	int  len,i = 0,j = 0;
	u64 seek = config.seek;
	char *ptr;

	switch(text[0]) {
	case '\0':
	case '?':
		eprintf(
		" / \\x7FELF      ; plain string search (supports \\x).\n"
		" /. [file]      ; search using the token file rules\n"
		" /s [string]    ; strip strings matching optional string\n"
		" /x A0 B0 43    ; hex byte pair binary search.\n"
		" /k# keyword    ; keyword # to search\n"
		" /m# FF 0F      ; Binary mask for search '#' (optional)\n"
		" /n[-]          ; seek to hit index N (/n : next, /n- : prev)\n"
		" /a [opcode]    ; Look for a string in disasembly\n"
		" /A             ; Find expanded AES keys from current seek(*)\n"
		" /w foobar      ; Search a widechar string (f\\0o\\0o\\0b\\0..)\n"
		" /r 0,2-10      ; launch range searches 0-10\n"
		" /p len         ; search pattern of length = len\n"
		" //             ; repeat last search\n");
		break;
	case 'p':
		do_byte_pat(atoi(text+1));
		radare_seek(seek, SEEK_SET);
		break;
	case 'm':
		if (text[1]=='?') {
			eprintf("/k[number] [keyword]\n");
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
		default:
			radare_search_seek_hit(atoi(text+1));
			break;
		}
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
	case 's':
		radare_strsearch(strchr(text,' '));
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
		search_range(buf);
		} break;
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
	case '.':
		search_from_file(text+2);
		break;
	case 'x':
		free(input2);
		len = strlen(input+2);
		input2 = (char *)malloc((len<<1)+10);
		input2[0] = '\0';
		for(i=0;i<len;i+=j) {
			char buf[10];
			int ch = hexpair2bin(input+2+i);
			if (i+4<len && input[2+i+2]==' ')
				j = 3;
			else j = 2;
			if (ch == -1) break;
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
	free(input2);

	return 0;
}

CMD_DECL(shell)
{
	int ret = 0;

	prepare_environment(input);
	if (input[0]=='!')
		radare_system(input+1);
	else
		ret = io_system(input);

	destroy_environment(input);

	return ret;
}

CMD_DECL(help)
{
	if (strlen(input)>0) {
		u64 res = get_math(input);
		cons_printf("0x"OFF_FMTx" ; %lldd ; %lloo ; ", res, res, res);
		PRINT_BIN(res); NEWLINE;
	}
	else show_help_message();

	return 0;
}

CMD_DECL(default)
{
	D eprintf("Invalid command '%s'. Try '?'.\n", input);

	return 0;
}
