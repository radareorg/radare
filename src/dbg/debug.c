/*
 * Copyright (C) 2007, 2008, 2009
 *       pancake <youterm.com>
 *       th0rpe <nopcode.org>
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

#define TRACE printf("%s:%d\n", __FILE__, __LINE__);

#include "libps2fd.h"
#include "arch/arch.h"
#include "../radare.h"
#include "../config.h"
#include "../print.h"
#include "../code.h"
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#if __linux__
#include <sys/prctl.h>
#include <linux/ptrace.h>
#endif
#include <sys/stat.h>
#include "wp.h"
#include "mem.h"
#include "thread.h"
#include "signals.h"
#include "events.h"
#include "debug.h"


#if 0
// CPUID
void get_cpuid(int op, int *a, int *b, int *c, int *d)
{
	__asm__ __volatile__(
	"pushl %%ebx"
	"cpuid\n"
	"movl %%ebx, %%esi\n"
	"popl %%ebx"
	: "=a" (*a), "=S" (*b), "=c" (*c), "=d" (*d)
	: "0" (op));
}

	get_cpuid (1, &eax, &ebx, &ecx)
	if (edx & (1<<23)) // MMX
	if (edx & (1<<25)) // SSE
	if (edx & (1<<26)) // SSE2
	if (ecx & (1))     // SSE3
#endif

int debug_syms()
{
	char buf[1024];
	snprintf(buf, 1022, ".!!rabin -ris '%s'", ps.argv[0]); //config.file);
	return radare_cmd_raw(buf, 0);
}

// TODO : stupid helper
int getv()
{
	return (int)config_get("cfg.verbose");
#if 0
	char *ptr = config_get("cfg.verbose");
	if (ptr&&ptr[0]=='1')
		return 1;
	return 0;
#endif
}

void debug_tt_ranges_new_callback(struct range_t *r)
{
	u8 bytes[64];
	struct aop_t aop;

	if (r == NULL) {
		printf("null range?\n");
		return;
	}
	if (r->from +1 == r->to) {
		debug_read_at(ps.tid, bytes, 32, r->from);
		arch_aop(r->from, bytes, &aop);
		r->datalen = aop.length;
		r->to = r->from + r->datalen;
		r->data = (u8*)malloc(r->datalen);
		debug_read_at(ps.tid, r->data, r->datalen, r->from);
	} else {
		r->datalen = r->to - r->from;
		r->data = (u8*)malloc(r->datalen);
		debug_read_at(ps.tid, r->data, r->datalen, r->from);
	}
	fprintf(stderr, " add 0x%llx - 0x%llx\n", r->from, r->to);
}

/* TODO: add ranges support here */
int debug_tt_range(const char *arg)
{
	struct aop_t aop;
	int pid, bpsz;
	ut64 pc, rpc; // program counter
	u8 *sa; // swap area
	u8 *cc; // breakpoint area
	ut64 sz;
	char *cmd = NULL;
	ut64 ba = config.seek; // base address
	struct list_head *rgs, *pos;
	struct range_t *r;
	int status;

	if (arg[0]=='\0') {
		cons_printf("Usage: !ttr [ranges..]\n");
		return 0;
	}
	ranges_new_callback = &debug_tt_ranges_new_callback;
	rgs = ranges_new(arg);

	sz = ranges_size(rgs);
	cons_printf("TouchTracing %lld bytes using ranges\n", sz);
	cons_flush();
	cmd = config_get("cmd.touchtrace");
	/* */
	radare_controlc();
	cc = (u8 *)malloc(sz);
	/* TODO: use get_arch_breakpoint() or whatever */
	switch(config.arch) {
	case ARCH_ARM:
		bpsz = 4;
		memcpy_loop(cc,"\x01\x00\x9f\xef", sz, bpsz);
		break;
	case ARCH_MIPS:
		bpsz = 4;
		memcpy_loop(cc,"\x0d\x00\x00\x00", sz, bpsz);
		break;
	case ARCH_X86:
	default:
		memset(cc, '\xCC', sz);
		bpsz = 1;
		break;
	}
	list_for_each(pos, rgs) {
		r = list_entry(pos, struct range_t, list);
		debug_write_at(ps.tid, cc, r->datalen, r->from);
	}
	free(cc);

	while(!config.interrupted) {
		debug_contp(ps.tid);
		pid = debug_waitpid(-1, &status);
		ps.tid = pid;

		pc = arch_pc(ps.tid);
		if ((pc & 0xffffffff) == 0xffffffff) {
			perror("arch_pc");
			break;
		}
		r = ranges_get(rgs,pc);
		if (r != NULL) {
			int delta;
			rpc = pc-bpsz;
			delta= rpc-r->from;
		//if (pc >= ba && pc <= ba+sz) {
			/* swap area and continue */
			if (cmd && *cmd)
				radare_cmd_raw(cmd, 0);
			arch_aop(rpc, r->data+delta, &aop);
			debug_write_at(ps.tid, r->data+delta, aop.length, rpc);
			arch_jmp(rpc); // restore pc // XXX this is x86 only!!!
			printf("0x%llx %d\n", rpc, aop.length);
			trace_add(rpc);
			continue;
		} else {
			/* unexpected stop */
			cons_printf("Oops: Unexpected stop oustide tracing area pid=%d at 0x%08llx\n", ps.tid, pc);
			break;
		}
	}
	radare_controlc_end();

	/* restore memory */
	list_for_each(pos, rgs) {
		r = list_entry(pos, struct range_t, list);
		debug_write_at(ps.tid, r->data, r->datalen, r->from);
	}

	return 1;
}

