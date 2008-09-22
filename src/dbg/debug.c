/*
 * Copyright (C) 2007, 2008
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

extern struct regs_off roff[];

int debug_alloc_status();

void debug_dumpcore()
{
#if __NetBSD__
	ptrace(PT_DUMPCORE, ps.tid, NULL, 0);
#else
	eprintf("Not supported for this platform\n");
#endif
}

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
	//snprintf(buf, 1022, ".!!rsc syms-dbg-flag '%s'", config.file);
	snprintf(buf, 1022, ".!!rabin -ris '%s'", config.file);
	return radare_cmd_raw(buf, 0);
}

// TODO : helper
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

int debug_tt(const char *arg)
{
	struct aop_t aop;
	int pid;
	u8 tmp[4];
	u64 pc; // program counter
	u8 *sa; // swap area
	u8 *cc; // breakpoint area
	u64 ba = config.seek; // base address
	u64 sz = get_math(arg+1); // size
	int status;
	if (sz <1) {
		cons_printf("Usage: !tt [size] @ [base_address]\n");
		cons_printf("Touch trace a section of N bytes starting at seek\n");
		return 0;
	}
	cons_printf("TouchTracing %lld bytes from 0x%08llx..\n", sz, ba);
	cons_flush();
	/* */
	radare_controlc();
	sa = (u8 *)malloc(sz);
	cc = (u8 *)malloc(sz);
	debug_read_at(ps.tid, sa, (int)sz, ba);
	/* TODO: make it depend on asm.arch */
	/* TODO: use memcpy_loop with get_arch_breakpoint() or whatever */
	memset(cc, '\xCC', sz);
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
			int bpsz = 1; // XXX for intel
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
}

/// XXX looks wrong
/// XXX use wait4 and get rusage here!!!
/// XXX move to dbg/unix
int debug_waitpid(int pid, int *status)
{
#define CRASH_LINUX_KERNEL 0
#if CRASH_LINUX_KERNEL
	if (pid == -1)
		return -1;
#endif

#if __WINDOWS__
	/* not implemented ? */
#elif __FreeBSD__
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
}
#endif

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

int hijack_fd(int fd, const char *file)
{
	int f;

	if (strnull(file) || fd==-1)
		return -1;

#if __UNIX__
	f = open(file, (fd?O_RDWR:O_RDONLY) | O_NOCTTY);
#else
	f = open(file, (fd?O_RDWR:O_RDONLY));
#endif
	// TODO handle pipes to programs
	// does not works
	if (f == -1) {
		f = open(file, (fd?O_RDWR:O_RDONLY)|O_CREAT ,0644);
		if (f == -1) {
			eprintf("Cannot open child.uh '%s'\n", file);
			return -1;
		}
	}
	close(fd);
	dup2(f, fd);

	return fd;
}

void debug_environment()
{
	const char *ptr;

	// TODO proper environment handle
	if (getv()) {} //eprintf("TODO: load environment from environ.txt or so.\n");

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
		hijack_fd(0, config_get("child.stdio"));
		hijack_fd(1, config_get("child.stdio"));
		hijack_fd(2, config_get("child.stdio"));
	} else {
		hijack_fd(0, config_get("child.stdin"));
		hijack_fd(1, config_get("child.stdout"));
		hijack_fd(2, config_get("child.stderr"));
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
	} else {
		eprintf("Invalid value for dbg.bttype. Use 'standard', 'old' or 'st'\n");
	}

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

int debug_stepret()
{
	char bytes[4];
	struct aop_t aop;

	/* make steps until esp = oesp and [eip] == 0xc3 */
	radare_controlc();
	while(!config.interrupted && ps.opened && debug_stepo()) {
		debug_read_at(ps.tid, bytes, 4, arch_pc(ps.tid));
		arch_aop((u64)arch_pc(ps.tid), (const u8 *)bytes, &aop);
		if (aop.type == AOP_TYPE_RET)
			break;
	}
	radare_controlc_end();

	return 0;
}

