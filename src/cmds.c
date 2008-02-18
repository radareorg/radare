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
#include "radare.h"
#include "rdb/rdb.h"
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
//#include "eperl.h"

print_fmt_t last_print_format = FMT_HEXB;
extern command_t commands[];
extern void show_help_message();
#define uchar unsigned char

/* Call a command from the command table */
void commands_parse (const char *_cmdline)
{
	char *cmdline = (char *)_cmdline;
	command_t *cmd;

	for(;cmdline[0]&&(cmdline[0]=='\x1b'||cmdline[0]==' ');
	cmdline=cmdline+1);

	// null string or comment
	if (cmdline[0]==0||cmdline[0]==';')
		return;

	if (cmdline[0]=='h') {
		(void)show_help_message();
		return;
	}

	for (cmd = commands; cmd->sname != 0; cmd++)
		if (cmd->sname == cmdline[0]) {
		    (void)cmd->hook(cmdline+1);
		    return;
		}

	/* default hook */
	(void)cmd->hook(cmdline);
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
		printf(" %-15s %s\n", cmdstr, cmd->help);
	}
	printf(" ? <expr>        calc    math expr and show result in hex,oct,dec,bin\n");
}


int fixed_width = 0;

command_t commands[] = {
	COMMAND('b', " [blocksize]",   "bsize   change the block size", blocksize),
	//COMMAND('B', " [baseaddr]",    "baddr   change virtual base address", baddr),
	//COMMAND('c', " [times]",       "count   limit of search hits and 'w'rite loops", count),
	COMMAND('c', "[f file] [bytes]","compare compare block with given value", compare),
	COMMAND('C', " [str]",         "comment adds a comment at the current position", comment),
	COMMAND('H', " [cmd]",         "performs a hack", hack),
	//COMMAND('e', " [0|1]",       "endian  change endian mode (0=little, 1=big)", endianess),
	COMMAND('e', " key=value",     "eval    evaluates a configuration expression", config_eval),
	COMMAND('f', "[d|-][name]",    "flag    flag the current offset (f? for help)", flag),
	//COMMAND('g', " [x,y]",         "gotoxy  move screen cursor to x,y", gotoxy),
	COMMAND('i', "",               "info    prints status information", status),
	//COMMAND('l', " [offset]",      "limit   set the top limit for searches", limit),
	COMMAND('m', " [size] [dst]",  "move    copy size bytes from here to dst", move),
	COMMAND('o', " [file]",        "open    open file", open),
	COMMAND('p', "[fmt] [len]",    "print   print data block", print),
	COMMAND('r', " [size]",        "resize  resize or query the file size", resize),
	COMMAND('P', "[so][i [file]]", "Project project Open, Save, Info", project),
	COMMAND('R', "[act] ([arg])",  "RDB     rdb operations", rdb),
	COMMAND('s', " [[+,-]pos]",    "seek    seek to absolute/relative expression", seek),
	COMMAND('u', "[!|?|u]",        "undo    undo seek (! = reset, ? = list, u = redo)", undoseek),
	COMMAND('V', "",               "Visual  go visual mode", visual),
	COMMAND('w', "[x] [string]",   "write   write ascii/hexpair string here", write),
	COMMAND('x', " [length]",      "examine the same as p/x", examine),
	COMMAND('y', "[y] [length]",   "yank    copy n bytes from cursor to clipboard (yy=paste)", yank),
	//COMMAND('Y', " [length]",      "Ypaste  copy n bytes from clipboard to cursor", yank_paste),
	COMMAND('.', "[!cmd]|[ file]", "script  interpret a commands script", interpret),
	COMMAND('_', " [oneliner]",    "perl/py interpret a perl/python script (use #!perl)", interpret_perl),
	COMMAND('-', "[size]",         "prev    go to previous block (-= block_size)", prev),
	COMMAND('+', "[size]",         "next    go to next block (+= block_size)", next),
	COMMAND('<', "",               "preva   go previous aligned block", prev_align),
	COMMAND('>', "",               "nexta   go next aligned block", next_align),
	COMMAND('/', "[?] [str]",      "search  find matching strings", search),
	COMMAND('!', "[command]",      "system  execute a shell command", shell), 
	//COMMAND('%', "ENVVAR [value]", "setenv  gets or sets a environment variable", envvar),
	COMMAND('#', "[hash|!lang]",   "hash    hash current block (#? or #!perl)", hash),
	COMMAND('?', "",               "help    show the help message", help),
	COMMAND('q', "",               "quit    close radare shell", quit),
	COMMAND( 0, NULL, NULL, default)
};