/* TODO: add ranges support here */
int debug_tt(const char *arg)
{
	struct aop_t aop;
	int pid, bpsz;
	ut64 pc; // program counter
	u8 *sa; // swap area
	u8 *cc; // breakpoint area
	const char *cmd = NULL;
	ut64 ba = config.seek; // base address
	ut64 sz = get_math(arg+1); // size
	int status;

	// XXX
	if (*arg == 'r')
		return debug_tt_range(arg+1);

	if (sz<1) {
		cons_printf("Usage: !tt[r ranges] | [size @ base_address]\n");
		cons_printf(" !tt 1024 @ eip                     ; touchtraces 1024 bytes from eip\n");
		cons_printf(" !ttr eip-eip+40,0x804800-0x8049000 ; touchtraces defined ranges (, and -)\n");
		return 0;
	}
	cmd = config_get("cmd.touchtrace");
	cons_printf("TouchTracing %lld bytes from 0x%08llx..\n", sz, ba);
	cons_flush();
	/* */
	radare_controlc();
	sa = (u8 *)malloc(sz);
	cc = (u8 *)malloc(sz);
	debug_read_at(ps.tid, sa, (int)sz, ba);
	/* TODO: use get_arch_breakpoint() or whatever */
	switch(config.arch) {
	case ARCH_ARM:
		bpsz = 4;
		memcpy_loop(cc,"\x01\x00\x9f\xef", sz, bpsz);
		break;
	case ARCH_MIPS:
		bpsz = 4;
		memcpy_loop(cc,"\x0d\x00\x00\x00", sz, bpsz);
		break;
	case ARCH_X86:
	default:
		memset(cc, '\xCC', sz);
		bpsz = 1;
		break;
	}
	debug_write_at(ps.tid, cc, (int)sz,ba);

	while(!config.interrupted) {
		debug_contp(ps.tid);
		pid = debug_waitpid(-1, &status);
		ps.tid = pid;

		pc = arch_pc(ps.tid);
		if ((pc & 0xffffffff) == 0xffffffff) {
			perror("arch_pc");
			break;
		}
		if (pc >= ba && pc <= ba+sz) {
			/* swap area and continue */
			int delta = pc-ba-bpsz;

#if 0
			TODO: ensure this is a breakpoint
			debug_read_at(ps.tid, tmp, 4, pc-bpsz);

			if (tmp[0]=='\xcc') {
				printf("RESTORING\n");
			} else {
				printf("SKIPPING 0x%llx\n",pc-bpsz);
			//arch_jmp(pc-bpsz); // restore pc // XXX this is x86 only!!!
			//	continue;
			}
#endif
			if (cmd && *cmd)
				radare_cmd_raw(cmd, 0);
			arch_aop(pc-bpsz, sa+delta, &aop);
			debug_write_at(ps.tid, sa+delta, aop.length, ba+delta);
			arch_jmp(pc-bpsz); // restore pc // XXX this is x86 only!!!
			printf("0x%llx %d\n", pc-bpsz, aop.length);
			trace_add(pc-bpsz); // trace it!
			continue;
		} else {
			/* unexpected stop */
			cons_printf("Oops: Unexpected stop oustide tracing area pid=%d at 0x%08llx\n", ps.tid, pc);
			break;
		}
	}
	radare_controlc_end();
	debug_write_at(ps.tid, (void *)sa, (int)sz,ba);
#if 0 /* check */
	debug_read_at(ps.tid, sa, (int)sz, ba);
	printf("%x %x\n", sa[0], sa[1]);
#endif
	free(sa);
	free(cc);
	return 1;
}

/// XXX looks wrong
/// XXX use wait4 and get rusage here!!!
/// XXX move to dbg/unix
/* stat 4 flag 1 => this message is thrown by the BSD kernel when trying to 
 * act over a process that have some waitpid events pending */
int debug_waitpid(int pid, int *status)
{
#if 0
#define CRASH_LINUX_KERNEL 0
#if CRASH_LINUX_KERNEL
	if (pid == -1)
		return -1;
#endif
#endif

//	return waitpid(-1, status, __WALL | __WCLONE | WUNTRACED);
#if __WINDOWS__
	/* not implemented ? */
#elif __FreeBSD__ || __NetBSD__ || __OpenBSD__
	return waitpid(pid, status, WUNTRACED);
#else
  #ifdef __WCLONE
	if (config_get("dbg.threads"))
		return waitpid(pid, status, __WALL | __WCLONE | WUNTRACED);
  #endif

  #if __linux__
	return waitpid(pid, status, WUNTRACED);
  #ifdef __WALL
	return waitpid(pid, status, __WALL | WUNTRACED);
  #else
    #ifdef WUNTRACED
	return waitpid(pid, status, WUNTRACED);
    #else
	return waitpid(pid, status, 0);
    #endif
  #endif
#endif
	return -1;
#endif
}

void debug_msg()
{
	cons_printf("%s\n", ps.msg);
}

int debug_init()
{
	// TODO use memcpy
	ps.tid      =  0;
	ps.msg      = NULL;
	ps.steps    =  0;
	ps.offset   = -1;
	ps.opened   =  0;
	ps.is_file  =  0;
	ps.isbpaddr =  0;
	ps.filename = NULL;
	ps.mem_sz   =  0;
	ps.map_regs_sz   =  0;
	ps.fd       = (rand()%30000)+1000;
	ps.argv[0] = NULL;
	//ps.argv     = (char **)malloc(254 * sizeof(char *));
	//memset(&ps.argv, '\0', sizeof(ps.argv));
	ps.bps_n    =  0;
	ps.wps_n    =  0;
	memset(ps.bps, '\0', sizeof(struct bp_t) * MAX_BPS);
	ps.args     = NULL;
        INIT_LIST_HEAD(&(ps.map_mem));
        INIT_LIST_HEAD(&(ps.map_reg));
        INIT_LIST_HEAD(&(ps.th_list));       
#if __i386__
	/* initialize DR registers */
	// XXX only for dbg:// !!!
	// pid:// can cause problems, so the process can have
	//        different DR registers than 0. uhm :/
	// init_dr();
#endif
	debug_os_init();

	return 0; //for warning message
}

int debug_print_maps(char *arg)
{
	if(debug_init_maps(1) == 0) {
		print_maps_regions((int)strchr(arg, '*'), arg[0]=='?');
		return 0;
	}

	return -1;
}

void debug_msg_set(const char *format, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf,1023, format, ap);
	free(ps.msg);
	ps.msg = strdup(buf);
	va_end(ap);
}