int debug_contu(const char *input)
{
	int is_user;

	if (debug_cont_until(input))
		return 0;

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
	u64 off = 0LL;
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
		eprintf("entry at: 0x%x\n", (unsigned int)ps.entrypoint);
		sprintf(buf, "0x%x\n", (unsigned int)off);
		debug_cont_until(buf);
	}

	if (!strcmp("main", addr)) {
		// XXX intel only
		// XXX BP_SOFT is ugly (linux supports DRX HERE)
#if 1
		debug_read_at(ps.tid, buf, 12, arch_pc(ps.tid));
		if (!memcmp(buf, "\x31\xed\x5e\x89\xe1\x83\xe4\xf0\x50\x54\x52\x68", 12)) {
			debug_read_at(ps.tid, &ptr, 4, arch_pc()+0x18);
			off = (u64)(unsigned int)(ptr[0]) | (ptr[1]<<8) | (ptr[2] <<16) | (ptr[3]<<24);
			sprintf(buf, "0x%x", (unsigned int)off);
			printf("== > main at : %s\n", buf);
			debug_cont_until(buf);
		} else
		if (!memcmp(buf, "^\x89\xe1\x83\xe4\xf0PTRh", 10)) {
			unsigned int addr;
			debug_read_at(ps.tid, &addr, 4, arch_pc(ps.tid)+0x16);
			off = (addr_t)addr;
			sprintf(buf, "0x%x", addr);
			printf("== > main at : %s\n", buf);
			debug_cont_until(buf);
		} else
			eprintf("Cannot find main analyzing the entrypoint. Try harder.\n");
#endif
	}

	if (config_get("dbg.syms"))
		debug_system("syms");

	return 0;
}

int debug_mmap(char *args)
{
	char *arg;
	char *file = args + 1;
	addr_t addr;
	addr_t size;
#if 0
Dump of assembler code for function mmap:
0xb7ec0110 <mmap+0>:    push   %ebp
0xb7ec0111 <mmap+1>:    push   %ebx
0xb7ec0112 <mmap+2>:    push   %esi
0xb7ec0113 <mmap+3>:    push   %edi
0xb7ec0114 <mmap+4>:    mov    0x14(%esp),%ebx
0xb7ec0118 <mmap+8>:    mov    0x18(%esp),%ecx
0xb7ec011c <mmap+12>:   mov    0x1c(%esp),%edx
0xb7ec0120 <mmap+16>:   mov    0x20(%esp),%esi
0xb7ec0124 <mmap+20>:   mov    0x24(%esp),%edi
0xb7ec0128 <mmap+24>:   mov    0x28(%esp),%ebp
0xb7ec012c <mmap+28>:   test   $0xfff,%ebp
0xb7ec0132 <mmap+34>:   mov    $0xffffffea,%eax
0xb7ec0137 <mmap+39>:   jne    0xb7ec0143 <mmap+51>
0xb7ec0139 <mmap+41>:   shr    $0xc,%ebp
0xb7ec013c <mmap+44>:   mov    $0xc0,%eax
0xb7ec0141 <mmap+49>:   int    $0x80
#endif

	if (!ps.opened) {
		eprintf(":signal No program loaded.\n");
		return 1;
	}

	if(!args)
		return debug_alloc_status();

	if ((arg = strchr(file, ' '))) {
		arg[0]='\0';
		addr = get_math(arg+1);
		size = 0; // TODO: not yet implemented
		if (strchr(arg+1, ' '))
			eprintf("TODO: optional size not yet implemented\n");
		//signal_set(signum, address);
		mmap_tagged_page(file, addr, size);
	} else {
		eprintf("Usage: !mmap [file] [address] ([size])\n");
		return 1;
	}

	return 0;
}