CMD_DECL(config_eval)
{
	int i = 0;
	for(i=0;input[i]&&input[i]!=' ';i++);
	config_eval(input+i);
}

CMD_DECL(project)
{
	char *arg = input + 2;

	switch(input[0]) {
	case 'o':
		if (input[1])
			project_open(arg);
		else	project_open(config_get("file.project"));
		break;
	case 's':
		if (input[1])
			project_save(arg);
		else	project_save(config_get("file.project"));
		break;
	case 'i':
		if (input[1])
			project_info(arg);
		else	project_info(config_get("file.project"));
		break;
	default:
		cons_printf(
		" Po [file]  open project\n"
		" Ps [file]  save project\n"
		" Pi [file]  info\n");
		break;
	}
}

CMD_DECL(hack)
{
	if (strnull(input)) {
		radare_hack_help();
	} else {
		radare_hack(input);
	}
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
		return;
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
		return;
	}
	/* remove */
	if (text[0]=='-') {
		int num = atoi(text+1);
		int i = 0;
		struct list_head *pos;
		list_for_each(pos, &config.rdbs) {
			struct program_t *mr = list_entry(pos, struct program_t, list);
			if (i ==  num) {
				// XXX MUST FREE!
				list_del(&(mr->list));
				return;
			}
			i++;
		}
		eprintf("Not found\n");
		return;
	}
	/* open */
	if (input[0] ==' ') {
		struct program_t *prg = program_open(text+1); // XXX FIX stripstring and so
		list_add_tail(&prg->list, &config.rdbs);
		return;
	}
	/* grefusa diffing */
	if (input[0] =='d') {
		int a, b;
		int i = 0;
		struct list_head *pos;
		struct program_t *mr0;
		struct program_t *mr1;

		sscanf(text, "%d %d", &a, &b);

		eprintf("RDB diffing %d vs %d\n", a, b);
		list_for_each(pos, &config.rdbs) {
			struct program_t *mr = list_entry(pos, struct program_t, list);
			if (i ==  a) mr0 = mr;
			if (i ==  b) mr1 = mr;
			i++;
		}

		rdb_diff(mr0, mr1, 0);
		return;
	}
	/* draw graph */
	if (input[0] =='G') {
#if VALA
		int num = atoi(text);
		int i = 0;
		struct list_head *pos;
		list_for_each(pos, &config.rdbs) {
			struct program_t *mr = list_entry(pos, struct program_t, list);
			if (i ==  num) {
				grava_program_graph(mr);
				return;
			}
			i++;
		}
		eprintf("Not found\n");
#else
		eprintf("Compiled without vala-gtk\n");
#endif
		return;
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
}

CMD_DECL(baddr)
{
	int i;
	for(i=0;input[i]&&input[i]!=' ';i++);
	if (input[i]!='\0')
		config.baddr = get_math(input+i+1);
	D { printf("0x"OFF_FMTx, config.baddr); NEWLINE; }
}

CMD_DECL(hash)
{
	int i;
	char buf[1024];

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
		return;
	}
	// XXX doesnt works with dbg:///
	// XXX use real temporal file instead of /tmp/xx
	if (config.debug) {
		radare_cmd("pr > /tmp/xx", 0);
		snprintf(buf, 1000, "hasher -fa '%s' '/tmp/xx'", input);
	} else
	snprintf(buf, 1000, "hasher -a '%s' -S "OFF_FMTd" -L '"OFF_FMTd"' '%s' | head -n 1", 
		input, (u64)config.seek, (u64)config.block_size, config.file);

	io_system(buf);

	if (config.debug)
		unlink("/tmp/xx");
}

CMD_DECL(interpret_perl)
{
#if HAVE_PERL
	char *ptr;
	char *cmd[] = { "perl", "-e", ptr, 0};
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
		return;
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
}

CMD_DECL(open)
{
	char *ptr = strdup(input);
	clear_string(ptr);
	config.file = ptr;
	radare_open(0);
	//radare_go();
	free(ptr);
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
		return;
	} else
	if (text[0]=='?') {
		printf("Get and set environment variables.\n");
		printf("To undefine use '-' or '(null)' as argument: f.ex: %%COLUMNS (null)\n");
		return;
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
		return;
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
}

CMD_DECL(blocksize)
{
	radare_set_block_size(input);
}