void debug_environment()
{
	const char *ptr;

	unsetenv("COLUMNS");
	unsetenv("LINES");
	// Load environment from file
	ptr = config_get("file.dbg_env");
	if (ptr && *ptr) {
		char buf[4096], *eq;
		FILE *fd;
		int quoted, len;
		eprintf("Loading file.dbg_env from '%s'\n", ptr);
		clearenv();
		// hack '_' to emulate the real one of the process
		setenv("_", ps.filename, 1);
		fd = fopen(ptr, "r");
		if (fd != NULL) {
			/* expected format is: [#comment][export ][VAR]="[VALUE]" */
			while(!feof(fd)) {
				fgets(buf, 4095, fd);
				if (feof(fd))
					break;
				if (!buf[0]) continue;
				len = strlen(buf);
				quoted = (buf[len-2]=='"')?2:1;
				buf[len-quoted]=0; // chop '\n'
				if (!memcmp(buf, "export ", 7))
					memcpy(buf, buf+7, strlen(buf+7)+1);
				eq = strchr(buf, '=');
				if (eq) {
					*eq = 0;
					if (eq[1]=='@') {
						char *contents = slurp(eq+quoted, NULL);
						setenv(buf, contents, 1);
						free(contents);
					} else setenv(buf, eq+quoted, 1);
				}
			}
		} else eprintf("Cannot open '%s'\n", ptr);
	} else {
		// hack '_' to emulate the real one of the process
		if (ps.filename[0]=='/') {
			char str[1024];
			snprintf(str, 1023, "./%s", ps.filename);
			setenv("_", str, 1);
		} else setenv("_", ps.filename, 1);
	}

	if (config_get_i("dbg.env_ldso")) {
		/* LD.SO */
#if 0
Valid options for the LD_DEBUG environment variable are:

  libs        display library search paths
  reloc       display relocation processing
  files       display progress for input file
  symbols     display symbol table processing
  bindings    display information about symbol binding
  versions    display version dependencies
  all         all previous options combined
  statistics  display relocation statistics
  unused      determined unused DSOs
  help        display this help message and exit

To direct the debugging output into a file instead of standard output
a filename can be specified using the LD_DEBUG_OUTPUT environment variable.
#endif
		// LD_PRELOAD
		// LD_BIND_NOW : resolve all symbols at startup
		setenv("LD_BIND_NOW", "true", 1);
		//setenv("LD_TRACE_LOADED_OBJECTS", "true", 1);

		setenv("LD_WARN", "true", 1);
		setenv("LD_DEBUG", "", 1);
		setenv("LD_VERBOSE", "", 1);
		setenv("CK_FORK","no",1); // to debug gstreamer check test programs
		//setenv("LD_SHOW_AUXV", "true", 1);
		//setenv("LD_PROFILE", "/lib/libc.so.6", 1);
	}

	// TODO: Set resources limits (setrlimit)
#if __linux__
	prctl(PR_SET_DUMPABLE, 1,1,1,1);
	//prctl(PR_SET_FPEMU); // only for ia64 :(
	// can trace fpu (man prctl)

	// signal q rebra el fill si el pare mor
	prctl(PR_SET_PDEATHSIG, SIGKILL, 0,0,0);
#endif
	ptr = config_get("child.chdir");
	if (!strnull(ptr)) chdir(ptr);
#if __UNIX__
	ptr = config_get("child.chroot");
	if(!strnull(ptr)) chroot(ptr);
	ptr = config_get("child.setuid");
	if (!strnull(ptr)) setuid(atoi(ptr));
	ptr = config_get("child.setgid");
	if (!strnull(ptr)) setgid(atoi(ptr));
#endif
	// TODO: add suid bin chmod 4755 ${FILE}
	if (config_get("child.stdio") != NULL) {
		debug_fd_hijack(0, config_get("child.stdio"));
		debug_fd_hijack(1, config_get("child.stdio"));
		debug_fd_hijack(2, config_get("child.stdio"));
	} else {
		debug_fd_hijack(0, config_get("child.stdin"));
		debug_fd_hijack(1, config_get("child.stdout"));
		debug_fd_hijack(2, config_get("child.stderr"));
	}
}

int debug_bt()
{
	const char *type = config_get("dbg.bttype");
	if ((type == NULL) || (!strcmp(type, "default"))) {
		struct list_head *bt = arch_bt();
		if(bt != NULL) {
			arch_view_bt(bt);
			free_bt(bt);
		}
	} else
	/* should be removed after testing it properly */
	if (!strcmp(type, "old")) {
		return arch_backtrace();
	} else
	if (!strcmp(type, "st")) {
		return arch_stackanal();
	} else eprintf("Invalid value for dbg.bttype. Use 'standard', 'old' or 'st'\n");

	return 0;
}

void debug_exit()
{
	debug_init();
}

int is_code(addr_t pc)
{
	struct list_head *pos;

	if (ps.map_regs_sz==0) {
		eprintf("Cannot determine usercode.\n");
		return 1;
	}

	list_for_each_prev(pos, &ps.map_reg) {
		MAP_REG	*mr = (MAP_REG *)((char *)pos + \
			sizeof(struct list_head) -  sizeof(MAP_REG));
		if (pc >= mr->ini && pc <= mr->end && !access(mr->bin, R_OK))
			return 1;
	}
	return 0;
}

int is_usercode(addr_t pc)
{
	struct list_head *pos;

	if (ps.map_regs_sz==0) {
		eprintf("Cannot determine usercode.\n");
		return 1;
	}

	list_for_each_prev(pos, &ps.map_reg) {
		MAP_REG	*mr = (MAP_REG *)((char *)pos + \
			sizeof(struct list_head) -  sizeof(MAP_REG));
		if (mr->flags & FLAG_USERCODE)
			if (pc >= mr->ini && pc <= mr->end)
				return 1;
	}
	return 0;
}

char *alloc_usercode(ut64 *base, ut64 *len)
{
	struct list_head *pos;
	u8 *buf = NULL;

	if (ps.map_regs_sz==0) {
		eprintf("Cannot determine usercode.\n");
		return 1;
	}

	list_for_each_prev(pos, &ps.map_reg) {
		MAP_REG	*mr = (MAP_REG *)((char *) pos + \
			sizeof(struct list_head) -  sizeof(MAP_REG));
		if (mr->flags & FLAG_USERCODE) { // && (pc >= mr->ini && pc <= mr->end)) {
			*base = mr->ini;
			*len = mr->end-mr->ini;
			buf = malloc(mr->end - mr->ini+1);
			debug_read_at(ps.tid, buf, *len, *base);
			eprintf("Using usercode range: 0x%08llx - 0x%08llx\n", *base, *base+*len);
			break;
		}
	}
	return buf;
}

int debug_stepret()
{
	char bytes[4];
	struct aop_t aop;

	/* make steps until esp = oesp and [eip] == 0xc3 */
	radare_controlc();
	while(!config.interrupted && ps.opened && debug_stepo()) {
		debug_read_at(ps.tid, bytes, 4, arch_pc(ps.tid));
		arch_aop((ut64)arch_pc(ps.tid), (const u8 *)bytes, &aop);
		if (aop.type == AOP_TYPE_RET)
			break;
	}
	radare_controlc_end();

	return 0;
}

