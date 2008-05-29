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

#define GETCOLOR int color = getv("COLOR");
#define TITLE if (color) cons_printf("\e[36m");
#define TITLE_END if (color) cons_printf("\e[0m");

int help_message()
{
	GETCOLOR
	printf("Usage: :!<cmd> (args)\n");
	TITLE
	printf(" Information\n");
	TITLE_END
	printf("  info            show debugger and process status\n");
	printf("  pid [tid]       show pid of the debug process, current tid and childs, or set tid.\n");
	//printf("  pids            show the pids of all the attachable processes\n"); // ??!?
	printf("  status          show the contents of /proc/pid/status\n");
	printf("  signal          show signals handler\n");
	printf("  maps[*]         flags the current memory maps (.!rsc maps)\n");
	printf("  syms            flags all syms of the debugged program (TODO: get syms from libs too)\n");
	printf("  fd[?][-#] [arg] file descriptors (fd? for help)\n");
	printf("  th[?]           threads control command\n");
	TITLE
	printf(" Stack analysis\n");
	TITLE_END
	printf("  bt              backtrace stack frames (use :!bt for user code only)\n");
	printf("  st              analyze stack trace (experimental)\n");
	TITLE
	printf(" Memory allocation\n");
	TITLE_END
	printf("  alloc [N]       allocates N bytes (no args = list regions)\n");
	printf("  mmap [F] [off]  mmaps a file into a memory region\n");
	printf("  free            free allocated memory regions\n");
	printf("  imap            map input stream to a memory region (DEPRECATED)\n");
	TITLE
	printf(" Loader\n");
	TITLE_END
	printf(" !run              load and start execution\n");
	printf(" !load             load a program in memory\n");
	printf(" !unload           unload a program or kill\n");
	printf(" !kill [-S] [pid]  sends a signal to a process\n");
	printf(" !attach [pid]     attach target pid\n");
	printf(" !detach           detach debugged process\n");
	TITLE
	printf(" Flow Control\n");
	TITLE_END
	printf(" !jmp [addr]       force program counter\n");
	printf(" !call [addr]      enters a subroutine\n");
	printf(" !ret              emulates a return from subroutine\n");
	printf(" !skip [N]         skip (N=1) instruction(s)\n");
	printf(" !step [N]         (s) step (N=1) instruction(s)\n");
	printf(" !stepbp [N]       steps using code analysis and breakpoints\n");
	printf(" !stepu            (so) step over library code and repz\n");
	printf(" !stepo            (so) step over calls and repz\n");
	printf(" !trace [N]        trace until bp or eof at N debug level\n");
	printf(" !wtrace           trace until a watchpoint is matched\n");
	printf(" !stepret          continue until ret (TODO)\n");
	printf(" !cont             continue until bp, eof\n");
	printf(" !contu ([addr])   continue until user code, bp, eof (or addr if defined)\n");
	printf(" !contuh           continue until here (loop analysis)\n");
	printf(" !contsc           continue until next syscall\n");
	printf(" !contfork         continue until fork\n");
#if __NetBSD__ || __OpenBSD__ || __APPLE__
	printf(" !ktrace           follow app until ktrace event occurs\n");
#endif
	TITLE
	printf(" Breakpoints\n");
	TITLE_END
	printf(" !wtrace           watchpoint trace\n");
	printf(" !bp [addr]        put a breakpoint at addr (no arg = list)\n");
	printf(" !wp[m|?] [expr]   put a watchpoint (wp? for help) (wpm for monitor)\n");
	printf(" !mp [rwx] [a] [s] change memory protections at [a]ddress with [s]ize\n");
	printf(" !ie               ignore events\n");
	TITLE
	printf(" Registers\n");
	TITLE_END
	printf(" ![o|d|fp]regs[*]  show registers (o=old, d=diff, fp=fpu, *=radare)\n");
	printf(" !oregs[*]         show old registers information (* for radare)\n");
	printf(" !set [reg] [val]  set a value for a register\n");
#if __i386__
	printf(" !dr[rwx-]         DR registers control (dr? for help) (x86 only)\n");
#endif
	TITLE
	printf(" Actions\n");
	TITLE_END
	printf(" !core             dump core of the process\n");
	printf(" !dump [N]         dump pages (and registers) to disk\n");
	printf(" !restore [N]      restore pages (and registers) from disk\n");
	printf(" !hack [N]         Make a hack.\n");
	printf(" !inject [bin]     inject code inside child process\n");

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
	CB_CMD( "run"      , CB_NOARGS   , debug_run )          , 
	CB_CMD( "stepret"  , CB_NOARGS   , debug_stepret )      , 
	CB_CMD( "stepbp"   , CB_INT      , debug_stepbp)        , 
	CB_CMD( "stepo"    , CB_NOARGS   , debug_stepo )        , 
	CB_CMD( "stepu"    , CB_NOARGS   , debug_stepu )        , 
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
	CB_CMD( "dump"     , CB_SPACE    , page_dumper )        , 
	CB_CMD( "restore"  , CB_SPACE    , page_restore )       , 
	CB_CMD( "status"   , CB_NOARGS   , debug_status )       , 
	CB_CMD( "pids"     , CB_NOARGS   , debug_pids )         , 
	CB_CMD( "pid"      , CB_SPACE    , debug_pstree )       , 
	CB_CMD( "attach"   , CB_INT      , debug_attach)        , 
	CB_CMD( "skip"     , CB_INT      , debug_skip)          , 
	CB_CMD( "detach"   , CB_NOARGS   , debug_detach )       , 
	CB_CMD( "load"     , CB_NOARGS   , debug_load )         , 
	CB_CMD( "unload"   , CB_NOARGS   , debug_unload )       , 
	CB_CMD( "ret"      , CB_NOARGS   , arch_ret )           , 
	CB_CMD( "jmp"      , CB_NORMAL   , debug_jmp )          , 
	CB_CMD( "call"     , CB_NORMAL   , debug_call )         , 
	CB_CMD( "info"     , CB_NORMAL   , debug_info )         , 
	CB_CMD( "set"      , CB_NORMAL   , debug_set_register ) , 
	CB_CMD( "wp"       , CB_NORMAL   , debug_wp )           , 
	CB_CMD( "mp"       , CB_NORMAL   , debug_mp )           , 
	CB_CMD( "inject"   , CB_NORMAL   , debug_inject )       , 
	CB_CMD( "trace"    , CB_NORMAL   , debug_trace )        , 
	CB_CMD( "wtrace"   , CB_NOARGS   , debug_wtrace )       , 
	CB_CMD( "signal"   , CB_SPACE    , debug_signal )       , 
	CB_CMD( "contsc"   , CB_NORMAL   , debug_contsc )       , 
	CB_CMD( "contfork" , CB_NOARGS   , debug_contfork )     , 
	CB_CMD( "contu"    , CB_NORMAL   , debug_contu )        , 
	CB_CMD( "contuh"   , CB_NOARGS   , debug_contuh )       , 
	CB_CMD( "cont"     , CB_NORMAL   , debug_cont )         , 
	CB_CMD( "get"      , CB_NORMAL   , debug_getregister )  , 
	CB_CMD( "regs"     , CB_ASTERISK , debug_registers )    , 
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
		return system(command);

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
	return arch_call(get_math(str));
}
