/*
 * Copyright (C) 2006, 2007, 2008
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
#include "radare.h"
#include "code.h"
#include "config.h"
#include "utils.h"
#include "print.h"
#include "plugin.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#if __UNIX__
#include <termios.h>
#include <sys/ioctl.h>
#endif
#if DEBUGGER
#include "dbg/debug.h"
#include "dbg/arch/arch.h"
#endif
#include "utils.h"
#include "cmds.h"
#include "readline.h"
#include "flags.h"
#include "undo.h"

struct binding {
	unsigned char key;
	char *cmd;
};

static int cursorseek=1; /* MUST BE IN SCR.CURSORSEEK */
static unsigned char *yank_buffer = NULL;
static int yank_buffer_size = 0;
static int do_repeat=0;
static int repeat=0;
static int repeat_weight=1;
static int scraccel = 0;
static int accel=1;
static int nbds = 0;
static struct binding *bds = NULL;
static int inv = 0;
static u64 mark = 0;
#define VMODES 12
static char modes[VMODES] =
{ FMT_HEXB, FMT_VISUAL, FMT_UDIS, FMT_OCT, FMT_CSTR,
  FMT_BIN, FMT_URLE, FMT_ASC, FMT_ASHC, FMT_HEXB, 0 };

static char *modestr[VMODES] =
{ "hexb", "visual", "disasm", "octal", "cstr",
  "binary", "url-encoding", "ascii", "ashc", "hexb" };
static char *zoom_string="zoom";

CMD_DECL(rotate_print_format);
CMD_DECL(rotate_print_format_prev);
CMD_DECL(insert_assembly);
CMD_DECL(insert_assembly_rsc);
CMD_DECL(insert_assembly_hack);
CMD_DECL(insert_string);
CMD_DECL(insert_hexa_string);
CMD_DECL(seek_to_end);
CMD_DECL(seek_to_flag);
CMD_DECL(step_in_dbg);
CMD_DECL(stepu_in_dbg);
CMD_DECL(stepo_in_dbg);
CMD_DECL(xrefs_here);
CMD_DECL(seek0);
CMD_DECL(invert);
CMD_DECL(insert);
CMD_DECL(add_comment);
CMD_DECL(edit_comment);
CMD_DECL(yank);
CMD_DECL(yank_paste);
CMD_DECL(zoom);
//CMD_DECL(trace);
CMD_DECL(zoom_reset);
CMD_DECL(edit_screen_filter);
CMD_DECL(zoom_reset);
CMD_DECL(setblocksize);

/* TODO: Move 'u', 't', 'e' ... here */
command_t keystrokes[] = {
	/*   key, wait, description, callback */
	COMMAND('#', 0, "edit screen filter", edit_screen_filter),
	COMMAND('g', 0, "seek to offset 0", seek0),
	COMMAND('G', 0, "seek to end of file", seek_to_end),
	COMMAND(';', 0, "add a comment", add_comment),
	COMMAND('p', 0, "change to next print format", rotate_print_format),
	COMMAND('P', 0, "change to previous print format", rotate_print_format_prev),
	COMMAND('a', 0, "insert assembly", insert_assembly),
	//COMMAND('t', 0, "simulate trace cursor position", trace),
	COMMAND('A', 0, "insert assembly", insert_assembly_rsc),
	COMMAND('y', 0, "yank (copy selected block to clipboard)", yank),
	COMMAND('Y', 0, "Yankee (paste clipboard here)", yank_paste),
	COMMAND('=', 0, "insert assembly hack", insert_assembly_hack),
	COMMAND('x', 0, "show xrefs of the current offset", xrefs_here),
	COMMAND('i', 0, "insert mode (tab to change hex,asm,ascii, q to back)", insert),
	COMMAND('I', 0, "invert current block", invert),
	COMMAND('_', 0, "set block size", setblocksize),
	COMMAND('z', 0, "zoom full/block with pO", zoom),
	COMMAND('Z', 0, "resets zoom preferences", zoom_reset),
	COMMAND('w', 1, "write hex string", insert_hexa_string),
	/* debugger */
        COMMAND('s', 0, "step into the debugger", step_in_dbg),
        COMMAND('S', 0, "step over the debugger", stepo_in_dbg),
	COMMAND('\0',0, NULL, seek_to_flag)
};


#define TITLE if (config.color) cons_printf("\x1b[36m");
#define TITLE_END if (config.color) cons_printf("\x1b[0m");

void visual_show_help()
{
	cons_strcat("\x1b[2J\x1b[0;0H\n");
	TITLE
	cons_printf("Visual keybindings:\n");
	TITLE_END
	cons_printf(
	":<cmd>     radare command (vi like)\n"
	";          edit or add comment\n"
	",.         ',' marks an offset, '.' seeks to mark or eip if no mark\n"
	"g,G        seek to beggining or end of file\n"
	"+-*/       +1, -1, +width, -width -> block size\n"
	"<>         seek block aligned (cursor mode = folder code)\n"
	"[]         adjust screen width\n"
	"a,A,=      insert patch assembly, rsc asm or !hack\n"
	"i          insert mode (tab to switch btw hex,asm,ascii, 'q' to normal)\n"
	"f,F        seek between flag list (f = forward, F = backward)\n"
	"n,N        seek between hits of the hit0 search\n"
	"t          visual track/browse flagspaces and flags\n"
	"e          visual eval configuration variables\n"
	"c          toggle cursor mode\n"
	"C          toggle scr.color\n"
	"d          convert cursor selected bytes to ascii, code or hex\n"
//	"b[k][cmd]  binds key 'k' to the specified command\n"
	"m          applies rfile magic on this block\n" // ???
	"I          invert block (same as pIx or so)\n"
	"y,Y        yank and Yankee aliases for copy and paste\n"
	"f,F        go next, previous flag (cursor mode to add/remove)\n"
	"h,j,k,l    scroll view to left, down, up, right.\n"
	"J,K        up down scroll one block.\n"
	"H,L        scroll left, right by 2 bytes (16 bits).\n"
	"p,P        switch between hex, bin and string formats\n"
	"x          show xrefs of the current offset\n"
	"w          write hexpair bytes in cursor\n"
	"q          exits visual mode\n");
	if (config.debug) {
	TITLE
	cons_printf("\nDebugger keybindings:\n");
	TITLE_END
	cons_printf(
	"!          show debugger commands help\n"
	"F1         commands help\n"
	"F2         set breakpoint (execute)\n"
	"F3         set watchpoint (read)\n"
	"F4         continue until here (!contuh)\n"
	"F6         continue until syscall (!contsc)\n"
	"F7         step in debugger user code (!step)\n"
	"F8         step over in debugger (!stepo)\n"
	"F9         continue execution (!cont)\n"
	"F10        continue until user code (!contu)\n"
	);
	}
	cons_flush();
}