int debug_contum(const char *input)
{
	struct aop_t aop;
	ut64 pc;
	char a[32];
	int ilen, dotrace = config_get("trace.log");
	ut64 base = 0, len = 0;
	u8 *buf = alloc_usercode(&base, &len);
	if (buf == NULL) {
		eprintf("No user code?\n");
		return 1;
	}

	radare_controlc();
	do {
		debug_step(1);
		ps.steps++;
		pc = arch_pc(ps.tid);
		if (dotrace)
			trace_add((addr_t)pc);
		
		if (pc >= base && pc <= base+len) {
			debug_read_at(ps.tid, a, 32, pc);
			ilen = arch_aop(pc, a, &aop);
			if (ilen>0) {
				if (memcmp(buf+(pc-base), a, ilen)) {
					eprintf("Some opcodes changed here!\n");
					eprintf("address = 0x%08llx\n", pc);
					eprintf("old bytes = %02x %02x %02x ...\n",
						buf[pc-base], buf[pc-base+1], buf[pc-base+2]);
					eprintf("new bytes = %02x %02x %02x ...\n",
						a[0], a[1], a[2]);
					break;
				}
			}
		}
	} while (!config.interrupted);
	radare_controlc_end();
	free(buf);

	return 0;
}

int debug_contu(const char *input)
{
	int is_user;

	if (input) {
		for(;*input==' ';input=input+1);
		if (*input) {
			eprintf("Stepping until %s...\n", input);
			debug_cont_until(input);
			return 0;
		}
	}

	eprintf("Stepping until user code...\n");
	radare_controlc();
	ps.verbose = 0;
	while(!config.interrupted && ps.opened && debug_step(1)) {
#if 0
eprintf(". (0x%08llx)\n", arch_pc(ps.tid));
radare_cmd(".!regs*",0);
radare_cmd("pd 1 @ eip",0);
#endif
		is_user = is_usercode(arch_pc(ps.tid));
		if (is_user)
			return 0;
	}
	radare_controlc_end();
	ps.verbose = 1;

	if (is_user) {
		if (config_get("trace.log"))
			trace_add((addr_t)arch_pc(ps.tid));
	}

	return 0;	
}

int debug_contwp()
{
	const char *cmd;
	int ret;

	/* not watchpoints, nothing */
	if(ps.wps_n == 0) {
		printf("not watchpoint to trace\n");
		return 0;
	}

	radare_controlc();

        ps.verbose = 0;
        while(!config.interrupted && ps.opened && debug_step(1)) {
		ret = debug_wp_match();
		if(ret >= 0) {
			printf("watchpoint %d matches at 0x%x\n",
				 ret, (unsigned int)arch_pc(ps.tid));

			cmd = config_get("cmd.wp");
			if (!strnull(cmd))
				radare_cmd_raw(cmd, 0);

			if (config_get("dbg.wptrace"))
				continue;

			break;
		}
        }
        ps.verbose = 1;

	radare_controlc_end();

        return 0;
}

int debug_ie(char *input)
{
	if (!input)
		return 0;

	if (strchr(input, '?')) {
		eprintf(
		"Usage: !ie [-]<event>\n"
		" - enables or disables the 'ignore event' \n");
		return 0;
	}
	while(input[0]==' ') input = input+1;

	if (strnull(input)) {
		event_ignore_list();
		return 0;
	}

	event_set_ignored(input+(input[0]=='-'), input[0]!='-');

	return 0;
}

int debug_until(const char *addr)
{
	ut64 off = 0LL;
	char buf[128];
	u8 ptr[128];

	if (!addr)
		return 0;

	ps.entrypoint = arch_get_entrypoint();

	off = 0;
	if ((!strcmp("entry", addr)
	|| !strcmp("main", addr)))
		off = ps.entrypoint;
	else	off = get_math(addr);

	if (off != 0) {
		eprintf("entry at: 0x%llx\n", (ut64) ps.entrypoint);
		sprintf(buf, "0x%llx\n", (ut64) off);
		debug_cont_until(buf);
	}

	if (config_get("dbg.syms"))
		debug_system("syms");

	if (!strcmp("main", addr)) {
		// XXX intel only
		// XXX BP_SOFT is ugly (linux supports DRX HERE)

		debug_cont_until("sym_main");
#if 1
		debug_read_at(ps.tid, buf, 12, arch_pc(ps.tid));
		if (!memcmp(buf, "\x31\xed\x5e\x89\xe1\x83\xe4\xf0\x50\x54\x52\x68", 12)) {
			debug_read_at(ps.tid, &ptr, 4, arch_pc(ps.tid)+24);
			off = (ut64)(unsigned int)(ptr[0]) | (ptr[1]<<8) | (ptr[2] <<16) | (ptr[3]<<24);
			sprintf(buf, "0x%x", (unsigned int)off);
			printf("==> 1 main at : %s\n", buf);
			if (off !=0&&ptr[0]!=0xff)
				debug_cont_until(buf);
		} else
		if (!memcmp(buf, "^\x89\xe1\x83\xe4\xf0PTRh", 10)) {
			unsigned int addr;
			debug_read_at(ps.tid, &addr, 4, arch_pc(ps.tid)+0x16);
			off = (ut64)addr;
			sprintf(buf, "0x%x", addr);
			printf("==> 2 main at : %s\n", buf);
			if (off !=0&&off!=0xffffffffLL)
				debug_cont_until(buf);
		} else
			eprintf("Cannot find main analyzing the entrypoint. Try harder.\n");
#endif
	}

	return 0;
}

int debug_free(char *args)
{
	addr_t addr;

	if(!args) {
		eprintf(":free invalid address\n");
		return 1;
	}

	addr = get_math(args + 1);

	if(dealloc_page(addr) == 0)
		eprintf(":free 0x%x region not found!\n", (unsigned int)addr);
	else
		eprintf(":free region 0x%x\n", (unsigned int)addr); 

	return 0;	
}

int debug_alloc_status()
{
	print_status_alloc();
	return 0;
}