u64 debug_alloc(char *args)
{
	int sz;
	char *param;
	addr_t addr;

	if(!args)
		return debug_alloc_status();

	param = args + 1;
	if(!strcmp(param, "status")) {
		print_status_alloc();
	} else {
		sz = get_math(param);
		if(sz <= 0) {
			eprintf(":alloc invalid size\n");
			return -1;
		}

		addr = alloc_page(sz);
		if(addr == (addr_t)-1) {
			eprintf(":alloc can not alloc region!\n");
			return -1;
		}
		printf("0x%08x\n", (unsigned int)addr);
	}

	return addr;
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

int debug_fd(char *cmd)
{
	char *ptr  = NULL,
	     *ptr2 = NULL,
	     *ptr3 = NULL;
	int whence = 0;

	if (cmd[0]=='?') {
		cons_printf("Usage: !fd[s|d] [-#] [file | host:port]\n"
		"  !fd                   ; list filedescriptors\n"
		"  !fdd 2 7              ; dup2(2, 7)\n"
		"  !fds 3 0x840          ; seek filedescriptor\n"
		"  !fd -1                ; close stdout\n"
		"  !fd /etc/motd         ; open file at fd=3\n"
		"  !fd 127.0.0.1:9999    ; open socket at fd=5\n");
		return 0;
	}

	if ((ptr=strchr(cmd,'-'))) {
		debug_fd_close(ps.tid, atoi(ptr+1));
	} else
	if (cmd[0]=='s') {
		ptr = strchr(cmd, ' ');
		if (ptr)
			ptr2 = strchr(ptr+1, ' ');
		if (ptr2)
			ptr3 = strchr(ptr2+1, ' ');
		if (!ptr||!ptr2) {
			eprintf("Usage: !fds [fd] [seek] ([whence])\n");
		} else {
			if (ptr3)
				whence = atoi(ptr3+1);

			printf("curseek = %08x\n", 
			(unsigned int)
			debug_fd_seek(ps.tid, atoi(ptr+1), get_math(ptr2+1), whence));
		}
	} else
	if (cmd[0]=='d') {
		ptr = strchr(cmd, ' ');
		if (ptr)
		ptr2 = strchr(ptr+1, ' ');
		if (!ptr||!ptr2)
			eprintf("Usage: !fdd [oldfd] [newfd]\n");
		else
			debug_fd_dup2(ps.tid, atoi(ptr+1), atoi(ptr2+1));
	} else
	if ((ptr=strchr(cmd, ' '))) {
		ptr = ptr+1;
		if (ptr)
		ptr2 = strchr(ptr, ':');
		if (ptr2)
			eprintf("SOCKET NOT YET\n");
		else
			eprintf("new fd = %d\n", debug_fd_open(ps.tid, ptr, 0));
	} else
		debug_fd_list(ps.tid);

	return 0;
}

int debug_loaduri(char *cmd)
{
	char pids[128];
	ps.filename = cmd;
	debug_load();
}

int debug_load()
{
	int ret = 0;
	char pids[128];

	if (ps.pid!=0) {
		/* TODO: check if pid is still running */
		// use signal(0) to check if its already there
		/* TODO: ask before kill */
	//	if (ps.is_file)
	//		debug_os_kill(ps.tid, SIGKILL);
	//	else return 0;
	}

	WS(event)   = UNKNOWN_EVENT;
	ps.bin_usrcode = NULL;

	ps_parse_argv();

	if (ps.argv[0]== NULL) {
		eprintf("No argv[0] available\n");
		return 1;
	}
	ps.is_file = !atoi(ps.filename);
	if (ps.is_file) 
		ret = debug_fork_and_attach();
	else
		ret = debug_attach(atoi(ps.filename));

	ps.entrypoint = arch_get_entrypoint();
	ps.th_active = 0;

	sprintf(pids, "%d", ps.tid);
	setenv("DPID", pids, 1);
	debug_init_maps(0);
	events_init();
	debug_until( config_get("dbg.bep") );

	debug_getregs(ps.pid, &(WS(regs)));

	return ret;
}

int debug_unload()
{
	ps.opened = 0;
#if __UNIX__
	debug_os_kill(ps.tid, SIGKILL);
#endif
	ps.pid = ps.tid = 0;

	return 0; //for warning message
}

#define debug_read_raw(x,y) ptrace(PTRACE_PEEKTEXT, x, y, 0)

#define ALIGN_SIZE 4096

int debug_read(pid_t pid, void *addr, int length)
{
	if (length<0)
		return -1;
	return debug_read_at(pid, addr, length, ps.offset);
}

int debug_write(pid_t pid, void *data, int length)
{
	return debug_write_at(ps.tid, data, length, ps.offset);
}

int debug_skip(int times)
{
	unsigned char buf[16];
	struct aop_t aop;
	int len;
	u64 pc = arch_pc(ps.tid);

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

int debug_stepu()
{
	unsigned long pc = arch_pc(ps.tid); //WS_PC();
	int i;

	radare_controlc();

	do {
		debug_step(1);
		ps.steps++;
		pc = arch_pc(ps.tid);
		i++;
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
		//debug_bp_rm_num(bp_pos);
		
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
			printf("jump %08llx fail %08llx -> here %08llx\n", aop.jump, aop.fail, pc+8);
					//sleep(2);

		debug_cont(0);
		//debug_bp_restore();

		printf("%08llx %08llx %08llx\n", aop.jump, aop.fail, pc+len);
		if (bp0!=-1)
			debug_bp_rm_addr(aop.jump);
			//debug_bp_rm_num(bp0);
		if (bp1!=-1)
			debug_bp_rm_addr(aop.fail);
		debug_bp_rm_addr(pc+len);
	} else {
		u64 ptr = pc;
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
	u64 pc, off;
	u64 old_pc = 0;
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
		}
		debug_print_wait("step");
	} else {
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

			if ((off=(addr_t)arch_is_soft_stepoverable((const u8*)opcode))) {
				debug_bp_set(NULL, pc+off, config_get("dbg.hwbp"));
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

			flagregs = config_get("trace.cmtregs");
			if (flagregs) {
				char buf[1024];
				char *ptr;
			//	radare_cmd("!dregs", 0);
				config_set("scr.buf", "true");
				arch_print_registers(0, "line");
				ptr = cons_get_buffer();
				if(ptr[0])ptr[strlen(ptr)-1]='\0';
				sprintf(buf, "CC %d %s @ 0x%08llx", ps.steps, strget(ptr), pc);
				config_set("scr.buf", "false"); // XXX
				radare_cmd(buf, 0);
				ptr[0]='\0'; // reset buffer
			}

			tracefile = config_get("file.trace");
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
		}
	}
	trace_add((addr_t)pc);
	ps.steps++;

	return (WS(event) != BP_EVENT);
}

int debug_set_register(const char *args)
{
	char *value, *tmp;

	if (!args) {
		eprintf("Usage: !set [reg] [value]\n");
		return 1;
	}
	if (!ps.opened) {
		eprintf(":regs No program loaded.\n");
		return 1;
	}
	value = strchr(args, '=');
	if (!value)
		value = strchr(args, ' ');
	if (!value) {
		eprintf("Usage: !set [reg] [value]\n");
		eprintf("  > !set eflags PZTI\n");
		eprintf("  > !set r0 0x33\n");
		return 1;
	}
	value[0]='\0';
	args = strclean(args);
	value = strclean(value+1);
	printf("%s=%s\n", args, value);

	return arch_set_register(args, value);
}

int debug_fpregisters(int rad)
{
	if (!ps.opened) {
		eprintf(":fpregs No program loaded.\n");
		return 1;
	}

	if (rad)
		cons_printf("fs fpregs\n");
	return arch_print_fpregisters(rad, "");
}

int debug_dregisters(int rad)
{
	if (!ps.opened) {
		eprintf(":regs No program loaded.\n");
		return 1;
	}

	return arch_print_registers(rad, "line");
}

int debug_oregisters(int rad)
{
	if (!ps.opened) {
		eprintf(":regs No program loaded.\n");
		return 1;
	}

	return arch_print_registers(rad, "orig");
}

int debug_registers(int rad)
{
	int fs, ret = 1;

	if (!ps.opened) {
		eprintf(":regs No program loaded.\n");
	} else {
		fs = flag_space_idx;
		flag_space_set("regs");
		ret = arch_print_registers(rad, "");
		flag_space_idx = fs;
	}
	return ret;
}

int debug_trace(char *input)
{
	// TODO: file.trace ???
	// TODO: show stack and backtrace only when is different
	int tbt = (int)config_get("trace.bt");
	long slip = (int)config_get_i("trace.sleep");
	int smart = (int)config_get("trace.smart");
	int tracelibs = (int)config_get("trace.libs");
	int level = atoi(input+1);
	unsigned long pc;

	if (strchr(input,'?')) {
		printf("!trace levels\n");
		printf("  0  no output\n");
		printf("  1  show addresses\n");
		printf("  2  address and disassembly\n");
		printf("  3  address, disassembly and registers\n");
		printf("  4  address, disassembly and registers and stack\n");
		printf("  > eval trace.bt = true ; to show backtrace\n");
		printf("  > eval trace.sleep = 1 ; animated stepping\n");
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

	radare_controlc();
	while(!config.interrupted && ps.opened && debug_step(1)) {
		radare_cmd(".!regs*", 0);
		if (smart) {
			cons_printf("[-] 0x%08llx\n", arch_pc(ps.tid));
			radare_cmd("!dregs", 0);
			radare_cmd("pd 4 @ eip", 0);
			//disassemble(20, 2);
		} else {
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
		if (slip)
			sleep(slip);
	}

	radare_controlc_end();

	if (ps.opened==0)
		debug_load();

	return 0;
}

int debug_bp_rms()
{
	int i;
	int ret, bps;

	bps = ps.bps_n;

	for(i = 0; i < MAX_BPS && ps.bps_n > 0; i++) {
		if(ps.bps[i].addr > 0) { 
		        if(ps.bps[i].hw)
                		ret = arch_bp_rm_hw(&ps.bps[i]);
        		else	ret = arch_bp_rm_soft(&ps.bps[i]);

			if(ret < 0) {
				eprintf(
					":debug_bp_rms error removing bp 0x%x\n",
					ps.bps[i].addr);
				break;
			}

			ps.bps[i].addr = 0;
			ps.bps_n--;
		}
	}

	return bps - ps.bps_n;
}

/* memory protection permissions */
struct mp_t {
	u64 addr;
	unsigned int size;
	int perms;
	struct list_head list;
};

static int mp_is_init = 0;

struct list_head mps;

// TODO: support to remove ( store old page permissions )
// TODO: remove overlapped memory map changes
int debug_mp(char *str)
{
	struct list_head *i;
	struct mp_t *mp;
	char buf[128];
	char buf2[128];
	char buf3[128];
	char *ptr = buf;
	u64 addr;
	u64 size;
	int perms = 0;

	// TODO: move this to debug_init .. must be reinit when !load is called
	if (!mp_is_init) {
		INIT_LIST_HEAD(&mps);
		mp_is_init = 1;
	}

	if (str[0]=='\0') {
		list_for_each(i, &(mps)) {
			struct mp_t *m = list_entry(i, struct mp_t, list);
			cons_printf("0x%08llx %d %c%c%c\n", m->addr, m->size,
			m->perms&4?'r':'-', m->perms&2?'w':'-', m->perms&1?'x':'-');
		}
		return 0;
	}

	if (strchr(str, '?')) {
		cons_printf("Usage: !mp [rwx] [addr] [size]\n");
		cons_printf("  > !mp       - lists all memory protection changes\n");
		cons_printf("  > !mp --- 0x8048100 4096\n");
		cons_printf("  > !mp rwx 0x8048100 4096\n");
		cons_printf("- addr and size are aligned to memory (-=%%4).\n");
		return 0;
	}

	sscanf(str, "%127s %127s %127s", buf, buf2, buf3);
	addr = get_math(buf2);
	size = get_math(buf3);

	if (size == 0) {
		eprintf("Invalid arguments\n");
		return 1;
	}

	for(ptr=buf;ptr[0];ptr=ptr+1) {
		switch(ptr[0]) {
		case 'r': perms |=4; break;
		case 'w': perms |=2; break;
		case 'x': perms |=1; break;
		}
	}

	// align to bottom
	addr = addr - (addr%4);
	size = size + (size-(size%4));

	mp = (struct mp_t*)malloc(sizeof(struct mp_t));
	mp->addr  = addr;
	mp->size  = (unsigned int)size;
	mp->perms = perms;
	list_add_tail(&(mp->list), &(mps));
	
	arch_mprotect((addr_t)mp->addr, mp->size, mp->perms);

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
		"  !wp           list all watchpoints (limit = 4)\n"
		"  !wp*          remove all watchpoints\n"
		"  !wp -#        removes watchpoint number\n"
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
			if (sizeof(ps.wps)==0) {
				eprintf("No watchpoints defined. Try !wp?\n");
			} else
				debug_wp_list();
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
	u64 sz = 0;
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
	u64 addr = input?get_math(input):0;

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

int debug_pids()
{
#if __UNIX__
	int i, fd;
	int n = 0;
	char cmdline[1025];

	// TODO: use ptrace to get cmdline from esp like tuxi does
	for(i=2;i<999999;i++) {
		switch( debug_os_kill(i, 0) ) {
		case 0:
			sprintf(cmdline, "/proc/%d/cmdline", i);
			fd = open(cmdline, O_RDONLY);
			cmdline[0] = '\0';
			if (fd != -1) {
				read(fd, cmdline, 1024);
				cmdline[1024] = '\0';
				close(fd);
			}
			printf("%d %s\n", i, cmdline);
			n++;
			break;
//		case -1:
//			if (errno == EPERM)
//				printf("%d [not owned]\n", i);
//			break;
		}
	}
	return n;
#else
	return -1;
#endif
}

u64 debug_get_regoff(regs_t *regs, int off)
{
	char *c = (char *)regs;
	debug_getregs(ps.tid, regs);
	return *(unsigned long *)(c + off);	
}

void debug_set_regoff(regs_t *regs, int off, unsigned long val)
{
	char *c = (char *)regs;
	*(unsigned long *)(c + off) = val;	

#if __WINDOWS__
	regs->ContextFlags = CONTEXT_FULL;
#endif
	debug_setregs(ps.tid, regs);
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

u64 debug_get_register(char *input)
{
	char *reg = input;
	int off;
	u64 ret;

	// TODO: user streclean
	if(*input == ' ')
		reg = input + 1;

	if((off = get_reg(reg)) == -1)
		return -1;

	/* TODO: debug_get_regoff must return a value addr_t */
	ret = (u64)debug_get_regoff(&WS(regs), roff[off].off); 

	printf("0x%08llx\n", ret);

	return ret;
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

	do {
		/* continue execution */
		debug_cont(0);

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

/* hacky util for dumping pages without knowing anything about map pages */
int debug_dumpall(const char *ptr)
{
	char file[128];
	FILE *fd = NULL;
	int ret,i=0;
	char buf[4096];
	u64 from=config.seek;
	u64 to = config.limit;
	if (to <= 0)
		to = 0xffffffff;
	from &= 0xfffffff0; // align hack
	cons_printf("Dumping from 0x%08llx to 0x%08llx...\n", from, to);

	radare_controlc();
	while(!config.interrupted) {
		ret = debug_read_at(ps.pid, buf, 4096, from);
		from += 4096;
		if (0==(i++%30)) printf("0x%08llx: %d    \r", from, ret);
		if (ret<0) {
			if (fd != NULL) {
				fclose(fd);
				fd = NULL;
			}
			continue;
		}
		if (fd == NULL) {
			sprintf(file, "0x%08llx.dall", from);
			printf("\nNew section found at 0x%08llx\n", from);
			i = 0;
			fd = fopen(file, "w");
			if (fd == NULL) {
				eprintf("Cannot create '%s'\n", file);
				break;
			}
		}
		fwrite(buf, ret, 1, fd);
	}
	if (config.interrupted)
		eprintf("Dump interrupted at 0x%08llx\n", from);
	if (fd != NULL)
		fclose(fd);
	radare_controlc_end();
	return 0;
}