CMD_DECL(edit_screen_filter)
{
	char buf[1024];
	const char *ed = config_get("cfg.editor");
	if (!strnull(ed)) {
		/* TODO: Handle errors, put a different file name for */
		snprintf(buf, 1023, "%s/.radare/screen-filter.txt", getenv("HOME"));
		config_set("file.scrfilter", buf);
		eprintf(":!!%s '%s'\n", ed, buf);
		{
		char buf2[1024];
		snprintf(buf2, 1024, "%s '%s'", ed, buf);
		radare_system(buf2);
		}
		cons_any_key();
		return 0;
	} else {
		eprintf("No cfg.editor defined\n");
		return 1;
	}
}

CMD_DECL(setblocksize)
{
	char bs[32];
	if (config.cursor_mode) {
		radare_set_block_size_i(config.cursor+1);
	} else {
		cons_set_raw(0);
		dl_prompt="Block size: ";
		bs[0]='\0';
		cons_fgets(bs, 31, 0, NULL);
		bs[strlen(bs)]='\0';
		if (bs[0] != '\0')
			radare_set_block_size_i(get_math(bs));
		cons_set_raw(1);
	}
}

CMD_DECL(insert)
{
	if (config_get("file.write")) {
		cons_clear();
		cons_printf("Insert mode activated.\n\nPress <esc> or 'q' to quit this mode\n");
		cons_printf("Use <tab> to change between ascii and hex\n");
		config.insert_mode = 1;
		config.cursor_mode = 1;
		config.ocursor = -1;
		cons_flush();
	} else {
		cons_printf("Not in write mode.\n");
		cons_any_key();
	}
	return 0;
}

CMD_DECL(invert)
{
	int inv = config_get_i("cfg.inverse")^1;
	config_set("cfg.inverse", inv?"true":"false");
	return 0;
}

CMD_DECL(zoom_reset)
{
	config.zoom.from = 0;
	config.zoom.size = config.size;
	config.zoom.piece = config.zoom.size/config.block_size;
	return 0;
}

#if 0
CMD_DECL(trace)
{
	if (config.cursor_mode)
		trace_add(config.seek+config.cursor);
	else	trace_add(get_offset("eip"));
	return 0;
}
#endif

static void visual_convert_bytes(int fmt)
{
	char argstr[128];
	int c;
	u64 off, len;
	if (fmt == -1) {
		cons_printf(
		"c - code\n"
		"d - data bytes\n"
		"s - string\n"
		"f - function\n"
		"m - memory format (pm)\n"
		"< - close folder\n"
		"> - open folder\n");
		cons_flush();
		c = cons_readchar();
		if (c != 'm' && c != 'c' && c!='d' && c!='s' && c!='f' && c!='<' && c!='>')
			return;
		fmt = FMT_HEXB;
		switch(c) {
		case 'm': fmt = DATA_STRUCT; break;
		case 'c': fmt = DATA_CODE; break;
		case 'd': fmt = DATA_HEX; break;
		case 's': fmt = DATA_STR; break;
		case 'f': fmt = DATA_FUN; 
			radare_cmd(".af*", 0);
			return;
		case '<': fmt = DATA_FOLD_C; break;
		case '>': fmt = DATA_FOLD_O; break;
		}
	}
	argstr[0]='\0';
	if (fmt==DATA_STRUCT) {
		const char *op = dl_prompt;
		print_mem_help();
		dl_prompt="> pm ";
		cons_set_raw(1);
		cons_fgets(argstr, 127, 0, NULL);
		argstr[strlen(argstr)]='\0';
		dl_prompt=op;
	}
	if (config.cursor > config.ocursor+1) {
		len = config.cursor-config.ocursor+1;
		off = config.ocursor;
	} else {
		len = config.ocursor-config.cursor+1;
		off = config.cursor;
	}
	off += config.seek;

	data_add_arg(off, fmt, argstr);

	if (config.cursor_mode)
		data_set_len(off, len);
	cons_clear();
}

CMD_DECL(zoom)
{
	if (config.zoom.size == 0)
		config.zoom.size = config.size;

	CLRSCR();
	config.zoom.enabled ^= 1;
	if (config.zoom.enabled) {
		config.cursor = config.block_size * config.seek/config.size;
	//	config.cursor_mode = 1;
		config.zoom.size = config.block_size;
		radare_cmd("pO", 0);
	} else {
		if (config.cursor_mode && ( config.ocursor != -1)) {
			config.zoom.from = config.cursor * config.zoom.piece;
			config.zoom.size = config.cursor-config.ocursor;
			if (config.zoom.size<0) config.zoom.size *= -1;
			config.zoom.size *=config.zoom.piece;
			config.zoom.piece = (config.zoom.size-config.zoom.from)/config.block_size;
			config.zoom.enabled = 1;
			config.seek = config.zoom.from;
			radare_cmd("pO", 0);
		} else {
			if (config.cursor_mode)
				config.seek = config.zoom.from + config.cursor * config.zoom.piece;
			radare_cmd("x", 0);
		}
		/* reset cursor */
		config.cursor_mode = 0;
		config.cursor = 0;
		config.ocursor = -1;
	}
	CLRSCR();
	return 0;
}

CMD_DECL(add_comment)
{
	int n;
	char buf[300];
	printf("Comment: ");
	fflush(stdout);
	strcpy(buf, "CC ");
	cons_set_raw(0);
	n = read(0, buf+2, 256);
	if (n<2) {
		sprintf(buf, "CC -0x%llx", config.seek+(config.cursor_mode?config.cursor:0));
		radare_cmd(buf,0);
	} else {
		buf[n+1]='\0';
		if (config.cursor_mode) {
			char ptr[128];
			sprintf(ptr, " @ +0x%x", config.cursor);
			strcat(buf, ptr);
		}
		radare_cmd(buf,0);
	}
	cons_set_raw(1);
	cons_clear();
	return 0;
}

CMD_DECL(seek0)
{
	if (config.cursor_mode)
		config.cursor = 0;
	else	config.seek = 0;
	return 0;
}