int debug_info(char *arg)
{
	int fd;
	char buf[4096];

	if (strchr(arg,'*')) {
		cons_printf("f entry @ 0x%08x\n", ps.entrypoint);
		//cons_printf("f bep @ %s\n", config_get("dbg.bep"));
	} else {
		cons_printf(" filename    %s\n", strget(ps.filename));

		cons_printf(" pid         %d\n", ps.pid);
		cons_printf(" tid         %d\n", ps.tid);
		sprintf(buf, "/proc/%d/cmdline", ps.tid);
		if ((fd = open(buf, O_RDONLY)) !=-1) {
			memset(buf,'\0',4096);
			read(fd, buf, 4095);
			cons_printf(" cmdline     %s\n", buf);
			close(fd);
		}

		// TODO is all this stuff necesary?
		cons_printf(" tid(current)%d\n", ps.tid);
		sprintf(buf, "/proc/%d/cmdline", ps.tid);
		if ((fd = open(buf, O_RDONLY)) !=-1) {
			memset(buf,'\0',4096);
			read(fd, buf, 4095);
			cons_printf(" cmdline     %s\n", buf);
			close(fd);
		}
		cons_printf(" dbg.bep     %s\n", config_get("dbg.bep"));
		cons_printf(" entry       0x%08x\n", ps.entrypoint);
		cons_printf(" ldentry     0x%08x\n", ps.ldentry);
		cons_printf(" opened      %d\n", ps.opened);
		cons_printf(" steps       %d\n", ps.steps);
		cons_printf(" offset      0x%llx\n", ps.offset);
		cons_printf(" isbpaddr    %d\n", ps.isbpaddr);
		cons_printf(" dbg_message %s\n", strget(ps.msg));
	}

	return 0;
}

int debug_th(char *cmd)
{
	int newpid = atoi(cmd);

	if (strchr(cmd,'?')) {
		eprintf("Usage: th [pid]\n");
		eprintf("- Change current thread\n");
		eprintf("- Use !pid to see other processes\n");
		return 0;
	} 

	if (newpid !=0)
		ps.tid = newpid;

	// TODO: use a single interface for managing threads in os dependant way
#if __APPLE__
	free_th();
	debug_list_threads(ps.tid);
#endif
#if __FreeBSD__
	th_init_freebsd(ps.tid);
#endif
#if __BSD__
	th_info_bsd(ps.tid);
#endif
	return th_list();
}

int debug_skip(int times)
{
	unsigned char buf[16];
	struct aop_t aop;
	int len;
	ut64 pc = arch_pc(ps.tid);

	if (ps.opened) {
		debug_read_at(ps.tid, buf, 16, arch_pc(ps.tid));

		if (times<1) times = 1;
		for (;times;times--){
			len = arch_aop(pc, buf, &aop);
			if (len >0) {
				pc+=len;
				arch_jmp(pc);
			} else {
				eprintf("Unknown opcode\n");
				break;
			}
		}
	}

	return 0;
}

int debug_stepu(const char *arg)
{
	ut64 until = get_math(arg);
	unsigned long pc = arch_pc(ps.tid); //WS_PC();

	/* step until address */
	if (arg && arg[0] && until != 0) {
		radare_controlc();
		do {
			debug_step(1);
			ps.steps++;
			pc = arch_pc(ps.tid);
		} while (!config.interrupted && pc != until);

		return 0;
	}

	radare_controlc();

	/* step until user-code*/
	do {
		debug_step(1);
		ps.steps++;
		pc = arch_pc(ps.tid);
	} while (!config.interrupted && !is_usercode(pc) );

	if (config_get("trace.log"))
		trace_add((addr_t)pc);
	radare_controlc_end();
//	cons_printf("%d instructions executed\n", i);

	return 0;
}

int debug_stepo()
{
	addr_t pc = arch_pc(ps.tid); //WS_PC();
	char cmd[4];
	int skip;
	int bp_pos;

	debug_read_at(ps.tid, cmd, 4, (addr_t)pc);

	if ((skip = arch_is_stepoverable((const unsigned char *)cmd))) {
		if (config_get("trace.log"))
			trace_add((addr_t)pc);

		///  XXX  BP_SOFT with debug_bp_restore doesnt restores EIP correctly
		bp_pos = debug_bp_set(NULL, pc+skip, BP_HARD);

		debug_cont(0);
		debug_bp_rm_num(bp_pos);
		
//		debug_bp_restore();
		return 1;
	} else
		debug_step(1);

	if (config_get("trace.log"))
		trace_add((addr_t)arch_pc(ps.tid));

	return 0;
}

int debug_stepbp(int times)
{
	int bp0 = -1;
	int bp1 = -1;
	unsigned char bytes[32];
	struct aop_t aop; /* ,aop2; */
	int i, len; /* ,len2; */
	addr_t pc = arch_pc(ps.tid);

	debug_read_at(ps.tid,bytes, 32, pc);
	len = arch_aop(pc, bytes, &aop);
#if 0
	if (aop.jump) { // autoskip nopz for mipz
		debug_read_at(ps.tid,bytes, 32, pc+len);
		len2 = arch_aop(pc, bytes, &aop2);
		if (aop2.type==AOP_TYPE_NOP)
			len+= len2;
	}
#endif
	if (times<2) {
		if (aop.jump>0)
			bp0 = debug_bp_set(NULL, aop.jump, BP_SOFT);
		if (aop.fail>0)
			bp1 = debug_bp_set(NULL, aop.fail, BP_SOFT);
		if ((aop.fail == 0) || (bp0==-1 && bp1==-1))
			bp0 = debug_bp_set(NULL, pc+len, BP_SOFT);

		printf("jump 0x%08llx fail %08llx -> here %08llx\n", aop.jump, aop.fail, pc);
					//sleep(2);
		debug_cont(0);
		//debug_bp_restore();

		eprintf("%08llx %08llx %08llx\n", aop.jump, aop.fail, pc+len);
		if (bp0!=-1)
			debug_bp_rm_addr(aop.jump);
			//debug_bp_rm_num(bp0);
		if (bp1!=-1)
			debug_bp_rm_addr(aop.fail);
		debug_bp_rm_addr(pc+len);
	} else {
		ut64 ptr = pc;
		for(i=0;i<times;i++) {
			debug_read_at(ps.tid, bytes, 32, ptr);
			len = arch_aop(pc, bytes, &aop);
			ptr += len;
		}
		printf("Stepping %d opcodes using breakpoint at %08llx\n", times, ptr);
		bp0 = debug_bp_set(NULL, ptr, BP_SOFT);
		debug_cont(0);
		debug_bp_rm_addr(ptr);
	}
		//debug_bp_rm_num(bp1);

	return 0;
}