CMD_DECL(comment)
{
	char buf[1024];
	char *text = input;

	while(text[0]==' ')
		text = text + 1 ;
	cons_flush();
	if (text[0]) {
		if (text[0]=='?')
			cons_printf("Usage: C [-addr|comment] @ address\n"
				"adds or removes comments for disassembly\n");
		else
		if (text[0]=='-')
			metadata_comment_del(config.seek, text+1);
		else
			metadata_comment_add(config.seek, text);
	} else {
		metadata_comment_list();
	}
}

#if 0
CMD_DECL(count)
{
	char *text = input;
	for(;*text&&!iswhitespace(*text);text=text+1);
	for(;*text&&iswhitespace(*text);text=text+1);
	if (text[0]!='\0'&&text[0]!=' ')
		config.count = get_math(text);
	D eprintf("count = %d\n", config.count);
}
#endif

CMD_DECL(endianess)
{
	char *text = input;
	for(;*text&&!iswhitespace(*text);text=text+1);
	for(;*text&&iswhitespace(*text);text=text+1);

	if (text[0]!='\0'&&text[0]!=' ')
		config.endian = atoi(text)?1:0;

	D eprintf("endian = %d %s\n", config.endian, (config.endian)?"big":"little");
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
}

CMD_DECL(move)
{
	char *text = input;
	for(;*text&&!iswhitespace(*text);text=text+1);
	for(;*text&&iswhitespace(*text);text=text+1);
	radare_move(text);
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
		fmt = format_get(input[1], fmt);
		if (fmt == FMT_ERR)
			format_show_help(MD_BLOCK|MD_ALWAYS|MD_EXTRA);
		else	radare_print(input+2, fmt, formats[fmt].mode|FMT_INV);
		break;
	default:
		fmt = format_get(input[0], fmt);
		if (fmt == FMT_ERR)
			format_show_help(MD_BLOCK|MD_ALWAYS|MD_EXTRA);
		else	radare_print(input+1, fmt, formats[fmt].mode);
	}
}

void radare_exit()
{
	char *ptr;
	int ret;
	char ch;

	ret = io_close(config.fd);

	if (ret==-2)
		return;
	if ( ret == 0) {
		#if HAVE_LIB_READLINE
		rad_readline_finish();
		#endif

		/* save project : user confirmation */
		ptr = config_get("file.project");

		if (ptr && ptr[0] ) {
			terminal_set_raw(1);
			printf("Save project? (Y/n) ");
			fflush(stdout);
			read(0,&ch, 1);
			write(1, &ch, 1);
			write(1, "\n", 1);
			if (ch != 'n') 
				project_save(ptr);
			terminal_set_raw(0);
		}
	}
	exit(0);
}

CMD_DECL(quit)
{
	radare_exit();
}

static void radare_resize(char *arg)
{
	int fd_mode = O_RDONLY;
	u64 size  = get_math(arg);

	// XXX move this check into a only one function for all write-mode functions
	// or just define them as write-only. and activate/deactivate them from
	// the readline layer.
	if (!config_get("cfg.write")) {
		eprintf("Only available for write mode. (-w)\n");
		return;
	}

	if (config.size == -1) {
		eprintf("Sorry, this file cannot be resized.\n");
		return;
	}
	if ( arg[0] == '\0' ) {
		D printf("Size:  "OFF_FMTd"\n", config.size);
		D printf("Limit: "OFF_FMTd"\n", config.limit);
		return;
	}
	if (arg[1]=='x') sscanf(arg, OFF_FMTx, &size);

	printf("resize "OFF_FMTd" "OFF_FMTd"\n", config.size, size);
	if (size < config.size) {
		D printf("Truncating...\n");
		ftruncate(config.fd, size);
		close(config.fd);
		if (config.limit > size)
			config.limit = size;
		config.size = size;
	}
	if (size > config.size ) {
		char zero = '\0';
		D printf("Expanding...\n");
		radare_seek(size, SEEK_SET);
		write(config.fd, &zero, 1);
		close(config.fd);
		config.limit = size;
		config.size  = size;
	}

	if (config_get("cfg.write"))
		fd_mode = O_RDWR;

	radare_open(1);
}

CMD_DECL(resize)
{
	char *text = input;

	if (strchr(input, '*')) {
		printf("0x"OFF_FMTx"\n", config.size);
		return;
	}

	for(;*text&&!iswhitespace(*text);text=text+1);
	for(;*text&&iswhitespace(*text);text=text+1);

	if (text[0] == '\0') {
		D printf("Size: "OFF_FMTd" (0x"OFF_FMTx")\n",
			(u64) config.size, (u64) config.size);
	} else	radare_resize(text);
}