CMD_DECL(yank)
{
	int off = 0;
	char *ptr = strchr(input, ' ');
	if (ptr == NULL)
		ptr = input;
	switch(input[0]) {
	case 'y':
		cmd_yank_paste(input);
		return 0;
	case 't':
		radare_move(input+1);
		return 0;
	}
	if (ptr[0]=='?') {
		eprintf("Usage: y[ft] [length]\n");
		eprintf(" > y 10 @ eip     ; yanks 10 bytes from eip\n");
		eprintf(" > yy @ edi       ; write these bytes where edi points\n");
		eprintf(" > yt [len] dst   ; copy N bytes from here to dst\n");
		return 0;
	}

	free(yank_buffer);
	yank_buffer_size = get_math(ptr);
	if (yank_buffer_size > config.block_size || yank_buffer_size == 0)
		yank_buffer_size = config.block_size;
	if (config.cursor_mode) {
		if (config.ocursor==-1)
			yank_buffer_size = config.block_size-config.cursor;
		else yank_buffer_size = config.cursor-config.ocursor+1;
		if (yank_buffer_size<0) {
			yank_buffer_size = -yank_buffer_size+2;
			off = config.cursor;
		} else
			off = config.ocursor;
		if (yank_buffer_size==0)
			yank_buffer_size = 1;
		if (off == 0)
			off = config.cursor;
	}
	radare_read(0);
	yank_buffer = (unsigned char *)malloc(yank_buffer_size);
	memcpy(yank_buffer, config.block+off, yank_buffer_size);
	D eprintf("%d bytes yanked. off=%d data=%02x %02x %02x...\n", yank_buffer_size,
		off, yank_buffer[0], yank_buffer[1], yank_buffer[2]);

	if (config.visual) {
		cons_any_key();
		CLRSCR();
	}
	return 0;
}

CMD_DECL(yank_paste)
{
	if (yank_buffer_size == 0) {
		eprintf("No buffer yanked\n");
		if (config.visual) {
			cons_any_key();
			CLRSCR();
		}
	} else {
		if (config_get("file.write")) {
			char *ptr = strchr(input, ' ');
			u64 sz;
			int off = 0;
			u64 old = config.seek;
			sz = get_math(ptr?ptr:input);
			if (sz == 0)
				sz = yank_buffer_size;
			if (config.cursor_mode)
				off = config.cursor;
			radare_seek(old+off, SEEK_SET);
			io_write(config.fd, yank_buffer, sz);
			eprintf("%d bytes yanked.\n", (int) sz);
			radare_seek(old, SEEK_SET);
			radare_read(0);
		} else {
			printf("Not in write mode\n");
			if (config.visual) {
				cons_any_key();
				CLRSCR();
			}
		}
	}
}

/* deprecated!! */
CMD_DECL(xrefs_here)
{
	u64 addr;
	int foo;
	cons_printf("Select XREF from list:\n");
	data_xrefs_here(config.seek);
	cons_printf("==> ");
	cons_flush();
	foo = cons_readchar() - '0';
	if (foo<10) {
		addr = data_seek_to(config.seek, -1, foo);
		if (addr != 0) {
			radare_seek(addr, 0);
		}
	}
	return 0;
}

CMD_DECL(stepu_in_dbg)
{
	if (!config.debug) {
		eprintf("not in debugger\n");
		cons_any_key();
	} else {
		eprintf("Stepping to user code. wait a while...\n");
		//eprintf("TODO: should take care about the backtrace and use...\n");
		fflush(stderr);
		radare_cmd("!stepu", 0);
	}
	radare_sync();
	return 0;
}

CMD_DECL(step_in_dbg)
{
	if (!config.debug) {
		eprintf("not in debugger\n");
		cons_any_key();
	} else
		radare_cmd("!step", 0);
	radare_sync();
	//trace_add(get_offset("eip"));
	return 0;
}

CMD_DECL(stepo_in_dbg)
{
	if (!config.debug) {
		eprintf("not in debugger\n");
		cons_any_key();
	} else
		radare_cmd("!stepo", 0);
	radare_sync();
	return 0;
}

CMD_DECL(seek_to_flag)
{
	flag_t *flg;
	unsigned char key;

	flag_list("");
	key = cons_readchar();

	if (key>='0' && key<='9') {
		flg = flag_get_i(key-'0');
		if (flg) {
			radare_seek(flg->offset, SEEK_SET);
			config.block_size = flg->length;
		}
	}
	return 0;
}

CMD_DECL(seek_to_end)
{
	if (config.cursor_mode)
		config.cursor = config.block_size - 1;
	else	config.seek = config.size - config.block_size;
	return 0;
}

CMD_DECL(insert_assembly_rsc)
{
	if (!config_get("file.write")) {
		eprintf("Sorry, but you're not in read-write mode\n");
		cons_any_key();
		return 1;
	}

	printf("write assembly (end with ^d):\n");
	fflush(stdout);
	cons_set_raw(0);
	radare_cmd("wA -", 0);
	cons_set_raw(1);
	return 0;
}

#define RADARE_OPCODES 12
static const char *radare_opcodes[RADARE_OPCODES]={
	NULL, "call ", "mov ", "nop", "jmp ", "jnz ", "jz ", "ret", "push ", "pop ", "trap", "int "};

CMD_DECL(insert_assembly)
{
	char buf[128];
	char buf2[64];
	const char *dl_prompt_old = dl_prompt;

	if (!config_get("file.write")) {
		eprintf("Sorry, but you're not in read-write mode\n");
		cons_any_key();
		return 1;
	}

	cons_set_raw(0);
	buf[0]='\0';
	strcpy(buf, "wa ");
	dl_prompt = strdup(":> wa ");
	/* TODO: autocomplete opcodes */
	cons_fgets(buf+3, 120, RADARE_OPCODES, radare_opcodes);
//	sprintf(buf2, " @ 0x%llx", config.seek+(config.cursor_mode?config.cursor:0));
//	strcat(buf, buf2);
	if(buf[3]) radare_cmd(buf, 0);
	else eprintf("ignored\n");
	
	cons_set_raw(1);
	free((void *)dl_prompt);
	dl_prompt = dl_prompt_old;

	return 0;
}

#warning XXX: insert_assembly_hack must be accesible without the debugger and scriptable (outsize eip)
CMD_DECL(insert_assembly_hack)
{
#if DEBUGGER
	char buf[16];
	if (config.debug) {
		radare_cmd("!hack", 0);
	} else {
		/* for non debugger -- all of them must be available !! */
		printf(" 0 - nop one opcode\n");
		printf(" 1 - negate jump (jz->jnz , ja->jbe, ..)\n");
		printf(" 2 - force jmp (only for { 0f, 0x80-0x8f })\n");
		printf(" 5 - add ret\n");
		printf(" 6 - add ret with eax=0\n");
		printf(" 7 - negate zero flag (TODO)\n");
	}
	
	buf[0]='\0';
	buf[0]=cons_readchar();
	buf[1]='\0';
	arch_hack(buf);
	CLRSCR();
#endif
	return 0;
}