int debug_step(int times)
{
	unsigned char opcode[32];
	ut64 pc, off;
	ut64 old_pc = 0;
	const char *tracefile;
	const char *flagregs;
	int ret;

	if(times < 1)
		times = 1;

	/* restore breakpoint */
	if(debug_bp_restore(-1))
		times--;

	/* mips step->stepbp bypass hack for mips */
	if (!strcmp(config_get("asm.arch"), "mips"))
		return debug_stepbp(times);

	if (!ps.opened) {
		eprintf("No program loaded.\n");
		return 0;
	}

	/* Skip breakpoint event before first step */
	WS(event) = UNKNOWN_EVENT;

	if (ps.verbose) {
		for(;WS(event) == UNKNOWN_EVENT && times; times--) {
			if (debug_os_steps() == -1) {
				perror("debug_os_steps");
				return 0;
			}
			ret = debug_dispatch_wait();
			printf("DISPATH WAIT: %d\n", ret);
	trace_add((addr_t)pc);
	ps.steps++;
		}
		debug_print_wait("step");
	} else {
		flagregs = config_get("trace.cmtregs");
		tracefile = config_get("file.trace");

		/* accelerate soft stepoverables */
		for(;WS(event) == UNKNOWN_EVENT && times; times--) {
			pc = arch_pc(ps.tid);
			if (config_get("trace.log"))
				trace_add(pc);

			if (pc == old_pc) {
				debug_read_at(ps.tid, opcode, 32, (addr_t)pc);
				// determine infinite loop
			#if __i386__
				// XXX this is not nice!
				if (opcode[0]==0xeb && opcode[1]==0xef) {
					eprintf("step: Infinite loop detected.\n");
					return 0;
				}
			#endif
			}

			if (config_get("dbg.stepo") && (off=(addr_t)arch_is_soft_stepoverable((const u8*)opcode))) {
				debug_bp_set(NULL, pc+off, config_get_i("dbg.hwbp"));
				debug_cont(0);
				debug_bp_rm_addr(pc+off);
				ps.steps++;
			} else {
				if (debug_os_steps() == -1) {
					perror("debug_os_steps");
					return 0;
				}
				ps.steps++;
				ret = debug_dispatch_wait();
			//	printf("DISPATH WAIT 2: %d\n", ret);
			}

			if (flagregs) {
				char buf[1024];
				char *ptr;
			//	radare_cmd("!dregs", 0);
			//	config_set("scr.buf", "true");
				arch_print_registers(0, "line");
				ptr = cons_get_buffer();
				if(ptr[0])ptr[strlen(ptr)-1]='\0';
				sprintf(buf, "CC %d %s @ 0x%08llx", ps.steps, strget(ptr), pc);
			//	config_set("scr.buf", "false"); // XXX
				radare_cmd(buf, 0);
				ptr[0]='\0'; // reset buffer
				cons_flush();
			}

			if (tracefile) {
				int fd = open(tracefile, O_WRONLY|O_APPEND);
				if (fd == -1)
					fd = open(tracefile, O_CREAT|O_WRONLY, 0644);
				if (fd != -1) {
					char buf[1024];
// XXX dbg.tracefile doesnt works as expected :(
					//pprint_fd(fd);
					// TODO: store moar nfo here like opcode and !dregs
					sprintf(buf, "0x%08llx\n", pc);
					write(fd, buf, strlen(buf));
					close(fd);
				}
			}
			old_pc = pc;
	trace_add((addr_t)pc);
	ps.steps++;
		}
	}

	return (WS(event) != BP_EVENT);
}

int debug_trace(char *input)
{
	// TODO: file.trace ???
	// TODO: show stack and backtrace only when is different
	int counter = 0;
	int tbt = (int)config_get("trace.bt");
	long slip = (int)config_get_i("trace.sleep");
	int smart = (int)config_get("trace.smart");
	int tracebps = (int)config_get("trace.bps");
	int tracelibs = (int)config_get("trace.libs");
	int tracecalls = (int)config_get("trace.calls");
	char *cmdtrace = config_get("cmd.trace");
	int level = atoi(input+1);
	unsigned long pc;

	if (strchr(input,'?')) {
		printf("!trace [level]\n");
		printf("  0  no output\n");
		printf("  1  show addresses\n");
		printf("  2  address and disassembly\n");
		printf("  3  address, disassembly and registers\n");
		printf("  4  address, disassembly and registers and stack\n");
		printf(" > eval trace.calls = true ; only trace calls\n");
		printf(" > eval trace.smart = true ; smart output\n");
		printf(" > eval trace.bps = true   ; do not stop on breakpoints\n");
		printf(" > eval trace.libs = true  ; trace into libraries\n");
		printf(" > eval trace.bt = true    ; to show backtrace\n");
		printf(" > eval trace.sleep = 1    ; animated stepping (1fps)\n");
		printf(" > eval cmd.trace = x@eip  ; execute this cmd on every traced opcode\n");
		return 0;
	}

	/* default debug level */
	printf("Trace level: %d\n", level);
	//if (level == 0 && input[0]!='0')
	//	level = 2;

	radare_cmd(".!regs*",0);
	if (level>2) {
		radare_cmd("!regs",0);
		radare_cmd("px  64 @ esp",0);
	}

	while(!config.interrupted && ps.opened && debug_step(1)) {
		/* XXX : if bp in addr stop! */
		counter++;
		radare_controlc();
		ut64 pc = arch_pc(ps.tid);
		radare_cmd(".!regs*", 0);
		if (smart) {
			cons_printf("[-] 0x%08llx\n", arch_pc(ps.tid));
			radare_cmd("!dregs", 0);
			radare_cmd("pd 4 @ eip", 0);
			//disassemble(20, 2);
		} else {
			int show = !tracecalls;
			if (tracecalls) {
				char label[128];
				struct aop_t aop;
				char buf[32];
				ut64 addr = arch_pc(ps.tid);
				radare_read_at(addr, buf, 32);
				arch_aop(addr, buf, &aop);
				switch(aop.type) {
				case AOP_TYPE_CALL:
				case AOP_TYPE_RCALL:
					label[0] = '\0';
					string_flag_offset(label, aop.jump, 0);
					cons_printf("[>] call 0x%08llx ; %s\n", aop.jump, label);
					show = 1;
					break;
				}
			}

			if (show)
			switch(level) {
			case 1:
				radare_cmd("!dregs", 0);
			case 0:
				cons_printf("0x%08llx\n", arch_pc(ps.tid));
				break;
			case 5:
				cons_printf("[-] 0x%08llx\n", arch_pc(ps.tid));
				radare_cmd("!regs", 0);
			case 4:
				radare_cmd(".!regs*", 0);
				radare_cmd("px 64 @ esp",0);
			case 2:
			case 3:
				radare_cmd("pd 1 @ eip",0);
			default:
				pc = arch_pc(ps.tid);
				if (tracelibs || is_usercode(pc)) {
					//radare_seek((addr_t)pc, SEEK_SET);
					//radare_read(0);
					//disassemble(10,1);
					if (level >= 3) {
						debug_registers(0);
						cons_printf("\n");
					}
					if (tbt) {
						// XXX must be internal call
						radare_cmd("!bt", 0);
					}
					if (config.interrupted)
						break;
				}
				break;
			}
		}
		cons_flush();

		if (cmdtrace && cmdtrace[0]!='\0')
			radare_cmd_raw(cmdtrace, 0);

		if (slip)
			sleep(slip);
		if (tracebps)
			eprintf("BREAKPOINT TRACED AT 0x%08llx!\n", pc);
		if (debug_bp_get(pc) != NULL) {
			eprintf("Breakpoint!\n");
			break;
		}
	}
	eprintf("%d traced opcodes\n");

	radare_controlc_end();

	if (ps.opened==0)
		debug_load();

	return 0;
}

