/*
 * Copyright (C) 2007, 2008
 *       pancake <youterm.com>
 *
 * radare is part of the radare project
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

#include "libps2fd.h"
#include "debug.h"
#include "arch/arch.h"
#include "mem.h"
#include <stdio.h>
#include <string.h>

#define GETCOLOR int color = config_get_i("scr.color");
#define TITLE if (color) cons_printf("\x1b[36m");
#define TITLE_END if (color) cons_printf("\x1b[0m");

static int help_message()
{
	GETCOLOR
	TITLE
	cons_printf(" Information\n");
	TITLE_END
	cons_printf("  info               show debugger and process status\n");
	cons_printf("  msg                show last debugger status message\n");
	cons_printf("  pid [tid] [action] show pid of the debug process, current tid and childs, or set tid.\n");
	//cons_printf("  pids            show the pids of all the attachable processes\n"); // ??!?
	cons_printf("  status             show the contents of /proc/pid/status\n");
	cons_printf("  signal             show signals handler\n");
	cons_printf("  syms               flags all syms of the debugged program (TODO: get syms from libs too)\n");
	cons_printf("  fd[?][-#] [arg]    file descriptors (fd? for help)\n");
	cons_printf("  th[?]              threads control command\n");
	TITLE
	cons_printf(" Stack analysis\n");
	TITLE_END
	cons_printf("  bt                 backtrace stack frames (use :!bt for user code only)\n");
	cons_printf("  st                 analyze stack trace (experimental)\n");
	TITLE
	cons_printf(" Memory allocation\n");
	TITLE_END
	cons_printf("  alloc [N]          allocates N bytes (no args = list regions)\n");
	//cons_printf("  lib [path]         load library on target process\n");
	cons_printf("  maps[*|?]          show mapped regions in program memory\n");
	cons_printf("  mmap [F] [off]     mmaps a file into a memory region\n");
	cons_printf("  free               free allocated memory regions\n");
	cons_printf("  imap               map input stream to a memory region (DEPRECATED)\n");
	TITLE
	cons_printf(" Loader\n");
	TITLE_END
	cons_printf("  run [args]         load and start execution\n");
	cons_printf("  (un)load           load or unload a program to debug\n");
	cons_printf("  kill [-S] [pid]    sends a signal to a process\n");
	cons_printf("  {a,de}ttach [pid]  attach or detach target pid\n");
	TITLE
	cons_printf(" Flow Control\n");
	TITLE_END
	cons_printf("  jmp [addr]         set program counter\n");
	cons_printf("  call [addr]        enters a subroutine\n");
	cons_printf("  ret                emulates a return from subroutine\n");
	cons_printf("  skip [N]           skip (N=1) instruction(s)\n");
	cons_printf("  step{o,u,bp,ret}   step, step over, step until user code, step until ret\n");
	cons_printf("  cont{u,uh,wp,sc,fork} continue until user code, here, watchpoint, syscall or fork\n");
	//cons_printf("  contwp             continue until watchpoint matches\n");
	TITLE
	cons_printf(" Tracing\n");
	TITLE_END
	cons_printf("  trace [N]          trace until bp or eof at N debug level (!trace? for help)\n");
	cons_printf("  tt [size]          touch trace using a swapable bps area\n");
#if __NetBSD__ || __OpenBSD__ || __APPLE__
	cons_printf("  ktrace             follow app until ktrace event occurs\n");
#endif
	cons_printf("  wp[?] [-id|expr]   add watchpoint (!wp? for help) (see dbg.wptrace)\n");

	TITLE
	cons_printf(" Breakpoints\n");
	TITLE_END
	cons_printf("  bp [addr]          put a breakpoint at addr (no arg = list)\n");
	cons_printf("  mp [rwx] [a] [s]   change memory protections at [a]ddress with [s]ize\n");
	cons_printf("  ie [-][event]      ignore debug events\n");
	TITLE
	cons_printf(" Registers\n");
	TITLE_END
	cons_printf("  [o|d|fp]regs[*]    show registers (o=old, d=diff, fp=fpu, *=radare)\n");
	cons_printf("  reg[s|*] [reg[=v]  show get and set registers\n");
	cons_printf("  oregs[*]           show old registers information (* for radare)\n");
#if __i386__
	cons_printf("  dr[rwx-]           DR registers control (dr? for help) (x86 only)\n");
#endif
	TITLE
	cons_printf(" Other\n");
	TITLE_END
	cons_printf("  dump/restore [N]   dump/restore pages (and registers) to/from disk\n");
	cons_printf("  dall               dump from current seek to cfg.limit all available bytes (no !maps required)\n");
	cons_printf("  core               dump core of the process\n");
	cons_printf("  fork               fork current process\n");
	cons_printf("  hack [N]           Make a hack.\n");
	//cons_printf("  inject [bin]       inject code inside child process (UNSTABLE)\n");
	cons_printf("Usage: !<cmd>[?] <args> @ <offset>     ; see eval dbg. fmi\n");

	return 0;
}

enum {
	CB_NOARGS,
	CB_NORMAL,
	CB_ASTERISK,
	CB_INT,
	CB_SPACE
};

#define CB_CMD(cmd, type, function) { cmd, type, (int(*)(char *))&function } 

static struct commads_t {
	char *name;
	int type;
	int (*callback)(char *);
} commands[] = {
	CB_CMD( "help"     , CB_NOARGS   , help_message )       , 
	CB_CMD( "?"        , CB_NOARGS   , help_message )       , 
	CB_CMD( "run"      , CB_NORMAL   , debug_run )          , 
	CB_CMD( "status"   , CB_NOARGS   , debug_status )       , 
	CB_CMD( "stepret"  , CB_NOARGS   , debug_stepret )      , 
	CB_CMD( "stepbp"   , CB_INT      , debug_stepbp)        , 
	CB_CMD( "stepo"    , CB_NOARGS   , debug_stepo )        , 
	CB_CMD( "stepu"    , CB_NORMAL   , debug_stepu )        , 
	CB_CMD( "step"     , CB_INT      , debug_step)          , 
	CB_CMD( "bp"       , CB_NORMAL   , debug_bp )           , 
	CB_CMD( "bt"       , CB_NORMAL   , debug_bt )           , 
	CB_CMD( "st"       , CB_NORMAL   , arch_stackanal )     , 
	CB_CMD( "fd"       , CB_NORMAL   , debug_fd )           , 
	CB_CMD( "th"       , CB_NORMAL   , debug_th )           , 
	CB_CMD( "ie"       , CB_NORMAL   , debug_ie )           , 
	CB_CMD( "hack"     , CB_SPACE    , arch_hack )          , 
	CB_CMD( "maps"     , CB_NORMAL   , debug_print_maps )   , 
	CB_CMD( "syms"     , CB_NORMAL   , debug_syms )         , 
	CB_CMD( "alloc"    , CB_SPACE    , debug_alloc )        , 
	CB_CMD( "loop"     , CB_SPACE    , debug_loop )         , 
	CB_CMD( "mmap"     , CB_SPACE    , debug_mmap )         , 
	CB_CMD( "free"     , CB_SPACE    , debug_free )         , 
	CB_CMD( "imap"     , CB_SPACE    , debug_imap )         , 
	CB_CMD( "core"     , CB_NOARGS   , debug_dumpcore )     , 
	CB_CMD( "dump"     , CB_SPACE    , process_dump )       , 
	CB_CMD( "restore"  , CB_SPACE    , process_restore )    , 
	CB_CMD( "pids"     , CB_NOARGS   , debug_pids )         , 
	CB_CMD( "pid"      , CB_SPACE    , debug_pstree )       , 
	CB_CMD( "attach"   , CB_INT      , debug_attach)        , 
	CB_CMD( "detach"   , CB_NOARGS   , debug_detach )       , 
	CB_CMD( "skip"     , CB_INT      , debug_skip)          , 
	CB_CMD( "tt"       , CB_NORMAL   , debug_tt)            , 
	CB_CMD( "load"     , CB_NOARGS   , debug_load )         , 
	CB_CMD( "unload"   , CB_NOARGS   , debug_unload )       , 
	CB_CMD( "ret"      , CB_NOARGS   , arch_ret )           , 
	CB_CMD( "jmp"      , CB_NORMAL   , debug_jmp )          , 
	CB_CMD( "call"     , CB_NORMAL   , debug_call )         , 
	CB_CMD( "info"     , CB_NORMAL   , debug_info )         , 
	CB_CMD( "msg"      , CB_NOARGS   , debug_msg )          , 
	CB_CMD( "set"      , CB_NORMAL   , debug_set_register ) , 
	CB_CMD( "wp"       , CB_NORMAL   , debug_wp )           , 
	CB_CMD( "mp"       , CB_NORMAL   , debug_mp )           , 
	//CB_CMD( "inject"   , CB_NORMAL   , debug_inject )       , 
	//CB_CMD( "lib"      , CB_NORMAL   , debug_lib)           , 
	CB_CMD( "trace"    , CB_NORMAL   , debug_trace )        , 
	CB_CMD( "signal"   , CB_NORMAL   , debug_signal )       , 
	CB_CMD( "sig"      , CB_NORMAL   , debug_signal )       , 
	CB_CMD( "contsc"   , CB_NORMAL   , debug_contsc )       , 
	CB_CMD( "contfork" , CB_NOARGS   , debug_contfork )     , 
	CB_CMD( "contwp"   , CB_NOARGS   , debug_contwp )       , 
	CB_CMD( "dall"     , CB_NOARGS   , debug_dumpall)       ,
	CB_CMD( "contu"    , CB_NORMAL   , debug_contu )        , 
	CB_CMD( "contuh"   , CB_NOARGS   , debug_contuh )       , 
	CB_CMD( "cont"     , CB_NORMAL   , debug_cont )         , 
	CB_CMD( "fork"     , CB_NORMAL   , debug_fork )         , 
	CB_CMD( "get"      , CB_NORMAL   , debug_get_register ) , 
	CB_CMD( "reg"      , CB_NORMAL   , debug_reg )          , //debug_registers )    , 
	CB_CMD( "oregs"    , CB_ASTERISK , debug_oregisters )   , 
	CB_CMD( "dregs"    , CB_ASTERISK , debug_dregisters )   , 
	CB_CMD( "fpregs"   , CB_ASTERISK , debug_fpregisters )  , 
#if __NetBSD__ || __OpenBSD__ || __APPLE__
	CB_CMD( "ktrace", CB_NOARGS, debug_ktrace ),
#endif
#if __i386__
	CB_CMD( "dr",  CB_NORMAL, debug_dr ),
#endif
	{ NULL, 0 }
};

int debug_lib(const char *arg)
{
	if (arg[0]!=' ')
		cons_printf("Usage: !lib /path/to/lib.so\n");
	else debug_lib_load(arg+1);
	return 0;
}

int debug_reg(const char *arg)
{
	if (arg[0]) {
		if (arg[0]=='?') {
			eprintf("Usage: !reg[2][*] [reg [=value]]\n");
		} else
		if (arg[0]=='2') {
			debug_registers(2);
		} else
		if ((arg[0]=='s' && arg[1]=='*') || arg[0] == '*')
			debug_registers(1);
		else {
			if (arg[0]=='s' || strnull(arg))
				debug_registers(0);
			else {
				if (strchr(arg+1, '='))
					debug_set_register(arg+1);
				else debug_get_register(arg+1);
			}
		}
	} else debug_registers(0);
	return 0;
}

int debug_system(const char *command)
{
	int i;

	for(i=0;commands[i].name;i++) {
		int len = strlen(commands[i].name);
		if (!memcmp(command, commands[i].name, len)) {
			switch(commands[i].type) {
			case CB_ASTERISK:
				return commands[i].callback(strchr(command+len,'*'));
			case CB_NORMAL:
				return commands[i].callback((char *)command+len);
			case CB_NOARGS:
				return commands[i].callback(NULL);
			case CB_SPACE:
				return commands[i].callback(strchr(command+len,' '));
			case CB_INT:
				return commands[i].callback((char *)atoi(command+len));
			}
		}
	}

	/* aliases */
	if (!strcmp(command, "s"))
		return debug_step(1);
	else
		return radare_system(command);

	if (!ps.opened)
		eprintf("No program loaded.\n");

	return -1;
}

int debug_jmp(const char *str)
{
	return arch_jmp(get_math(str));
}

int debug_call(const char *str)
{
	return arch_call(str);
}