CMD_DECL(insert_string)
{
	int ret, inc = 0;
	unsigned char key;

	if (!config_get("file.write")) {
		eprintf("Not in write mode\n");
		return 0;
	}

	printf("(press return to cancel)\n write stdin->file: ");
	fflush(stdout);
	radare_seek(config.seek+((config.cursor_mode)?config.cursor:0), SEEK_SET);
	while((ret=read(0, &key, 1)!=-1 && key!='\r')) {
		inc++;
		if (key=='\n') break;
		write(1, &key, 1);
		io_write(config.fd, &key, 1);
		// TODO update bytes in screen? :D
	}
	printf("\n\nWritten %d bytes.\n", inc);
	if (config.cursor_mode)
		radare_seek(config.seek-config.cursor, SEEK_SET);
	return 0;
}

CMD_DECL(insert_hexa_string) // TODO: control file has growed here too!! maybe int _write?
{
	char buf[1025];
	const char *dl_prompt_old = dl_prompt;
	u64 oseek = config.seek;

	if (!config_get("file.write")) {
		eprintf("Not in write mode.\n");
		return 0;
	}

	cons_set_raw(0);
	buf[0]='\0';
	strcpy(buf, "wx ");
	dl_prompt = strdup(":> wx ");
	/* TODO: autocomplete opcodes */
	cons_fgets(buf+3, 1024, 0, NULL);

	if(buf[3]) {
		if (config.cursor_mode && config.ocursor != -1) {
			/* repeat in loop to fill the selected area */
			char *tmp;
			unsigned char out[1024]; // XXX can be overflowed
			int osize;
			int size = config.cursor - config.ocursor+1;
			if (size<0) size=-size;
			tmp = (char *)malloc(size+1);
			osize = hexstr2binstr(buf+3, out);
			memcpy_loop(tmp, out, size, osize);
			radare_seek(oseek + CMPMIN(config.cursor, config.ocursor),SEEK_SET);
			io_write(config.fd, tmp, size);
			free(tmp);
			radare_seek(oseek, SEEK_SET);
		} else {
			radare_cmd(buf, 0);
		}
	}
	
	cons_set_raw(1);
	free((void *)dl_prompt);
	dl_prompt = dl_prompt_old;
	return 0;
}

CMD_DECL(rotate_print_format)
{
	int i;
	for(i=0;i<VMODES;i++)
		if (modes[i] == last_print_format)
			break;

	last_print_format = modes[(i==VMODES)?0:i+1];
	cons_clear();
	return 0;
}

char *get_print_format_name(int j)
{
	int i;
	if (last_print_format == FMT_ZOOM)
		return zoom_string;

	for(i=0;i<VMODES;i++)
		if (modes[i] == j)
			return modestr[i];
	return NULL;
}

CMD_DECL(rotate_print_format_prev)
{
	int i;
	for (i=0;i<VMODES;i++)
		if (modes[i] == last_print_format)
			break;
	last_print_format = modes[(i==VMODES)?0:(i==0)?VMODES-2:i-1];
	cons_clear();
	return 0;
}

static int keystroke_run(unsigned char key) {
	command_t cmd;
	int i = 0;

	for (cmd = keystrokes[0]; cmd.sname ; cmd = keystrokes[i++])
		if (cmd.sname == key) {
			(void)cmd.hook(""); // no args
			if (cmd.options)
				cons_any_key();
			return 1;
		}

	return 0;
}

// XXX Does not support Fx..is this ok?
static void visual_bind_key()
{
#if HAVE_LIB_READLINE
	char *ptr;
#endif
	char key;
	char buf[1024];
	int i, n;

	printf("Press a key to bind (? for listing): ");
	fflush(stdout);
	key = cons_readchar();
	if (!is_printable(key)) {
		printf("\n\nInvalid keystroke\n");
		cons_any_key();
		return;
	}
	printf("%c\n", key);
	fflush(stdout);
	for(i=0;keystrokes[i].sname;i++) {
		if (key == keystrokes[i].sname) {
			printf("\n\nInvalid keystroke (handled by radare)\n");
			cons_any_key();
			return;
		}
	}
	if (key=='?') {
		if (nbds == 0) {
			printf("No user keybindings defined.\n");
		} else {
			printf("Keybindings:");
			NEWLINE;
			for(i=0;i<nbds;i++) {
				printf(" key '%c' = \"%s\"", bds[i].key, bds[i].cmd);
				NEWLINE;
			}
		}
		cons_any_key();
		return;
	}

	cons_printf("\nradare command: ");
	fflush(stdin);
	fflush(stdout);
	cons_flush();
	cons_set_raw(0);
	buf[0]='\0';

#if HAVE_LIB_READLINE
	ptr = readline(VISUAL_PROMPT);
	if (ptr) {
		strcpy(buf, ptr);
		free(ptr);
	}
	//else buf[0]='\0';
	cons_set_raw(1);
	return;
#else
	if (cons_fgets(buf, 1000, 0, NULL) <0)
		buf[0]='\0';
#endif
	cons_set_raw(1);
	if (!buf[0]) {
		printf("ignored!\n");
		return;
	}

	if (is_printable(key)) {
		for(i = 0; i<nbds; i++)
			if (bds[i].key == key) {
				n = i;
				break;
			}

		if (i == nbds)
			bds = (struct binding *)realloc(bds, sizeof(struct binding)*(++nbds));

		bds[n].key = key;
		bds[n].cmd = strdup(buf);
	} 
	cons_flush();
}

