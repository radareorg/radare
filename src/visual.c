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
#include "utils.h"
#include "cmds.h"
#include "dbg/arch/arch.h"
#include "readline.h"
#include "flags.h"
#include "undo.h"

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
CMD_DECL(add_comment);
CMD_DECL(show_environ);
CMD_DECL(edit_comment);
CMD_DECL(yank);
CMD_DECL(yank_paste);
CMD_DECL(zoom);
CMD_DECL(trace);
CMD_DECL(zoom_reset);

#if 0
make keybindings be structured like:
 int char
 int len
 char rest[5]
 void *callback
 int id
#endif

command_t keystrokes[] = {
	/*   key, wait, description, callback */
	COMMAND('g', 0, "seek to offset 0", seek0),
	COMMAND('G', 0, "seek to end of file", seek_to_end),
	COMMAND(';', 0, "add a comment", add_comment),
	COMMAND('p', 0, "change to next print format", rotate_print_format),
	COMMAND('P', 0, "change to previous print format", rotate_print_format_prev),
	COMMAND('a', 0, "insert assembly", insert_assembly),
	COMMAND('t', 0, "simulate trace cursor position", trace),
	COMMAND('A', 0, "insert assembly", insert_assembly_rsc),
	COMMAND('y', 0, "yank (copy selected block to clipboard)", yank),
	COMMAND('Y', 0, "Yankee (paste clipboard here)", yank_paste),
	COMMAND('=', 0, "insert assembly hack", insert_assembly_hack),
	COMMAND('x', 1, "show xrefs of the current offset", xrefs_here),
	COMMAND('i', 1, "show information", status),
	COMMAND('I', 0, "invert current block", invert),
	COMMAND('z', 0, "zoom full/block with pO", zoom),
	COMMAND('Z', 0, "resets zoom preferences", zoom_reset),
	COMMAND('w', 1, "write string", insert_string),
	COMMAND('W', 1, "write hex string", insert_hexa_string),
	//COMMAND('f', 0, "seek to flag", seek_to_flag),
	COMMAND('%', 0, "show radare environment", show_environ),
	/* debugger */
        COMMAND('s', 0, "step into the debugger", step_in_dbg),
        COMMAND('S', 0, "step over the debugger", stepo_in_dbg),
	COMMAND('\0',0, NULL, seek_to_flag)
};

struct binding {
	unsigned char key;
	char *cmd;
};
static int nbds = 0;
static struct binding *bds = NULL;
static int inv = 0;

void press_any_key()
{
	int key;

	D cons_printf("\n--press any key--\n");
	cons_flush();
	cons_readchar();
	cons_strcat("\e[2J\e[0;0H");
}

CMD_DECL(invert)
{
	if (inv)
		inv = 0;
	else	inv = FMT_INV;
}

CMD_DECL(show_environ)
{
	CLRSCR();
	radare_cmd("%", 0);
	press_any_key();
	CLRSCR();
}

CMD_DECL(zoom_reset)
{
	config.zoom.from = 0;
	config.zoom.size = config.size;
	config.zoom.piece = config.zoom.size/config.block_size;
}

CMD_DECL(trace)
{
	if (config.cursor_mode)
		trace_add(config.seek+config.cursor);
	else	trace_add(get_offset("eip"));
}

CMD_DECL(zoom)
{
	if (config.zoom.size = 0)
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
}

CMD_DECL(add_comment)
{
	int n;
	char buf[300];
	printf("Comment: ");
	fflush(stdout);
	strcpy(buf, "C ");
	terminal_set_raw(0);
	n = read(0, buf+2, 256);
	buf[n+1]='\0';
	if (config.cursor_mode) {
		char ptr[128];
		sprintf(ptr, " @ +0x%x", config.cursor);
		strcat(buf, ptr);
	}
	terminal_set_raw(1);
	radare_cmd(buf,0);
	CLRSCR();
}

CMD_DECL(seek0)
{
	if (config.cursor_mode)
		config.cursor = 0;
	else	config.seek = 0;
}

unsigned char *yank_buffer = NULL;
int yank_buffer_size = 0;