CMD_DECL(flag)
{
	int ret;
	char *text = input;
	char *eof = input + strlen(input)-1;

	for(;*text&&!iswhitespace(*text);text=text+1);
	for(;*text&&iswhitespace(*text);text=text+1);
	for(;iswhitespace(eof[0]);eof=eof-1) eof[0]='\0';

	if (input[0]=='?') { flag_help(); } else
	if (input[0]=='o') { radare_fortunes(); } else
	if (input[0]=='d') { print_flag_offset(config.seek); NEWLINE; } else
	if (text[0]=='\0') { flag_list(text); } else
	if (text[0]=='*') { flag_set("*",0,0); } else
	if (text[0]=='-') { flag_clear(text+1); }
	else {
		ret = flag_set(text, config.seek, input[0]=='n');
		D if (ret) {
			if (!config.debug)
				eprintf("flag '%s' redefined to "OFF_FMTs"\n", text, config.seek);
		} else {
			flags_setenv();
			eprintf("flag '%s' at "OFF_FMT" and size %d\n",
				text, config.seek, config.block_size);
		}
	}
}

CMD_DECL(undoseek)
{
	switch(input[0]) {
	case 'n':
		undo_seek();
	case '*':
	case 'l':
		undo_list();
		break;
	case '!':
	case '-':
		undo_reset();
		break;
	case 'u':
		undo_redo();
		break;
	case '?':
	default:
		cons_printf(
		"un   undo seek\n"
		"uu   redo\n"
		"u*   list all seeks done\n"
		"u!   reset seek history\n"
		"u?   help this help\n");
		break;
	}
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

	if (text[0] != '\0') {
		switch(text[0]) {
		case '-': sign = -1; text++; whence = SEEK_CUR; break;
		case '+': sign = 1;  text++; whence = SEEK_CUR; break; }

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
}

CMD_DECL(status)
{
#if 0
	if (strchr(input, '*')) {
		printf("b %d\n", config.block_size);
		printf("e %d\n", config.endian);
		printf("eval  %d\n", config.count);
		printf("s 0x"OFF_FMTx"\n", config.seek);
		printf("B 0x"OFF_FMTx"\n", config.baddr);
		printf("l 0x"OFF_FMTx"\n", config.limit);
		return;
	}
#endif

	INILINE;
	cons_printf(" file    %s",   config.file); NEWLINE;
	cons_printf(" rdb     %s",   config_get("file.rdb")); NEWLINE;
	cons_printf(" mode    %s",   config_get("cfg.write")?"read-write":"read-only"); NEWLINE;
	cons_printf(" debug   %d",   config.debug); NEWLINE;
	cons_printf(" endian  %d   \t %s",   config.endian, config.endian?"big":"little"); NEWLINE;
//	printf(" count   %d   \t 0x%x", config.count, config.count); NEWLINE;
	cons_printf(" baddr   "OFF_FMTd" \t 0x"OFF_FMTx, config.baddr, config.baddr); NEWLINE;
	cons_printf(" bsize   %d   \t 0x%x", config.block_size, config.block_size); NEWLINE;
	cons_printf(" seek    "OFF_FMTd" 0x"OFF_FMTx,
		(u64)config.seek, (u64)config.seek); NEWLINE;
	cons_printf(" delta   "); 
	fflush(stdout);
	print_flag_offset(config.seek);
	cons_printf("\n size    "OFF_FMTd" \t 0x"OFF_FMTx,
		(u64)config.size, (u64)config.size); NEWLINE;
	cons_printf(" limit   "OFF_FMTd" \t 0x"OFF_FMTx,
		(u64)config.limit, (u64)config.limit); NEWLINE;

	if (config.debug)
		io_system("info");
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
		radare_compare(&off, config.block, 4);
		break;
	case 'f':
		if (input[1]!=' ') {
			eprintf("Please. use 'wf [file]'\n");
			return;
		}
		fd = fopen(input+1,"r");
		if (fd == NULL) {
			eprintf("Cannot open file\n");
			return 0;
		}
		buf = (char *)malloc(config.block_size);
		fread(buf, config.block_size, 1, fd);
		fclose(fd);
		radare_compare(buf, config.block, config.block_size);
		free(buf);
		break;
	case 'x':
		if (input[1]!=' ') {
			eprintf("Please. use 'wx 00 11 22'\n");
			return;
		}
		buf = (unsigned char *)malloc(strlen(input+2));
		ret = hexstr2binstr(input+2, buf);
		radare_compare(buf, config.block, ret);
		free(buf);
		break;
	case ' ':
		radare_compare(input+1,config.block, strlen(input+1)+1);
		break;
	case '?':
		eprintf(
		"Usage: w[?|w|x|f] [argument]\n"
		"  c  [string]   - compares a plain with escaped chars string\n"
		"  cd [offset]   - compare a doubleword from a math expression\n"
		"  cx [hexpair]  - compare hexpair string\n"
		"  cf [file]     - compare contents of file at current seek\n");
		break;
	default:
		eprintf("Usage: c[?|d|x|f] [argument]\n");
		return;
	}
}