void visual_draw_screen()
{
	const char *ptr;
	char buf[256];
	char buf2[256];

	/* printage */
	switch (last_print_format) {
	case FMT_HEXB:
	case FMT_BIN:
	case FMT_CSTR:
	case FMT_OCT:
		//if (config.size!=-1 && config.seek+config.block_size> config.size)
		//	printf("\x1b[2J\x1b[0;0H");
		break;
	default:
		if (!config.cursor_mode)
			cons_clear();
	}
	switch(config.insert_mode) {
	case 1:
		strcpy(buf, "<insert hex pairs> ('q','tab')");
		break;
	case 2:
		strcpy(buf, "<insert ascii> ('tab')");
		break;
	case 3:
		strcpy(buf, "<insert assembly> ('tab')");
		break;
	default:
		string_flag_offset(buf, config.seek);
		if (config.cursor!=-1)
			string_flag_offset(buf2, config.seek+config.cursor);
	}
	ptr = config_get("scr.seek");
	if (ptr&&ptr[0]&&last_print_format==FMT_REF) {
		u64 off = get_math(ptr);
		if (off != 0)
		radare_seek(off, SEEK_SET);
	}

	monitors_run();

	/* HUH */
	cons_clear00();
	V cons_printf("\x1b[0m");

	if (inc >config.block_size)
		inc = config.block_size;

	if (config.cursor_mode)
	cons_printf("[ 0x%llx (inc=%d, bs=%d cur=%d sz=%d mark=0x%llx) %s %s] %s -> %s         \n",
		(config.seek+config.baddr), inc,
		(unsigned int)config.block_size,
		config.cursor, (config.ocursor==-1)?0:config.cursor-config.ocursor+1,
		mark, get_print_format_name(last_print_format),
		(inv)?"inv ":"", buf, buf2);
	else
	cons_printf("[ 0x%llx (inc=%d, bs=%d sz=%d mark=0x%llx) %s %s] %s            \n",
		(config.seek+config.baddr), inc,
		(unsigned int)config.block_size,
		(config.ocursor==-1)?0:config.cursor-config.ocursor+1,
		mark, get_print_format_name(last_print_format),
		(inv)?"inv ":"", buf);

	/* Spaguetti monster lives here ! */
	ptr = config_get("cmd.vprompt");
	if (ptr&&ptr[0]) {
		int tmp = last_print_format;
		radare_cmd_raw(ptr, 0);
		last_print_format = tmp;
	}
	ptr = config_get("cmd.vprompt2");
	if (ptr&&ptr[0]) {
		int tmp = last_print_format;
		radare_cmd_raw(ptr, 0);
		last_print_format = tmp;
	}
	ptr = config_get("cmd.vprompt3");
	if (ptr&&ptr[0]) {
		int tmp = last_print_format;
		radare_cmd_raw(ptr, 0);
		last_print_format = tmp;
	}

	radare_seek(config.seek, SEEK_SET);
	radare_print("", last_print_format);

	fflush(stdout);
	cons_flush();
}

// autoadjust height of screen
static void ringring()
{
	int h   = config.height;
	int now = cons_get_columns();

	// TODO: use config.width here
	if (!getenv("COLUMNS"))
	if (now!=config.width || h!=config.height ) {
		config.width = now;
		CLRSCR();
		visual_draw_screen();
	}
#if __UNIX__
	go_alarm(ringring);
#endif
}

static void check_accel(int foo)
{
#if _UNIX_
	static suseconds_t ulast = 0;
	static time_t last = 0;
	struct timeval tv;
	static int counter = 0;
	if (!scraccel)
		return;
	if (!foo) {
		last = 0;
		return;
	}
	gettimeofday(&tv, NULL);
	if (last == 0) {
		last = tv.tv_sec;
		ulast = tv.tv_usec;
	} else {
		if (last+2>tv.tv_sec) {
			counter++;
			if (counter >= accel*scraccel)
				accel ++;
		} else accel = 1;
	}
	last = tv.tv_sec;
	ulast = tv.tv_usec;
#endif
}

void visual_f(int f)
{
	u64 addr;
	struct bp_t *bp;
	char line[128];

	switch(f) {
		case 1:
			cons_clear();
			radare_cmd("!help", 0);
			cons_any_key();
			cons_clear();
			break;
#if DEBUGGER
		case 2:
			addr = config.seek + (config.cursor_mode?config.cursor:0);
			bp = debug_bp_get(addr);
			if (bp) {
				sprintf(line, "!bp -0x%08x", (unsigned int)addr);
				radare_cmd(line,0);
				//debug_rm_bp(addr, 0);
			} else {
				sprintf(line, "!bp 0x%08x", (unsigned int)addr);
				radare_cmd(line,0);
			}
			if (!debug_bp_get(addr))
				flag_clear_by_addr(addr);
			cons_clear();
			break;
		case 3:
			cons_set_raw(0);
			printf("Watchpoint at: ");
			strcpy(line, "!drw ");
			fgets(line+5, sizeof(line), stdin);
			line[strlen(line)-1]='\0';
			radare_cmd(line,0);
			cons_set_raw(1);
			cons_any_key();
			cons_clear();
			break;
#endif
		case 4:
			radare_cmd("!contuh", 0);
			cons_clear();
			break;
#if DEBUGGER
		case 5:
			arch_jmp(config.seek + (config.cursor_mode?config.cursor:0));
			break;
#endif
		case 6:
			radare_cmd("!contsc", 0);
			break;
		case 7:
			if (config_get("trace.libs")) {
				//CMD_NAME(step)(NULL);
				debug_step(1);
			} else {
				CMD_NAME(stepu_in_dbg)(NULL);
			}
			break;
		case 8:
			if (config.debug)
				CMD_NAME(stepo_in_dbg)(NULL);
			break;
	}
}