int debug_wp(const char *str)
{
	int i = 0;
	int key = str[0];

	if(ps.wps_n == sizeof(ps.wps)) {
		eprintf(":error	max watchpoints are 4!\n");
		return -1;
	}

	if (key==0 && ps.wps_n == 0)
		key='?';
	else
	if (str[0] == ' ')
		key = str[1];

	switch(key) {
	/* remove watchpoint */
	case '-':
		str = str+1;
		skip_chars(&str);
		i = atoi(str);
		if(i >= 0 && i <= 4) {
			if( debug_wp_rm (i))
				printf("watchpoint %d free\n", i);
		} else {
			eprintf(":error invalid watchpoint number\n");
			return -1;
		}
		break;
	/* print watchpoints */
	case '?':
		eprintf("Usage: !wp[?|*] ([expression|-idx])\n"
		"  !wp         ; list all watchpoints (limit = 4)\n"
		"  !wp*        ; remove all watchpoints\n"
		"  !wp -#      ; removes watchpoint number\n"
		" Expression example:\n"
		"  !wp %%eax = 0x8048393 and ( [%%edx] > 0x100 )\n");
/*
	TODO: implement wps (maps), wph (drx) --  this is done with !mp  KEEP IT SIMPLE!!
		printf("  !bps          software breakpoint\n");
		printf("  !bph          hardware breakpoint\n");
*/
		return 0;

	/* add a new watchpoint */
	default:
		skip_chars(&str);
		if(!*str) {
			if (sizeof(ps.wps)==0)
				eprintf("No watchpoints defined. Try !wp?\n");
			else debug_wp_list();
		} else {
			for(i = 0; i < sizeof(ps.wps); i++) {
				if(!ps.wps[i].cond) {
					ps.wps[i].cond = (void*) parse_cond(str);
					if(!ps.wps[i].cond) {
						eprintf("Invalid conditional string (%s)\n", str);
						return -1;
					}
					ps.wps[i].str_cond = strdup(str);
					ps.wps_n++;
					cons_printf("%d: %s\n", i, str);
					break;
				}
			}
			if (i == sizeof(ps.wps))
				eprintf("No free watchpoints slots. Sorry.\n");
		}
	}

	return 0;
}

int print_syscall()
{
	return arch_print_syscall();
}

/* continue until here! */
int debug_contuh(char *arg)
{
	int bp;
	addr_t off = arch_pc(ps.tid);
	debug_step(1);
	bp = debug_bp_set(NULL, off, BP_SOFT);
	debug_cont(0);
	debug_bp_restore(-1);
	debug_bp_rm_num(bp);
	return 0;
}

int debug_contsc(char *arg)
{
	int ret;
	int num = (int)get_math(arg+1);

	if (strchr(arg, '?')) {
		eprintf("Usage: !contsc <syscall-number|name>\n");
		debug_os_syscall_list();
		return 0;
	}

	if (!ps.opened) {
		eprintf(":contc No program loaded.\n");
		return -1;
	}

	/* TODO: str_to_int_syscall */
	if (num == 0)
		num = syscall_name_to_int(arg+1);

	if (num == 0 && arg[0]!='\0') {
		eprintf("Unknown syscall. Use '!contsc?' to get a list\n");
		return 0;
	}

	/* restore breakpoint */
	do {
		debug_bp_restore(-1);

		/* launch continue */
		debug_contscp();
		debug_dispatch_wait();
		/* should be aligned.. or so */
		if (config_get("dbg.contsc2")) {
			debug_contscp();
			debug_dispatch_wait();
		}

		/* print status */
		debug_print_wait(NULL);
		ret = print_syscall();
		/// XXX this code skips breakpoints!!

		// Oops detect cadavers here
		if (!ps.opened)
			break;
	} while (ret != -1 && arg[0] && num != ret );

	if (config_get("dbg.contscbt"))
		radare_cmd("!bt", 0);

	return 1;
}

int debug_inject_buffer(unsigned char *fil, int sz)
{
	int i;
	unsigned long eip = arch_pc(ps.tid); //WS_PC();
	unsigned char *ptr;

	// XXX make this work on ARM and MIPS
#warning debug_inject() only supports X86 architecture
	fil[sz]=0xcc; // breakpoint on X86 (XXX broken concept for non intel)
	for(i=0;i<sz+1;i++){
		if (!(i%5))eprintf("\n");
		eprintf("%02x", fil[i]);
	}
	eprintf("\n");

	ptr = (unsigned char *)malloc(sz);
	// backup code
	debug_read_at(ps.tid, ptr, sz, (addr_t)ps.entrypoint);
	// inject shellcode
	debug_write_at(ps.tid, fil, sz, (addr_t)ps.entrypoint);
	// execute it
	arch_jmp(ps.entrypoint);
	debug_cont(0);
	// restore
	arch_jmp(eip);
	debug_write_at(ps.tid, ptr, sz, (addr_t)eip);
	free(ptr);

	return 0;
}

int debug_inject(char *file)
{
	ut64 sz = 0;
	int fd = open(file, O_RDONLY);
	unsigned char *fil = NULL;

	if (fd == -1) {
		eprintf("Cannot open '%s' to inject\n", file);
		return 0;
	}

	lseek(fd, (off_t)0, SEEK_END); // + (addr_t)4;
	lseek(fd, (off_t)0, SEEK_SET);
	if (sz>0xffff) {
		eprintf("File too big\n");
		close(fd);
		return 0;
	}
	// read the code
	fil = (unsigned char *)malloc(sz);
	read(fd, fil, sz);

	debug_inject_buffer(fil, sz);
	eprintf("%d bytes injected and executed from %s\n", (unsigned int)sz-1, file);

	free(fil);
	close(fd);

	return 1;
}