CMD_DECL(yank)
{
	int off = 0;
	char *ptr = strchr(input, ' ');
	if (ptr == NULL)
		ptr = input;
	if (ptr[0]=='y') {
		cmd_yank_paste(input);
		return;
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
	D fprintf(stderr, "%d bytes yanked. off=%d data=%02x %02x %02x...\n", yank_buffer_size,
		off, yank_buffer[0], yank_buffer[1], yank_buffer[2]);

	if (config.visual) {
		press_any_key();
		CLRSCR();
	}
}

CMD_DECL(yank_paste)
{
	if (yank_buffer_size == 0) {
		fprintf(stderr, "No buffer yanked\n");
		if (config.visual) {
			press_any_key();
			CLRSCR();
		}
	} else {
		if (config_get("cfg.write")) {
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
			fprintf(stderr, "%d bytes yanked.\n", (int) sz);
			radare_seek(old, SEEK_SET);
			radare_read(0);
		} else {
			printf("Not in write mode\n");
			if (config.visual) {
				press_any_key();
				CLRSCR();
			}
		}
	}
}

CMD_DECL(xrefs_here)
{
	char buf[4096];
	snprintf(buf, 4095, "%s %s %d",
		config_get("asm.xrefs"), config.file, (unsigned int)config.seek);
	io_system(buf);
}

CMD_DECL(stepu_in_dbg)
{
	if (!config.debug) {
		eprintf("not in debugger\n");
		press_any_key();
	} else {
		eprintf("Stepping to user code. wait a while...\n");
		//eprintf("TODO: should take care about the backtrace and use...\n");
		fflush(stderr);
		radare_cmd("!stepu", 0);
	}
	radare_sync();
}

CMD_DECL(step_in_dbg)
{
	if (!config.debug) {
		eprintf("not in debugger\n");
		press_any_key();
	} else
		radare_cmd("!step", 0);
	radare_sync();
	//trace_add(get_offset("eip"));
}

CMD_DECL(stepo_in_dbg)
{
	if (!config.debug) {
		eprintf("not in debugger\n");
		press_any_key();
	} else
		radare_cmd("!stepo", 0);
	radare_sync();
}

CMD_DECL(seek_to_flag)
{
	rad_flag_t *flg;
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
}

CMD_DECL(seek_to_end)
{
	if (config.cursor_mode)
		config.cursor = config.block_size - 1;
	else	config.seek = config.size - config.block_size;
}

CMD_DECL(insert_assembly_rsc)
{
	if (!config_get("cfg.write")) {
		eprintf("Sorry, but you're not in read-write mode\n");
		press_any_key();
		return;
	}

	printf("write assembly (end with ^d):\n");
	fflush(stdout);
	terminal_set_raw(0);
	radare_cmd("wA -", 0);
	terminal_set_raw(1);
}

CMD_DECL(insert_assembly)
{
	char buf[129];

	if (!config_get("cfg.write")) {
		fprintf(stderr, "Sorry, but you're not in read-write mode\n");
		press_any_key();
		return;
	}

	terminal_set_raw(0);
	printf(":> wa ");
	fflush(stdout);
	buf[0]='\0';
	strcpy(buf, "wa ");
	fgets(buf+3, 120, stdin);
	if(buf[3]) {
		buf[strlen(buf)-1]='\0';
		radare_cmd(buf, 0);
	} else {
		eprintf("ignored\n");
	}
	
	terminal_set_raw(1);
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
}

CMD_DECL(insert_string)
{
	int ret, inc = 0;
	unsigned char key;

	if (!config_get("cfg.write")) {
		eprintf("Not in write mode\n");
		return;
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
}

CMD_DECL(insert_hexa_string) // TODO: control file has growed here too!! maybe int _write?
{
	int ret, inc = 0;
	unsigned char key;
	unsigned char byte;
	int bytes = 0;
	int count = 0;
	int tmp = 0;

	if (!config_get("cfg.write")) {
		eprintf("Not in write mode.\n");
		return;
	}

	printf("write hexa string: ");
	fflush(stdout);
	radare_seek(config.seek+((config.cursor_mode)?config.cursor:0), SEEK_SET);

	while((ret=read(0, &key, 1)!=-1 && key!='\r')) {
		inc++;
		if (key=='\n') break;
		if (key==' ') continue;
		if (hex2int((unsigned char *)&tmp ,key)) {
			printf("Invalid hex character at byte %d\n", inc);
			break;
		}
		switch(count++) {
		case 0:
			byte = tmp;
			printf("%x", tmp);
			fflush(stdout);
			continue;
		case 1:
			byte<<=4;
			byte |= tmp;
			count = 0;
		}
		printf("%01x(%c) ", tmp, is_printable(tmp)?tmp:'.');
		bytes++;
		fflush(stdout);
		write(config.fd, &byte, 1);
	}

	printf("\n\nWritten %d bytes.\n", bytes);
	if (config.cursor_mode)
		radare_seek(config.seek-config.cursor, SEEK_SET);
}


#define VMODES 12
static char modes[VMODES] =
{ FMT_HEXB, FMT_VISUAL, FMT_UDIS, FMT_OCT, FMT_CSTR,
  FMT_BIN, FMT_URLE, FMT_ASC, FMT_ASHC, FMT_HEXB, 0 };

static char *modestr[VMODES] =
{ "hexb", "visual", "disasm", "octal", "cstr",
  "binary", "url-encoding", "ascii", "ashc", "hexb" };
static char *zoom_string="zoom";

CMD_DECL(rotate_print_format)
{
	int i;
	for(i=0;i<VMODES;i++)
		if (modes[i] == last_print_format)
			break;

	last_print_format = modes[(i==VMODES)?0:i+1];
	cons_clear();
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
}

int keystroke_run(unsigned char key) {
	command_t cmd;
	int i = 0;

	for (cmd = keystrokes[0]; cmd.sname ; cmd = keystrokes[i++])
		if (cmd.sname == key) {
			(void)cmd.hook(""); // no args
			if (cmd.options)
				press_any_key();
			return 1;
		}

	return 0;
}

#define TITLE if (config.color) cons_printf("\e[36m");
#define TITLE_END if (config.color) cons_printf("\e[0m");

void visual_show_help()
{
#warning TODO: use 0123456789 as % places of the file ? :D that looks cool, or predefined seekz
	cons_strcat("\e[2J\e[0;0H\n");
	TITLE
	cons_printf("Visual keybindings:\n");
	TITLE_END
	cons_printf(
	":          radare command (vi like)\n"
	";          edit or add comment\n"
	"g,G        seek to beggining or end of file\n"
	"<>         seek block aligned\n"
	"+-*/       +1, -1, +width, -width -> block size\n"
	"[]         decrease or increment the width limits\n"
	"a,A,=      insert patch assembly, rsc asm or !hack\n"
	"w,W        insert string (w) or hexpair string (W)\n"
	"f #        seek to search result 0-9\n"
	"c          toggle cursor mode\n"
	"C          toggle color\n"
	"d          disassemble current block (pd)\n"
	"b[k][cmd]  binds key 'k' to the specified command\n"
	"D          delete current flag\n"
	"m          applies rfile magic on this block\n"
	"I          invert block (same as pIx or so)\n"
	"y,Y        yank and Yankee aliases for copy and paste\n"
	"f,F        go next, previous flag\n"
	"h,j,k,l    scroll view to left, down, up, right.\n"
	"J,K        up down scroll one block.\n"
	"H,L        scroll left, right by 2 bytes (16 bits).\n"
	"p,P        switch between hex, bin and string formats\n"
	"x          show xrefs of the current offset\n"
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

// XXX Does not support Fx..is this ok?
void visual_bind_key()
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
		fprintf(stderr, "\n\nInvalid keystroke\n");
		press_any_key();
		return;
	}
	printf("%c\n", key);
	fflush(stdout);
	for(i=0;keystrokes[i].sname;i++) {
		if (key == keystrokes[i].sname) {
			fprintf(stderr,"\n\nInvalid keystroke (handled by radare)\n");
			press_any_key();
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
		press_any_key();
		return;
	}

	cons_printf("\nradare command: ");
	fflush(stdin);
	fflush(stdout);
	cons_flush();
	terminal_set_raw(0);
	buf[0]='\0';

#if HAVE_LIB_READLINE
	ptr = readline(VISUAL_PROMPT);
	if (ptr) {
		strcpy(buf, ptr);
		free(ptr);
	}
	//else buf[0]='\0';
return 0;
#else
	buf[0]='\0';
	fgets(buf, 1000, stdin);
	buf[strlen(buf)-1] = '\0';
#endif
	terminal_set_raw(1);
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
	char *ptr;
	char buf[256];

	/* printage */
	switch (last_print_format) {
	case FMT_HEXB:
	case FMT_BIN:
	case FMT_CSTR:
	case FMT_OCT:
		//if (config.size!=-1 && config.seek+config.block_size> config.size)
		//	printf("\e[2J\e[0;0H");
		break;
	default:
		if (!config.cursor_mode)
			cons_clear();
	}
	if (!getenv("COLUMNS")) {
		terminal_set_raw(0);
		config.width = terminal_get_columns();
		terminal_set_raw(1);
	}
	string_flag_offset(buf, config.seek);
#if __WINDOWS__
	//cons_clear();
	gotoxy(0,0);
	//NEWLINE;
#else
	cons_strcat("\e[0;0H");
#endif
	cons_printf("[ "OFF_FMTs" (inc=%d, bs=%d, cur=%d) %s %s] %s            \n",
		(config.seek+config.baddr), inc,
		(unsigned int)config.block_size,
		config.cursor,
		get_print_format_name(last_print_format),
		(inv)?"inv ":"",
		buf);

	ptr = config_get("cmd.vprompt");
	if (ptr&&ptr[0]) {
		int tmp = last_print_format;
		radare_cmd_raw(ptr, 0);
		last_print_format = tmp;
	}

	radare_seek(config.seek, SEEK_SET);
#if __WINDOWS__
#endif
	radare_print("", last_print_format, MD_BLOCK|inv);

	fflush(stdout);
	cons_flush();
}

static void ringring()
{
	int h   = config.height;
	int now = terminal_get_columns();

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

static int do_repeat=0;
static int repeat=0;
static int repeat_weight=1;

CMD_DECL(visual)
{
	unsigned char key;
	int i, lpf;
	char *name;
	char line[1024];
#if HAVE_LIB_READLINE
	char *ptr;
#endif

	undo_push();

	if (config.height<1)
		config_set_i("scr.height", 24);

	if (bds == NULL)
		bds = (struct binding *)malloc(sizeof(struct binding));

	unsetenv("COLUMNS");
#if __UNIX__
	go_alarm(ringring);
#endif
	config.visual = 1;

	cons_clear();
	terminal_set_raw(1);
	while(1) {
		if (inc<1) inc = 1;
		dec = inc;
		setenv("VISUAL", "1", 1);
		update_environment();
		radare_sync();
		if (config.debug)
			radare_cmd(".!regs*", 0);
		radare_prompt_command();
		visual_draw_screen();

		if (last_print_format == FMT_UDIS) {
		char *follow = config_get("asm.follow");	
		if (follow&&follow[0]) {
			u64 addr = get_offset(follow);
			if ((addr < config.seek) || ((config.seek+config.block_size)<addr))
				radare_seek(addr, SEEK_SET);
		}
		}


	__go_read_a_key:
		/* user input */
		key = cons_readchar();

		switch(key) {
		case 27:
			key = cons_readchar();
			switch(key) {
			case 0x1b:
				key = 'q';
				break;
			case 0x4f: // Fx
				key = cons_readchar();
				key-=0x4f;
				if (config.debug)
				switch(key) {
				case 1: // F1 -help
					cons_clear();
					radare_cmd("!help", 0);
					press_any_key();
					cons_clear();
					continue;
#if DEBUGGER
				/* debugger */
				case 2: // F2 - breakpoint
{
	struct bp_t *bp;
	u64 addr = config.seek + (config.cursor_mode?config.cursor:0);
		//			terminal_set_raw(0);
		//			printf("Breakpoint at: ");
					//toggle_breakpoint(config.seek+(config.cursor_mode?config.cursor:0));

		bp = debug_get_bp(addr);
		if (bp) {
			debug_rm_bp(addr, 0);
		} else {
					sprintf(line, "!bp 0x%08x", (unsigned int)addr);
/*
					strcpy(line, "!bp ");
					fgets(line+4, sizeof(line), stdin);
					line[strlen(line)-1]='\0';
*/
					radare_cmd(line,0);
		}
//					press_any_key();
		//			terminal_set_raw(1);
					CLRSCR();
					continue;
}
				case 3: // F3 - watchpoint
					terminal_set_raw(0);
					printf("Watchpoint at: ");
					strcpy(line, "!drw ");
					fgets(line+5, sizeof(line), stdin);
					line[strlen(line)-1]='\0';
					radare_cmd(line,0);
					terminal_set_raw(1);
					press_any_key();
					cons_clear();
					continue;
				case 4: // F4 - watchpoint
					radare_cmd("!contuh", 0);
					cons_clear();
					continue;
#endif
				}
				break;
			case 0x5b:
				key = cons_readchar();
				switch(key) {
				case 0x35: key='K'; break; // re.pag
				case 0x36: key='J'; break; // av.pag
				case 0x41: key='k'; break; // up
				case 0x42: key='j'; break; // down
				case 0x43: key='l'; break; // right
				case 0x44: key='h'; break; // left
				case 0x31: // Fn
				case 0x32:
					key = cons_readchar();
					switch(key) {
					case 0x37: // F6
						radare_cmd("!contsc", 0);
						break;
					case 0x38: // F7
						if (config.debug)
							CMD_NAME(stepu_in_dbg)(NULL);
						continue;
					case 0x39: // F8
						if (config.debug)
							CMD_NAME(stepo_in_dbg)(NULL);
						continue;
					case 0x30: // F9
						if (config.debug)
							radare_cmd("!cont", 0);
						continue;
					}
				case 0x3b:
					key = cons_readchar();
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
					case 126: // F10
						D printf("Walking until user code...\n");
						radare_cmd("!contu", 0);
						continue;
					default:
						printf("50 unknown key %d\n", key);
						key = cons_readchar();
					} break;
				default:
					printf("0x5b unknown key 0x%x\n", key);
					key = cons_readchar();
					break;
				}
				break;
			default:
				printf("27 unknown key 0x%x\n", key);
				key = cons_readchar();
				break;
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
				udis_jump(key-'0'-1);
				last_print_format = FMT_UDIS;
				continue;
			}
		}

		if (inc<1)inc=1;
		if (repeat==0)repeat=1;
		//else repeat-=1;
		for (i=0;i<repeat;i++) {
		switch(key) {
		case 'r':
			repeat--;
			do_repeat =1;
			break;
			
		case ':':
			terminal_set_raw(0);
			lpf = last_print_format;
#if HAVE_LIB_READLINE
			ptr = readline(VISUAL_PROMPT);
			if (ptr) {
				strncpy(line, ptr, sizeof(line));
				radare_cmd(line, 1);
				//commands_parse(line);
				free(ptr);
			}
#else
			printf(VISUAL_PROMPT);
			fflush(stdout);
			fgets(line, sizeof(line)-1, stdin);
			line[strlen(line)-1]='\0';
			radare_cmd(line, 1);
#endif
			last_print_format = lpf;
			terminal_set_raw(1);
			if (line[0])
				press_any_key();
			cons_strcat("\e[2J\e[0;0H");
			continue;
		case 'b':
			visual_bind_key();
			break;
		case 'm':
			printf("\nrfile magic:\n\n");
			radare_dump_and_process( DUMP_MAGIC, config.block_size);
			press_any_key();
			break;
		case 'd':
			radare_dump_and_process( DUMP_DISASM, config.block_size);
			press_any_key();
			break;
		case 'C':
			config.color^=1;
			sprintf(line, "%d", config.color);
			config_set("scr.color", line);
			break;
		case 'c':
			config.cursor_mode ^= 1;
			break;
		case 'h':
			if (config.cursor_mode) {
				if (config.cursor!=0)
					config.cursor --;
				config.ocursor = -1;
			} else {
				config.seek--;
				if (config.seek<0) config.seek=0;
			}
			break;
		case ']':
		case '[': {
			char buf[1024];
			char *c = getenv("COLUMNS");
			int cols;
			if (!c) cols = config.width;
			else cols = atoi(c);
			sprintf(buf, "%d", cols+((key==']')?4:-4));
			setenv("COLUMNS", buf,1);
			cons_strcat("\e[2J\e[0;0H");
			fflush(stdout);
			break; }
		case 'L':
			if (config.cursor_mode) {
				if (config.ocursor==-1)
					config.ocursor = config.cursor;
				config.cursor += 1;
				if (config.cursor >= config.block_size)
					config.cursor = config.block_size - 1;
			} else
				config.seek += 2;
			break;
		case 0xd:
		case 'j':
			if (config.cursor_mode) {
				config.cursor += inc;
				if (config.cursor >= config.block_size)
					config.cursor = config.block_size - 1;
				config.ocursor = -1;
			} else {
				config.seek += inc;
				if (config.block_size >= (config.size-config.seek))
					cons_clear();
			}
			break;
		case 'J':
			if (config.cursor_mode) {
				if (config.ocursor==-1)
					config.ocursor = config.cursor;
				config.cursor += inc;
				if (config.cursor >= config.block_size)
					config.cursor = config.block_size - 1;
				if (config.block_size >= (config.size-config.seek))
					cons_clear();
				continue;
			} else CLRSCR();
		case ' ':
			if (last_print_format == FMT_DISAS)
				config.seek += 4;
			else	config.seek += config.block_size;

			if (config.size!=-1)
				if (config.seek>config.size)
					config.seek=config.size;
			break;
		case 0x8:
		case 'k':
			if (config.cursor_mode) {
				if (dec > config.cursor)
					config.cursor = 0;
				else config.cursor -= dec;
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
				if (inc > config.cursor)
					config.cursor = 0;
				else	config.cursor -= inc;
			} else {
				if (last_print_format == FMT_DISAS)
					config.seek -= 4;
				else	config.seek -= config.block_size;
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
		case 'f': { int i;
			rad_flag_t *flag = flag_get_next(1);
			if (!flag) flag = flag_get_next(-1);
			if (!flag) flag = flag_get_reset();
			if (flag) config.seek = flag->offset; }
			break;
		case 'F': { rad_flag_t *flag = flag_get_next(-1);
			if (!flag) flag = flag_get_reset();
			if (flag) config.seek = flag->offset; }
			break;
		case 'D':
			name = flag_name_by_offset(config.seek);
			if (name) {
				rad_flag_t *next = flag_get_next(1);
				if (next)
					config.seek = next->offset;
				flag_clear(name);
			}
			break;
		case '<':
			config.seek-= config.block_size;
			if (config.seek % config.block_size)
				cmd_next_align("");
			break;
		case '>':
			config.seek += config.block_size - (config.seek % config.block_size);
			break;
		case 'H':
			if (config.cursor_mode) {
				if (config.ocursor==-1)
					config.ocursor = config.cursor;
				if (config.cursor)
					config.cursor--;
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
			cons_strcat("\e[2J\e[0;0H");
			break;
		case '+':
			radare_set_block_size_i(config.block_size+1);
			break;
		case '-':
			radare_set_block_size_i(config.block_size-1);
			cons_strcat("\e[2J\e[0;0H");
			break;
		case '!':
			cons_clear();
			radare_cmd("!help", 0);
			press_any_key();
			break;
		case '?':
			CLRSCR();
			visual_show_help();
			press_any_key();
			CLRSCR();
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
#if 0
		if (repeat>1) {
			cons_strcat("\e[2J");
			radare_sync();
		}
#endif
		}
		do_repeat = 0;
		repeat = 0;
		repeat_weight=1;
	}

__visual_exit:
	config.visual = 0;
	config.cursor_mode = 0;
	terminal_set_raw(0);
}