CMD_DECL(visual)
{
	unsigned char key;
	int i, lpf;
	int nibble;
	char line[1024];
#if HAVE_LIB_READLINE
	char *ptr;
#endif

	switch(input[0]) {
	case 'g':
	case 'G':
		eprintf("Visual GUI\n");
		visual_gui();
		return;
	}

	cons_get_real_columns();
	config_set_i("scr.width", config.width);
	config_set_i("scr.height", config.height);

	nibble = 1; // high first

	undo_push();

#if 0
	config.height = config_get_i("scr.height");
	if (config.height<1)
		config_set_i("scr.height", 24);
#endif

	if (bds == NULL)
		bds = (struct binding *)malloc(sizeof(struct binding));

	unsetenv("COLUMNS");
#if __UNIX__
	go_alarm(ringring);
#endif
	config.visual = 1;

	cons_clear();
	cons_set_raw(1);
	while(1) {
		if (inc<1) inc = 1;
		dec = inc;
		setenv("VISUAL", "1", 1);
		scraccel = config_get_i("scr.accel");
		update_environment();
		radare_sync();
		if (config.debug)
			radare_cmd(".!regs*", 0);
		radare_prompt_command();
		visual_draw_screen();

		if (last_print_format == FMT_UDIS) {
			const char *follow = config_get("asm.follow");
			if (follow&&follow[0]) {
				u64 addr = get_offset(follow);
				if ((addr < config.seek) || ((config.seek+config.block_size)<addr))
					radare_seek(addr, SEEK_SET);
			}
		}

	__go_read_a_key:
		/* user input */
		key = cons_readchar();

		/* insert mode . 'i' key */
		switch(config.insert_mode) {
		case 1:
			key = cons_get_arrow(key); // get ESC+char, return 'hjkl' char
#if 0
			if (key==0x1b) {
				key = cons_readchar();
				if (key==0x5b) {
					// TODO: must also work in interactive visual write ascii mode
					key = cons_readchar();
					switch(key) {
					case 0x35: key='K'; break; // re.pag
					case 0x36: key='J'; break; // av.pag
					case 0x41: key='k'; break; // up
					case 0x42: key='j'; break; // down
					case 0x43: key='l'; break; // right
					case 0x44: key='h'; break; // left
					case 0x3b:
						break;
					default:
						key = 0;
					}
				}
			}
#endif
			switch(key) {
			case 9: // TAB
				if (last_print_format == FMT_DISAS
					|| last_print_format == FMT_UDIS
					|| last_print_format == FMT_VISUAL)
					config.insert_mode = 3;
				else
				config.insert_mode = 2;
				nibble=1;
				break;
			case 'q':
			case 0x1b: // ESC
				config.insert_mode = 0;
				config.cursor_mode = 0;
				cons_clear();
				break;
			case '*': radare_set_block_size_i(config.block_size+inc); cons_clear(); break;
			case '/': radare_set_block_size_i(config.block_size-inc); cons_clear(); break;
			case '+': radare_set_block_size_i(config.block_size+1); break;
			case '-': radare_set_block_size_i(config.block_size-1); cons_clear(); break;
			case 'p': CMD_NAME(rotate_print_format)(""); cons_clear();  break;
			case 'P': CMD_NAME(rotate_print_format_prev)(""); cons_clear(); break;
			case 'H': config.seek -= 1; cons_clear(); break;
			case 'L': config.seek += 1; cons_clear(); break;
			case 'J': config.seek += config.block_size*accel/2; cons_clear(); break;
			case 'K': config.seek -= config.block_size*accel/2; cons_clear(); break;
			case 'h':
				if (config.cursor--<1)
					config.cursor = 0;
				nibble=1;
				break;
			case 'j':
				config.cursor+=inc;
				if (config.cursor>=config.block_size)
					config.cursor = config.block_size - 1;
				nibble=1;
				break;
			case 'k':
				if (((int)(config.cursor-inc))<0)
					config.cursor = 0;
				else config.cursor-=inc;
				nibble=1;
				break;
			case 'l':
				if (++config.cursor>=config.block_size)
					config.cursor = config.block_size - 1;
				nibble=1;
				break;
			default:
				if ((key>='0'&&key<='9')
				||  (key>='a'&&key<='f')
				||  (key>='A'&&key<='F')) {
					int lol = 0;
					unsigned char str[2] = { key, 0};
					sscanf(str,"%1x", &lol);
					radare_read_at(config.seek+config.cursor, str, 1);
					str[0] &= (nibble)?0x0f:0xf0;
					str[0] |= lol<<(nibble*4);
					radare_write_at(config.seek+config.cursor, str, 1);
					
					// TODO WRITE CHAR HERE
					if (nibble) {
						nibble = 0;
					} else {
						config.cursor++;
						nibble = 1;
					}
				}
				break;
			}
			if (config.cursor<0)
				config.cursor = 0;
			continue;
		case 2: // insert scii
			nibble=1;
			switch(key) {
			case 9:
				config.insert_mode = 1;
				break;
			case 0x1b:
				cons_readchar();
				switch(cons_readchar()) {
				case 0x04:
					config.insert_mode = 0;
					config.cursor_mode = 0;
					cons_clear();
					break;
				case 0x44: //'h':
					if (--config.cursor<0)
						config.cursor = 0;
					break;
				case 0x42: //'j':
					config.cursor+=inc;
					if (config.cursor>=config.block_size)
						config.cursor = config.block_size - 1;
					break;
				case 0x41: // 'k':
					if (((int)(config.cursor-inc))<0)
						config.cursor = 0;
					else config.cursor-=inc;
					break;
				case 0x43: // 'l':
					if (++config.cursor>=config.block_size)
						config.cursor = config.block_size - 1;
					break;
				}
				break;
			default:
				radare_write_at(config.seek+config.cursor,&key,1);
				config.cursor++;
				if (config.cursor>=config.block_size)
					config.cursor = config.block_size - 1;
				break;
			}
			continue;
		case 3: // insert asm
			case 9:
				config.insert_mode = 1;
				continue;
			break;
		}

		if (key == '~') // skip ignored char
			key = cons_readchar();

		/* normal visual mode */
		switch(key) {
		case 0x1b: // ESC
			key = cons_readchar();
			switch(key) {
			case 0x4f: // Fx
				key = cons_readchar() - 0x4f;
				if (config.debug)
				switch(key) {
				case 1: // F1 -help
					visual_f(1);
					continue;
				#if DEBUGGER
				case 2: // F2 - breakpoint
					visual_f(2);
					continue;
				case 3: // F3 - watchpoint
					visual_f(3);
					continue;
				case 4: // F4 - watchpoint
					visual_f(4);
					continue;
				#endif
				}
				break;
			case 0x1b: // DOUBLE ESC
				key='q';
				break;
			case '0':
			case '[':
				key = cons_readchar();
				switch(key) {
				case '[':
					key = cons_readchar();
					//cons_readchar();
					switch(key) {
					case 'A': visual_f(1); break;
					case 'B': visual_f(2); break;
					case 'C': visual_f(3); break;
					case 'D': visual_f(4); break;
					}
					
					break;
				case 0x35: key='K'; break; // re.pag
				case 0x36: key='J'; break; // av.pag
				case 0x41: key='k'; break; // up
				case 0x42: key='j'; break; // down
				case 0x43: key='l'; break; // right
				case 0x44: key='h'; break; // left
				case '1':
					key = cons_readchar(); // Read dummy '~'
					switch(key) {
					case ';': // arrow + shift
						key = cons_readchar();
						key = cons_readchar();
						switch(key) {
						case 'A': //Up
							key = 'K';
							break;
						case 'B': //down
							key = 'J';
							break;
						case 'C': //right
							key = 'L';
							break;
						case 'D': //left
							key = 'H';
							break;
						}
						break;
					case '1': // F1
						visual_f(1);
						continue;
					case '2': // F2
						visual_f(2);
						continue;
					case '3': // F3 - watchpoint
						visual_f(3);
						continue;
					case '4': // contuh
						visual_f(4);
						continue;
					case '5': // F5 - set eip
						visual_f(5);
						continue;
					case '7': // F6
						visual_f(6);
						continue;
					case '8': // F7
						visual_f(7);
						continue;
					case '9': // F8
						visual_f(8);
						continue;
					}
					break;
				case '2':
					key = cons_readchar();
					cons_readchar(); // Read dummy '~'
					switch(key) {
					case 50:
						key = cons_readchar();
						switch(key) {
						case 65: key='K'; break;
						case 66: key='J'; break;
						case 67: key='L'; break;
						case 68: key='H'; break;
						}
						break;
					case '0': // F9
						if (config.debug)
							radare_cmd("!cont", 0);
						continue;
					case '~': // F10 // XXX this is not ok?!?
						D printf("Walking until user code...\n");
						radare_cmd("!contu", 0);
						continue;
					case 49: // F10
						radare_cmd("!contu", 0);
						continue;
					default:
						printf("50 unknown key %d\n", key);
						key = cons_readchar();
					}
					break;
				default:
					printf("0x5b unknown key 0x%x\n", key);
					key = cons_readchar();
					break;
				}
				break;
			default:
				// TODO: HANDLE ESC KEY HERE
				cons_readchar();
				for(i=0;i<nbds;i++) {
					if (bds[i].key == key) {
						radare_cmd(bds[i].cmd, 0);
						break;
					}
				}
				printf("27 unknown key 0x%x %c\n", key, key);
				continue;
			}
			break;
		case 0x7f:
			key = cons_readchar();
			if (key >='0' && key <= '9')  {
				if (config.size != -1) {
					int pc = key-'0';
					radare_seek(config.size*pc/10, SEEK_SET);
				}
			}
			break;
		}

		/* command repeat */
		if (key>'0'&&key<='9') {
			if (do_repeat) {
				repeat*=10;
				repeat+=(key-'0');
				continue;
			} else {
				undo_push();
				udis_jump(key-'0'-1);
				last_print_format = FMT_UDIS;
				continue;
			}
		}

		if (inc<1)inc=1; // XXX dup check?
		if (repeat==1)repeat=0;
		//else repeat-=1;
		repeat=1;
		for (i=0;i<repeat;i++)
		switch(key) {
		case 'r':
			repeat--;
			do_repeat =1;
			break;
		case ':':
			cons_set_raw(0);
			lpf = last_print_format;
			config.visual=0;
#if HAVE_LIB_READLINE
			ptr = readline(VISUAL_PROMPT);
			if (ptr) {
				strncpy(line, ptr, sizeof(line));
				radare_cmd(line, 1);
				//commands_parse(line);
				free(ptr);
			}
#else
			line[0]='\0';
			dl_prompt = ":> ";
			if (cons_fgets(line, 1000, 0, NULL) <0)
				line[0]='\0';
			//line[strlen(line)-1]='\0';
			radare_cmd(line, 1);
#endif
			config.visual=1;
			last_print_format = lpf;
			cons_set_raw(1);
			if (line[0])
				cons_any_key();
			cons_gotoxy(0,0);
			cons_clear();
			continue;
		case ',':
			if (config.seek == mark) {
				mark = 0;
			} else {
				mark = config.seek + ((config.cursor_mode)?config.cursor:0);
			}
			break;
		case '.':
			// TODO: WHAT IS THIS DOING? I THINK '.' is better for seek to (nice with cursor)
			if (config.cursor_mode) {
				undo_push();
				radare_seek(config.seek + ((config.cursor_mode)?config.cursor:0), SEEK_SET);
				config.cursor = 0;
			} else {
				if (mark==0) {
					u64 u = get_offset("eip");	
					if (u!=0) {
						undo_push();
						radare_seek(u, SEEK_SET);
					}
				} else {
					undo_push();
					radare_seek(mark, SEEK_SET);
				}
			}
			break;
#if 0
		case 'b':
			visual_bind_key();
			break;
#endif
		case 'm':
			printf("\nrfile magic:\n\n");
			radare_dump_and_process( DUMP_MAGIC, config.block_size);
			cons_any_key();
			break;
		case 'd':
			visual_convert_bytes(-1);
			break;
		case 'C':
			config.color^=1;
			sprintf(line, "%d", config.color);
			config_set("scr.color", line);
			break;
		case 'c':
			config.cursor_mode ^= 1;
			break;
		case 'n':
			radare_search_seek_hit(+1);
			break;
		case 'N':
			radare_search_seek_hit(-1);
			break;
		case 'h':
			if (config.cursor_mode) {
				config.cursor --;
				if (cursorseek && IS_LTZ(config.cursor)) {
					inc = 1;
					radare_seek(config.seek-1, SEEK_SET);
				}
				if (IS_LTZ(config.cursor))
					config.cursor =0;
				config.ocursor = -1;
			} else {
				config.seek--;
				if (config.seek<0) config.seek=0;
			}
			break;
		case ']':
		case '[': {
			int cols = config.width;
			if (cols < 10) cols = 10;
			config_set_i("scr.width", cols+(key==']'?+4:-4));
			cons_clear();
			break; }
		case 'L':
			if (config.cursor_mode) {
				if (config.ocursor==-1)
					config.ocursor = config.cursor;
				config.cursor ++;
				if (cursorseek && config.cursor >= config.block_size) {
inc = 1;
					radare_seek(config.seek+inc, SEEK_SET);
					config.cursor-=inc;
					if (config.ocursor != -1)
						config.ocursor-=inc;
				}
				if (config.cursor >= config.block_size)
					config.cursor = config.block_size - 1;
			} else
				config.seek += 2;
			break;
		case 0xd:
		case 'j':
			if (config.cursor_mode) {
				config.cursor += inc;
				if (cursorseek && config.cursor >= config.block_size) {
					radare_seek(config.seek+inc, SEEK_SET);
					config.cursor-=inc;
					if (config.ocursor != -1)
						config.ocursor-=inc;
				}
				if (config.cursor >= config.block_size)
					config.cursor = config.block_size - 1;
				config.ocursor = -1;
			} else {
				check_accel(0);
				config.seek += inc;
				if (config.block_size >= (config.size-config.seek))
					cons_clear();
			}
			break;
		case ' ':
		case 'J':
			if (config.cursor_mode) {
				if (config.ocursor==-1)
					config.ocursor = config.cursor;
				config.cursor += inc;
				if (cursorseek && config.cursor >= config.block_size) {
					radare_seek(config.seek+inc, SEEK_SET);
					config.cursor-=inc;
					if (config.ocursor != -1)
						config.ocursor-=inc;
				}
				if (config.cursor >= config.block_size)
					config.cursor = config.block_size - 1;
				if (config.block_size >= (config.size-config.seek))
					cons_clear();
				continue;
			} else {
				check_accel(1);
				if (last_print_format == FMT_DISAS)
					config.seek += inc;
				else	config.seek += config.block_size*accel;
				cons_clear();
			}
			break;
		case 0x8:
		case 'k':
			check_accel(0);
			if (config.cursor_mode) {
				config.cursor-=dec;
				if (cursorseek && IS_LTZ(config.cursor)) {
					radare_seek(config.seek-dec, SEEK_SET);
				}
				if (IS_LTZ(config.cursor))
					config.cursor = 0;
				config.ocursor = -1;
			} else {
				config.seek -= dec;
				if (config.block_size >= (config.size-config.seek))
					cons_clear();
			}
			break;
		case 'K':
			if (config.cursor_mode) {
				if (config.ocursor==-1)
					config.ocursor = config.cursor;
				config.cursor-=dec;
				if (cursorseek && IS_LTZ(config.cursor)) {
					radare_seek(config.seek-dec, SEEK_SET);
					config.ocursor+=dec+1;
				}
				if (IS_LTZ(config.cursor))
					config.cursor = 0;
			} else {
				check_accel(1);
				if (last_print_format == FMT_DISAS)
					config.seek -= 4;
				else	config.seek -= config.block_size*accel;
				cons_clear();
			}
			if (config.block_size >= (config.size-config.seek))
				cons_clear();
			break;
		case 'u':
			undo_seek();
			break;
		case 'U':
			undo_redo();
			break;
		case 'f':
			if (config.cursor_mode) {
				char name[1024];
				eprintf("Flag name: ");
				fflush(stderr);
				cons_set_raw(0);
				cons_fgets(name, 1000, 0, NULL);
				cons_set_raw(1);
				if (name[0])
					flag_set(name, config.baddr+ config.seek+config.cursor, 1);
			} else {
				flag_t *flag = flag_get_next(1);
				if (flag) { config.seek = flag->offset;
					cons_clear(); }
			}
			break;
		case 'F': 
			if (config.cursor_mode) {
				flag_clear_by_addr(config.seek+config.cursor);
			} else { flag_t *flag = flag_get_next(-1);
				if (!flag) flag = flag_get_reset();
				if (flag) { config.seek = flag->offset;
					cons_clear(); }
			}
			break;
		case 'e':
			config_visual_menu();
			break;
		case 't':
			flags_visual_menu();
			break;
		case '<':
			// fold
			if (config.cursor_mode) {
				// one byte = 
				if (config.ocursor == -1) {
					// check if current cursor seek is a function or expand
					// unexpand function or close folder
					int type = data_type_range(config.seek+config.cursor);
					if (type == -1 || type == DATA_FOLD_O) {
						data_set((u64)(config.seek+config.cursor), DATA_FOLD_C);
						cons_clear();
					}
				} else {
					visual_convert_bytes(DATA_FOLD_C);
				}
			} else {
				config.seek-= config.block_size;
				if (config.seek % config.block_size)
					cmd_next_align("");
			}
			break;
		case '>':
			if (config.cursor_mode) {
				int type = data_type_range(config.seek+config.cursor);
				if (type == DATA_FOLD_C) {
					data_set(config.seek+config.cursor, DATA_FOLD_O);
				}
				cons_clear();
				
				// unfold or expand
				// check if current cursor position is jump -> open it
				// check if current cursor position is a folder -> open it
			} else {
				config.seek += config.block_size - (config.seek % config.block_size);
			}
			break;
		case 'H':
			if (config.cursor_mode) {
				config.cursor--;
				if (cursorseek && IS_LTZ(config.cursor)) {
					inc = 1;
					radare_seek(config.seek-1, SEEK_SET);
					config.ocursor++;
				}
				if (config.ocursor==-1)
					config.ocursor = config.cursor+1;
				if (IS_LTZ(config.cursor))
					config.cursor =0;
			} else
				config.seek -= 2;
			break;
		case 'l':
			if (config.cursor_mode) {
				if (++config.cursor>=config.block_size)
					config.cursor = config.block_size - 1;
				config.ocursor = -1;
			} else
				config.seek++;
			break;
		case '*':
			radare_set_block_size_i(config.block_size+inc);
			break;
		case '/':
			radare_set_block_size_i(config.block_size-inc);
			cons_clear00();
			break;
		case '+':
			if (config.cursor_mode) {
				char buf[128];
				int c, ch;
				if (config.cursor>=0 && config.cursor< config.block_size) {
					c = config.cursor;
					ch = config.block[config.cursor];
					sprintf(buf, "wx %02x @ 0x%llx", (unsigned char)(++ch), config.seek); //+config.cursor);
					radare_cmd(buf, 0);
					config.cursor = c;
					config.ocursor = -1;
					cons_clear00();
				}
			} else
				radare_set_block_size_i(config.block_size+1);
			break;
		case '-':
			if (config.cursor_mode) {
				char buf[128];
				int c, ch;
				if (config.cursor>=0 && config.cursor< config.block_size) {
					c = config.cursor;
					ch = config.block[config.cursor];
					eprintf("cursor %d\n", config.cursor);
					sprintf(buf, "wx %02x @ 0x%llx", (unsigned char)(--ch), config.seek); //+config.cursor);
					radare_cmd(buf, 0);
					config.cursor = c;
					config.ocursor = -1;
				}
			} else radare_set_block_size_i(config.block_size-1);
			cons_clear();
			break;
		case '!':
			cons_clear00();
			radare_cmd("!help", 0);
			cons_any_key();
			break;
		case '?':
			cons_clear00();
			visual_show_help();
			cons_any_key();
			cons_clear();
			break;
		case 'Q':
		case 'q':
			setenv("VISUAL", "0", 1);
#if __UNIX__
			go_alarm(SIG_IGN);
#endif
			goto __visual_exit;
		default:
			if (!keystroke_run(key)) {
				int i;
				for(i=0;i<nbds;i++)
					if (bds[i].key == key)
						radare_cmd(bds[i].cmd, 0);
				goto __go_read_a_key;
			}
		}

		if (config.seek<0)
			config.seek = 0;
		do_repeat = 0;
		repeat = 0;
		repeat_weight=1;
	}

__visual_exit:
	config.visual = 0;
	config.cursor_mode = 0;
	cons_set_raw(0);
}