CMD_DECL(write)
{
	int ret;
	unsigned long off;

	switch (input[0]) {
	case 'd':
		off = (unsigned long) get_offset(input);
		io_write(config.fd, &off, 4);
		break;
	case 'f':
		if (input[1]!=' ') {
			eprintf("Please. use 'wf [file]'\n");
			return;
		}
		radare_poke(input+2);
		break;
	case 'A': {
		unsigned char data[1024];
		snprintf(data, 1023, "wx `!rsc asm '%s'", input + 2);
		radare_cmd(data, 0);
		}
		break;
	case 'a': {
		// TODO: use config.cursor here
		unsigned char data[256];
		int ret = rasm_asm(config_get("asm.arch"), config.seek, input+2, data);
		if (ret<1)
			eprintf("Invalid opcode for asm.arch. Try 'wa?'\n");
		else {
			u64 tmp = config.seek;
			radare_seek(config.seek, SEEK_SET);
			io_write(config.fd, data, ret);
			radare_seek(tmp, SEEK_SET);
		}
		
		} break;
	case 'x':
		if (input[1]!=' ') {
			eprintf("Please. use 'wx 00 11 22'\n");
			return;
		}
		ret = radare_write(input+2, WMODE_HEX);
		break;
	case 'w':
		if (input[1]!=' ') {
			eprintf("Please. use 'ww string-with-scaped-hex'.\n");
			return;
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
		"  wd [offset]   - writes a doubleword from a math expression\n"
		"  ww [string]   - write wide chars (interlace 00s in string)\n"
		"  wx [hexpair]  - write hexpair string\n"
		"  wf [file]     - write contents of file at current seek\n");
		break;
	default:
		eprintf("Usage: w[?|a|A|d|w|x|f] [argument]\n");
		return;
	}
}

CMD_DECL(examine)
{
	switch(input[0]) {
	case '\0':
		radare_print(input, FMT_HEXB, MD_BLOCK);
		break;
	case ' ':
		radare_print(input+1, FMT_HEXB, MD_BLOCK);
		break;
	default:
		eprintf("Error parsing command.\n");
		break;
	}
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
}

print_fmt_t last_search_fmt = FMT_ASC;

// TODO: show /k && /m together
CMD_DECL(search) {
	char buf[128];
	char *input2 = (char *)strdup(input);
	char *text   = input2;
	int  len,i = 0,j = 0;
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
		" /a             ; Find expanded AES keys from current seek(*)\n"
		" /w foobar      ; Search a widechar string (f\\0o\\0o\\0b\\0..)\n"
		" /r 0,2-10      ; launch range searches 0-10\n"
		" //             ; repeat last search\n");
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
			extern char **environ;
			for(i=0;environ[i];i++) {
				if (!memcmp(environ[i], "SEARCH[", 7)) {
					int j = atoi(environ[i]+7);
					sprintf(buf, "SEARCH[%d]", j);
					ptr = getenv(buf);
					if (ptr) {
						cons_printf("%02d %s\n", j, ptr);
					} else {
						cons_printf("%02d (no keyword)\n", i);
					}
				}
			}
		}
		break;
	case 'a':
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
}

CMD_DECL(shell)
{
	prepare_environment(input);
	io_system(input);
	destroy_environment(input);
}

CMD_DECL(help)
{
	if (strlen(input)>0) {
		u64 res = get_math(input);
		cons_printf("0x"OFF_FMTx" ; %lldd ; %lloo ; ", res, res, res);
		PRINT_BIN(res); NEWLINE;
	}
	else show_help_message();
}

CMD_DECL(default)
{
	D eprintf("Invalid command '%s'. Try '?'.\n", input);
}