/* XXX : Stops at the following opcode at this address...debug_bp_restore() doesnt works? */
int debug_cont_until(const char *input)
{
	ut64 addr = input?get_math(input):0;

	/* continue until address */
	if (addr != 0) {
		int bp;
		eprintf("Continue until (%s) = 0x%08llx\n", input, addr);
		bp = debug_bp_set(NULL, addr,config_get_i("dbg.hwbp"));
		debug_cont(NULL);
		debug_bp_restore(bp);
		debug_bp_rm_num(bp);
		return 1;
	} 
	return 0;
}

#define CONTINUE_IN_NEW_PROCESS() (WS(event) == CLONE_EVENT && config_get("dbg.forks"))
#define SKIP_USER_CONTROLC() (WS(event) == INT_EVENT && !config_get("dbg.controlc"))

int debug_cont(const char *input)
{
	int ret;

	if (!ps.opened) {
		eprintf("cont: No program loaded.\n");
		return -1;
	}

	if (debug_cont_until(input))
		return 0;

	/* restore breakpoint */
	debug_bp_restore(-1);

	/*
	 * TODO: must launch one thread per process to debug sending PTRACE_CONT
	 *  must keep the shell clean for the debugger
	 */

	do { 
		/* restore breakpoint */
		debug_bp_restore(0);

		/* launch continue */
		ret = debug_contp(ps.tid);
		if (ret == -1) {
			eprintf("debug_contp = -1\n");
			break;
		}
		ret = debug_dispatch_wait();
		//debug_msg_set("debug_dispatch_wait: RET = %d WS(event)=%d INT3_EVENT=%d INT_EVENT=%d CLONE_EVENT=%d\n", 
	//		ret, WS(event), INT3_EVENT, INT_EVENT, CLONE_EVENT);
		printf("debug_dispatch_wait: RET = %d WS(event)=%d INT3_EVENT=%d INT_EVENT=%d CLONE_EVENT=%d\n", 
			ret, WS(event), INT3_EVENT, INT_EVENT, CLONE_EVENT);
	#warning TODO: add in debug_print_foo() disassembly of eip here and change process id or so
		if (SKIP_USER_CONTROLC()) {
			eprintf("user control-c pressed\n");
			continue;
		}

		if (CONTINUE_IN_NEW_PROCESS()) {
			eprintf("continue after new process\n");
			continue;
		}
	} while( ret ); //ret && (debug_dispatch_wait() == 1 || CONTINUE_IN_NEW_PROCESS()));
		//(CONTINUE_IN_NEW_PROCESS()) && debug_contp(ps.tid) != -1))));

	/* print status */
	debug_print_wait("cont");

	return 0;
}

int debug_run(char *input)
{
	char *buf;
	if (input[0]) {
		//printf("INPUT IS: %s\n", input);
		free(ps.filename);
		buf = (char *)malloc(strlen(ps.args)+strlen(input)+5);
		if (buf == NULL) {
			eprintf("Cannot malloc?\n");
			return 1;
		} 
		strcpy(buf, ps.args);
		strcat(buf, input);
		printf("DebugLoad(%s)\n", buf);
		return debug_loaduri(buf);
	} else
	if (ps.opened) {
		if (getv())
			eprintf("To cleanly stop the execution, type: "
				       "\"^Z kill -STOP %d && fg\"\n", ps.tid);
		return debug_cont(0);
	}

	eprintf("No program loaded.\n");
	return 1;
}

/* do loop */
int debug_loop(char *addr_str)
{
	int ret;
	addr_t pc, ret_addr;
	struct bp_t *bp;
      
        if(addr_str == (char *)NULL)
		return -1;

	ret_addr = get_offset(addr_str);
	pc = arch_pc(ps.tid);

	/* if exist a breakpoint at PC address then delete it */
	bp = debug_bp_get(pc);
	if(bp != (struct bp_t *)NULL)
		debug_bp_rm_addr(pc);

	/* set HW breakpoint on return address */
	if(debug_bp_set(NULL, ret_addr, BP_HARD) < 0) {
		eprintf("Cannot set hw on return address(0x%x)\n",
				ret_addr);
		return -1;
	}

	do { debug_cont(0);
		/* matched ret address or fatal exception */
	} while(!(WS(event) == BP_EVENT && WS(bp)->addr == ret_addr) && 
		  WS(event) != FATAL_EVENT && WS(event) != EXIT_EVENT &&
		  WS(event) != INT_EVENT);

	/* get return code */
	switch(WS(event)) {
	case FATAL_EVENT:
		ret = 3;
		break;
	case EXIT_EVENT:
		ret = 2;
		break;
	case INT_EVENT:
		ret = 1;
		break;
	case BP_EVENT:
		ret = 0;
		break;
	default:
		ret = -2;
	}

	/* remove HW bp */
	debug_bp_rm_addr(ret_addr);

	/* set pc */
	arch_jmp(pc);

	return ret;
}

int debug_inject2(char *input)
{
	ut64 pc, size;
	int status;
	char buf[1024];
	char *bak;
	char *file, *sym, *ptr, *ptr2 = input;

	ptr = strchr(ptr2, ' ');
	if (!ptr)
		return 0;
	file = ptr2 = ptr + 1;
	ptr = strchr(ptr2, ' ');
	if (!ptr)
		return 0;
	ptr[0] = '\0';
	sym = ptr + 1;
	snprintf(buf, 1024, "!!gcc -c -o .file.o %s", file);
	radare_cmd_raw(buf, 0);
	snprintf(buf, 1024, "!!rsc syms-dump .file.o | grep %s | sed 's/.* //' > .file.hex", sym);
	radare_cmd_raw(buf, 0);
	radare_cmd_raw("!!echo 'cc90' >> .file.hex", 0);
	size = file_size(".file.hex");
	bak = malloc(size);
	if (!bak) {
		radare_cmd_raw("!!rm -f .file.hex .file.o", 0);
		return 0;
	}
	pc = arch_pc(ps.tid);
	debug_read_at(ps.tid, bak, size, pc);
	snprintf(buf, 1024, "s 0x%08llx && wF .file.hex", pc);
	radare_cmd_raw(buf, 0);
	radare_cmd_raw("!!rm -f .file.hex .file.o", 0);
	debug_contp(ps.tid);
	debug_waitpid(ps.tid, &status);
	debug_write_at(ps.tid, (unsigned char*)bak, size, pc);
	free(bak);
	arch_jmp(pc);

	return 1;
}
