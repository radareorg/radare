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
#include "../config.h"
#include "../code.h"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <dirent.h>
#if __UNIX__
#include <sys/wait.h>
#endif
#include <sys/types.h>
#if __linux__
#include <sys/prctl.h>
#include <linux/ptrace.h>
#endif
#include <sys/stat.h>
#include "wp.h"
#include "mem.h"
#include "thread.h"
#include "signal.h"

void debug_dumpcore()
{
#if __NetBSD__
	ptrace(PT_DUMPCORE, ps.tid, NULL, 0);
#else
	eprintf("Not supported for this platform\n");
#endif
}

int debug_syms()
{
	// XXX: native implementation
	radare_command(".!rsc syms-dbg-flag ${FILE}", 0);
	return 0; //for warning message
}

// TODO: move to ps structure
int stepping_in_progress = 0;

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

void signal_step(int sig) // XXX sig does nothing?!?!
{
	stepping_in_progress = 0;
	eprintf("\n\n");
	fflush(stderr);
	signal(SIGINT, SIG_IGN);
}

/// XXX looks wrong
/// XXX use wait4 and get rusage here!!!
pid_t debug_waitpid(int pid, int *status)
{
#ifdef __WCLONE
	return waitpid(pid, status, __WALL | __WCLONE | WUNTRACED);
#ifdef __WALL
	return waitpid(pid, status, __WALL | WUNTRACED);
#else
	return waitpid(pid, status, WUNTRACED);
#endif
#endif
}

int debug_init()
{
	// TODO use memcpy
	ps.tid      =  0;
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
		print_maps_regions(strchr(arg, '*'));
		return 0;
	}

	return -1;
}

int hijack_fd(int fd, const char *file)
{
	int f;

	if (strnull(file)||fd==-1)
		return 0;

	f = open(file, fd?O_RDWR:O_RDONLY);
	// TODO handle pipes to programs
	if (f == -1) {
		eprintf("Cannot open child.stdin '%s'\n", file);
	}
	close(fd);
	dup2(f, 0);
}

void debug_environment()
{
	FILE *fd;
	int p;
	char buf[1204];
	char *ptr;
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
	if (!strnull(ptr)) chdir(p);
#if __UNIX__
	ptr = config_get("child.chroot");
	if(!strnull(ptr)) chroot(ptr);
	ptr = config_get("child.setuid");
	if (!strnull(ptr)) setuid(atoi(ptr));
	ptr = config_get("child.setgid");
	if (!strnull(ptr)) setgid(atoi(ptr));
#endif
	// TODO: add suid bin chmod 4755 ${FILE}
	hijack_fd(0, config_get("child.stdin"));
	hijack_fd(1, config_get("child.stdout"));
	hijack_fd(2, config_get("child.stderr"));
}

void debug_reload_bps()
{
        int bps;
        int i;

	// XXX must free the lists before
        INIT_LIST_HEAD(&(ps.th_list));       
        INIT_LIST_HEAD(&(ps.map_mem));
        INIT_LIST_HEAD(&(ps.map_reg));

        bps = 0;
        for(i = 0; bps != ps.bps_n; i++) {
                if(ps.bps[i].addr != 0) {
                        if(ps.bps[i].hw) {
                                if(arch_set_bp_hw(&ps.bps[i], ps.bps[i].addr) < 0)
                                	eprintf(":debug_reload_bps failed "
                                        	"set breakpoint HARD at 0x%x\n",
						 ps.bps[i].addr);
			} else {
				if(arch_set_bp_soft(&ps.bps[i], ps.bps[i].addr) != 0)
                                	eprintf(":debug_reload_bps failed "
                                        	"set breakpoint SOFT at 0x%x\n",
						 ps.bps[i].addr);
                        }

                        bps++;
                }
        }
}

int debug_bt()
{
	char *type = config_get("dbg.bttype");
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

int is_code(unsigned long pc)
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

int is_usercode(unsigned long pc)
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
	unsigned char bytes[4];
	struct aop_t aop;

	/* make steps until esp = oesp and [eip] == 0xc3 */
	signal(SIGINT, &signal_step);
	stepping_in_progress = 1;
	while(stepping_in_progress && ps.opened && debug_stepo()) {
		debug_read_at(ps.tid, bytes, 4, arch_pc());
		arch_aop((unsigned long) arch_pc(), bytes, &aop);
		if (aop.type == AOP_TYPE_RET)
			break;
	}
	stepping_in_progress = 0;
	signal(SIGINT, SIG_IGN);

	return 0;
}

int debug_contu()
{
	signal(SIGINT, &signal_step);
	stepping_in_progress = 1;

	ps.verbose = 0;
	while(stepping_in_progress && ps.opened && debug_step(1)) {
		if (is_usercode(arch_pc()))
			return 0;
	}
	ps.verbose = 1;

	stepping_in_progress = 0;
	signal(SIGINT, SIG_IGN);

	return 0;	
}

int debug_wtrace()
{
	int ret;

	/* not watchpoints, nothing */
	if(ps.wps_n == 0) {
		printf("not watchpoint to trace\n");
		return 0;
	}

        signal(SIGINT, &signal_step);
        stepping_in_progress = 1;

        ps.verbose = 0;
        while(stepping_in_progress && ps.opened && debug_step(1)) {
		ret = wp_matched();
		if(ret >= 0) {
			printf("watchpoint %d matched at 0x%x\n",
				 ret, arch_pc());		
			break;
		}
        }
        ps.verbose = 1;

        stepping_in_progress = 0;
        signal(SIGINT, SIG_IGN);

        return 0;
}

int debug_ie(char *input)
{
	if (!input)
		return;

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
}

int debug_until(char *addr)
{
	int bp_pos;
	unsigned long ptr;
	char buf[12];
	off_t off;

	if (!addr)
		return 0;

	ps.entrypoint = arch_get_entrypoint();

	off = 0;
	if ((!strcmp("entry", addr)
	|| !strcmp("main", addr)))
		off = ps.entrypoint;
	else	off = get_math(addr);

	if (off != 0) {
		eprintf("entry at: 0x%x\n", ps.entrypoint);
		bp_pos = debug_set_bp(NULL, off, BP_SOFT);
		debug_cont();
		debug_rm_bp_num(bp_pos);
		restore_bp();
	}

	if (!strcmp("main", addr)) {
		// XXX intel only
		// XXX BP_SOFT is ugly (linux supports DRX HERE)
		debug_read_at(ps.tid, buf, 12, arch_pc());
		if (!memcmp(buf, "\x31\xed\x5e\x89\xe1\x83\xe4\xf0\x50\x54\x52\x68", 12)) {
			debug_read_at(ps.tid, &ptr, 4, arch_pc()+0x18);
			off = (off_t)ptr;
			bp_pos = debug_set_bp(NULL, off, BP_SOFT);
			debug_cont();
			debug_rm_bp_num(bp_pos);
			restore_bp();
			//arch_jmp(arch_pc()-1); // XXX only x86
		} else
		if (!memcmp(buf, "^\x89\xe1\x83\xe4\xf0PTRh", 10)) {
			unsigned int addr;
			debug_read_at(ps.tid, &addr, 4, arch_pc()+0x16);
			off = (off_t)addr;
			bp_pos = debug_set_bp(NULL, addr, BP_SOFT);
			printf("main at: 0x%x\n", addr);
			debug_step(1);
			debug_cont();
			debug_rm_bp_num(bp_pos);
			restore_bp();
		}
	}

	if (config_get("dbg.syms"))
		debug_system("syms");

	return 0;
}

int debug_mmap(char *args)
{
	char *arg;
	char *file = args + 1;
	off_t addr;
	off_t size;
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
	if (!args) {
		eprintf("Usage: !mmap [file] [address] ([size])\n");
		return 0;
	}

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

int debug_alloc(char *args)
{
	int sz;
	char *param;
	void *addr;

	if(!args)
		return debug_alloc_status();

	param = args + 1;
	if(!strcmp(param, "status")) {
		print_status_alloc();
	} else {
		sz = get_math(param);
		if(sz <= 0) {
			eprintf(":alloc invalid size\n");
			return 1;
		}

		addr = (void *)alloc_page(sz);
		if(addr == (void *)-1) {
			eprintf(":alloc can not alloc region!\n");
			return 1;
		}
		printf("* new region allocated at: 0x%x\n", addr);
	}

	return 0;
}

int debug_free(char *args)
{
	long addr;

	if(!args) {
		eprintf(":free invalid address\n");
		return 1;
	}

	addr = get_math(args + 1);

	if(dealloc_page((void *)addr) == 0)
		eprintf(":free 0x%x region not found!\n", addr);
	else
		eprintf(":free region 0x%x\n", addr); 

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
		cons_printf(" filename %s\n", ps.filename);

		cons_printf(" pid         %d\n", ps.pid);
		sprintf(buf, "/proc/%d/cmdline", ps.pid);
		if ((fd = open(buf, O_RDONLY)) !=-1) {
			memset(buf,'\0',4096);
			read(fd, buf, 4095);
			cons_printf(" cmdline  %s\n", buf);
			close(fd);
		}

		// TODO is all this stuff necesary?
		cons_printf(" tid(current)%d\n", ps.tid);
		sprintf(buf, "/proc/%d/cmdline", ps.tid);
		if ((fd = open(buf, O_RDONLY)) !=-1) {
			memset(buf,'\0',4096);
			read(fd, buf, 4095);
			cons_printf(" cmdline  %s\n", buf);
			close(fd);
		}
		cons_printf(" dbg.bep     %s\n", config_get("dbg.bep"));
		cons_printf(" entry       0x%08x\n", ps.entrypoint);
		cons_printf(" ldentry     0x%08x\n", ps.ldentry);
		cons_printf(" opened      %d\n", ps.opened);
		cons_printf(" steps       %d\n", ps.steps);
		cons_printf(" offset      0x%llx\n", ps.offset);
		cons_printf(" isbpaddr    %d\n", ps.isbpaddr);
	}

	return 0;
}

int debug_th(char *cmd)
{
	int newpid = atoi(cmd);
	if (newpid !=0) {
		ps.tid = newpid;
	}
	if (strchr(cmd,'?')) {
		eprintf("Usage: th [pid]\n");
		eprintf("- Change current thread\n");
		eprintf("- Use !pstree to see other processes\n");
		return 0;
	} 
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

	if (ptr=strchr(cmd,'-')) {
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
	if (ptr=strchr(cmd, ' ')) {
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
	debug_fork_and_attach();
	printf("%d\n", ps.tid);
	exit(0);
}

int debug_load()
{
	int ret;
	char pids[128];
	stepping_in_progress = 0;

	if (ps.pid!=0) {
		/* TODO: check if pid is still running */
		// use signal(0) to check if its already there
		/* TODO: ask before kill */
	//	if (ps.is_file)
	//		kill(ps.tid, SIGKILL);
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
	debug_until(config_get("dbg.bep"));

	return ret;
}

int debug_unload()
{
	ps.opened = 0;
#if __UNIX__
	if (ps.tid != 0)
		kill(ps.tid, SIGKILL);
#endif
	return 0; //for warning message
}

#define debug_read_raw(x,y) ptrace(PTRACE_PEEKTEXT, x, y, 0)

#define ALIGN_SIZE 4096

int debug_read(pid_t pid, void *addr, int length)
{
	//unsigned long dword;
        //int i = length;
	//int align = ps.offset%ALIGN_SIZE;

	return debug_read_at(pid, addr, length, ps.offset);
}

int debug_write(pid_t pid, void *data, int length)
{
	return debug_write_at(ps.tid, data, length, ps.offset);
}

int debug_skip(int times)
{
#if __x86__
	unsigned char buf[16];
	regs_t reg;
	int size;

	if (ps.opened) {
		debug_getregs(ps.tid, &reg);
		debug_read_at(ps.tid, buf, 16, R_EIP(reg));

		if (times<1) times = 1;
		for (;times;times--){
			R_EIP(reg) += instLength(buf,16, 0);
			debug_setregs(ps.tid, &reg);
		}
	}
#else
#warning TODO: debug_skip()
#endif
	return 0;
}

int debug_imap(char *args)
{
	int fd = -1;
	int ret = -1;

	if (!ps.opened) {
		eprintf(":imap No program loaded.\n");
		return 1;
	}

	if(!args) {
		eprintf(":imap No insert input stream.\n");
		return 1;
	}

	if (!strncmp("file://", args + 1, 7)) {
		struct stat inf;
		char *addr, *pos;
		char buf[4096];
		char *filename = args + 8;
		int len;

		if((fd = open(filename, O_RDONLY)) < 0) {
			perror(":map open");
			goto err_map;
		}

		if(fstat(fd, &inf) == -1) {
			perror(":map fstat");
			goto err_map;
		}

		if(inf.st_size > MAX_MAP_SIZE) {
			eprintf(":map file too long\n");
			goto err_map;
		}

		addr = (char *)alloc_tagged_page(args + 1, inf.st_size);
		if(addr == (char *)-1) {
			eprintf(":imap memory size %i failed\n", inf.st_size);
			goto err_map;
		}

		pos = addr;
		while((len = read(fd, buf, 4096)) > 4096) {
			debug_write_at(ps.tid, buf, 4096, (long)pos);
			pos += 4096;
		}

		if(len > 0)
			debug_write_at(ps.tid, buf, len, (long)pos);

		eprintf("file %s mapped at 0x%x\n", filename, addr);
	} else {
		eprintf(":imap Invalid input stream\n");
		goto err_map;
	}

	ret = 0;

err_map:
	if(fd >= 0)
		close(fd);

	return ret;
}

int debug_signal(char *args)
{
	int signum;
	char *signame;
	char *arg;
	unsigned long handler;
	off_t address;

	if (!ps.opened) {
		eprintf(":signal No program loaded.\n");
		return 1;
	}

	if (!args) {
		print_sigah();
		return 0;
	}
	signame = args + 1;
	if ((arg= strchr(signame, ' '))) {
		arg[0]='\0'; arg=arg+1;
		signum = name_to_sig(signame);
		address = get_math(arg);
		//signal_set(signum, address);
		arch_set_sighandler(signum, address);
	} else {
		signum = name_to_sig(signame);	

		if (signum == -1) {
			eprintf(":signal Invalid signal name %s.\n", signame);
			return 1;
		}

		debug_print_sigh(signame, (unsigned long)arch_get_sighandler(signum));
	}

	return 0;
}

int debug_stepu()
{
	unsigned long pc = arch_pc(); //WS_PC();
	unsigned char cmd[4];
	int i;

	// TODO handle ^C
	signal(SIGINT, &signal_step);
	stepping_in_progress = 1;

	do {
		debug_step(1);
		i++;
	} while (stepping_in_progress && !is_usercode(arch_pc()));

	stepping_in_progress = 0;
	signal(SIGINT, SIG_IGN);
//	cons_printf("%d instructions executed\n", i);

	return 0;
}

int debug_stepo()
{
	unsigned long pc = arch_pc(); //WS_PC();
	unsigned char cmd[4];
	int skip;
	int bp_pos;

	debug_read_at(ps.tid, cmd, 4, (off_t)pc);

	if ((skip = arch_is_stepoverable(cmd))) {
		pc+=skip;

		///  XXX  BP_SOFT with restore_bp doesnt restores EIP correctly
		bp_pos = debug_set_bp(NULL, pc, BP_HARD);

		debug_cont();
		debug_rm_bp_num(bp_pos);
		
//		restore_bp();
		return 1;
	} else
		debug_step(1);

	return 0;
}

int debug_step(int times)
{
	char opcode[4];
	unsigned long pc, off;
	unsigned long old_pc = 0;
	char *tracefile;
	char *flagregs;

	if (!ps.opened) {
		eprintf("No program loaded.\n");
		return 0;
	}

	if(times < 1)
		times = 1;

	/* restore breakpoint */
	if(restore_bp())
		times--;

	if (ps.verbose) {
		for(;WS(event) == UNKNOWN_EVENT && times; times--) {
			debug_steps();
			ps.steps++;
			debug_dispatch_wait();
		}
		debug_print_wait("step");
	} else {
		/* accelerate soft stepoverables */
		for(;WS(event) == UNKNOWN_EVENT && times; times--) {
			pc = arch_pc();
			if (pc == old_pc) {
				debug_read_at(ps.tid, opcode, 4, (off_t)pc);
				// determine infinite loop
			#if __i386__
				// XXX this is not nice!
				if (opcode[0]=='\xeb' && opcode[1]=='\xef') {
					eprintf("step: Infinite loop detected.\n");
					return 0;
				}
			#endif
				if (off=arch_is_soft_stepoverable(opcode)) {
					pc += off;
			#if __i386__
					debug_set_bp(NULL, pc, BP_HARD);
			#else
					debug_set_bp(NULL, pc, BP_SOFT);
			#endif
					debug_cont();
					debug_rm_bp_addr(pc);
				} else {
					debug_steps();
					ps.steps++;
					debug_dispatch_wait();
				}
			} else {
				debug_steps();
				ps.steps++;
				debug_dispatch_wait();
			}

			flagregs = config_get("trace.cmtregs");
			if (flagregs) {
				char buf[1024];
				char *ptr;
			//	radare_command("!dregs", 0);
				config_set("scr.buf", "true");
				arch_print_registers(0, "line");
				ptr = cons_get_buffer();
				if(ptr[0])ptr[strlen(ptr)-1]='\0';
				sprintf(buf, "C %d %s @ 0x%08x",
					ps.steps, ptr, (unsigned long)arch_pc());
				config_set("scr.buf", "false"); // XXX
				radare_command(buf, 0);
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
					sprintf(buf, "0x%08llx ", (off_t)pc);
					write(fd, buf, strlen(buf));
					//pprint_fd(1);
#if 0
					int t = config.verbose;
					int o = dup(1);
					close(1);
					dup2(fd, 1); // output to file
					//config.verbose = 0;

					disassemble(16, 10); // udis.c
					fflush(stdout);
					write(1,"\n", 1);

					dup2(o, 1);
					config.verbose = t;

					close(6667);
#endif
					close(fd);
				}
			}
			old_pc = pc;
		}
		return 1;
	}

	return (WS(event) != BP_EVENT);
}

int debug_set_register(char *args)
{
	char *value;

	if (!args) {
		eprintf("Usage: !set [reg] [value]\n");
		return 1;
	}
	if (!ps.opened) {
		eprintf(":regs No program loaded.\n");
		return 1;
	}
	args = args + 1;
	value = strchr(args, ' ');
	if (!value) {
		eprintf("Usage: !set [reg] [value]\n");
		return 1;
	}
	value[0]='\0';
	value = value + 1;

	return arch_set_register(args, value);
}

int debug_fpregisters(int rad)
{
	if (!ps.opened) {
		eprintf(":fpregs No program loaded.\n");
		return 1;
	}

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
	if (!ps.opened) {
		eprintf(":regs No program loaded.\n");
		return 1;
	}

	return arch_print_registers(rad, "");
}

int debug_trace(char *input)
{
	int c = 0;
	// TODO: file.trace ???
	int tbt = (int)config_get("trace.bt");
	long slip = (int)config_get_i("trace.sleep");
	int smart = (int)config_get("trace.smart");
	int level = atoi(input+1);
	unsigned long pc;

	if (strchr(input,'?')) {
		printf("!trace levels\n");
		printf("  0  no output\n");
		printf("  1  show addresses\n");
		printf("  2  address and disassembly\n");
		printf("  3  address, disassembly and registers\n");
		printf("  > eval dbg.tracebt = true ; show backtrace\n");
		return 0;
	}

	/* default debug level */
	if (level == 0 && input[0]!='0')
		level=2;

	radare_controlc();

	while(!config.interrupted && ps.opened && debug_step(1)) {
		if (smart) {
			cons_printf("[-] 0x%08x\n", arch_pc());
			radare_command("s eip && f -eip", 0);
			disassemble(20, 2);
			radare_command("!dregs", 0);
		} else {
			switch(level) {
			case 0:
				break;
			case 1:
				cons_printf("0x%08x\n", arch_pc());
				break;
			case 2:
			case 3:
			default:
				pc = arch_pc();
				if (is_usercode(pc)) {
					radare_seek((off_t)pc, SEEK_SET);
					radare_read(0);
					udis(10,1);
					if (level == 3) {
						debug_registers(0);
						cons_printf("\n");
					}
					if (tbt) {
						// XXX must be internal call
						radare_command("!bt", 0);
					}
				}
				fflush(stdout);
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

void debug_print_bps()
{
	int bps;
	int i;

	if (ps.bps_n) {
		eprintf("breakpoints:\n");
		bps = ps.bps_n;
		for(i = 0; i < MAX_BPS && bps > 0; i++) {
			if(ps.bps[i].addr > 0) { 
				if(ps.bps[i].hw)
					eprintf("   at 0x%08x HARD\n", ps.bps[i].addr); 
				else	eprintf("   at 0x%08x SOFT\n", ps.bps[i].addr);
				bps--;	
			}
		}
	} else
		eprintf("breakpoints not set\n");
}

int debug_rm_bps()
{
	int i;
	int ret, bps;

	bps = ps.bps_n;

	for(i = 0; i < MAX_BPS && ps.bps_n > 0; i++) {
		if(ps.bps[i].addr > 0) { 
		        if(ps.bps[i].hw)
                		ret = arch_rm_bp_hw(&ps.bps[i]);
        		else	ret = arch_rm_bp_soft(&ps.bps[i]);

			if(ret < 0) {
				eprintf(
					":debug_rm_bps error removing bp 0x%x\n",
					ps.bps[i].addr);
				break;
			}

			ps.bps[i].addr = 0;
			ps.bps_n--;
		}
	}

	return bps - ps.bps_n;
}

int debug_wp(const char *str)
{
	int i = 0;

	if(ps.wps_n == sizeof(ps.wps)) {
		eprintf(":error	max watchpoints are 4!\n");
		return -1;
	}

	switch(*str) {
	/* remove watchpoint */
	case '-':
		str++;
		skip_chars(&str);
		i = atoi(str);
		if(i >= 0 && i <= 4) {
			if(rm_wp(i))
				printf("watchpoint %d free\n", i);
		} else {
			eprintf(":error invalid watchpoint number\n");
			return -1;
		}
		break;
	/* print watchpoints */
	case '?':
		cons_printf("Usage: !wp[?|*] ([expression|-idx])\n"
		"  !wp           list all watchpoints\n"
		"  !wp*          remove all watchpoints\n"
		"  !wp -#        removes watchpoint number\n"
		" Expression example:\n"
		"  !wp %%eax = 0x8048393 and ( [%%edx] > 0x100 )\n");
/*
	TODO: implement wps (maps), wph (drx)
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
				print_wps();
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

int debug_bp(const char *str)
{
	unsigned long addr;
	const char *ptr = str;
	const char *type;
	int bptype = BP_NONE;

	switch(str[0]) {
	case 's':
		bptype = BP_SOFT;
		break;
	case 'h':
		bptype = BP_HARD;
		break;
	case '?':
		cons_printf("Usage: !bp[?|s|h|*] ([addr|-addr])\n"
		"  !bp [addr]    add a breakpoint\n"
		"  !bp -[addr]   remove a breakpoint\n"
		"  !bp*          remove all breakpoints\n"
		"  !bps          software breakpoint\n"
		"  !bph          hardware breakpoint\n");
		return 0;
	}

	if (bptype == BP_NONE) {
		type = config_get("dbg.bptype");
		if (type) {
			if (type[0]=='s')
				bptype = BP_SOFT;
			else
			if (type[0]=='h')
				bptype = BP_HARD;
		}
	}

	while ( *ptr && *ptr != ' ') ptr = ptr + 1;
	while ( *ptr && *ptr == ' ' ) ptr = ptr + 1;

	type = ptr;

	while ( *type && *type != ' ') type = type + 1;
	while ( *type && *type == ' ' ) type = type + 1;

	switch(ptr[0]) {
	case '-':
		addr = get_offset(ptr+1);
		if(debug_rm_bp_addr(addr) == 0)
			eprintf("breakpoint at 0x%x dropped\n", addr);
		flag_clear_by_addr(addr);
		break;
	case '*':
		eprintf("%i breakpoint(s) removed\n", debug_rm_bps());
		break;
	default:
		addr = get_offset(ptr);
		if (ptr[0]==0 || addr == 0)
			debug_print_bps();
		else {
			flag_set("breakpoint", addr, 1);
			debug_set_bp(NULL, addr, bptype);
			eprintf("new breakpoint at 0x%lx\n", addr);
		}
		break;
	}

	return 0;
}

struct bp_t *debug_get_bp(unsigned long addr)
{
	int i = 0;

	for(i = 0; i < MAX_BPS; i++) 
		if(ps.bps[i].addr == addr)
			return &ps.bps[i]; 

	return  NULL;
}

/* HACK: save a hardware/software breakpoint */
inline int restore_bp()
{
        if(WS(event) == BP_EVENT) {
		arch_restore_bp(WS(bp));
		return 1;
        }

	return 0;
}

int print_syscall()
{
	return arch_print_syscall();
}

int debug_contuh(char *arg)
{
	off_t off = arch_pc();
	int bp;
	debug_step(1);
	bp = debug_set_bp(NULL, off, BP_SOFT);
	debug_cont();
	restore_bp();
	debug_rm_bp_num(bp);
}

int debug_contsc(char *arg)
{
	int ret;
	int wait_val;
	int num = (int)get_math(arg);

	if (!ps.opened) {
		eprintf(":contc No program loaded.\n");
		return -1;
	}

	/* restore breakpoint */
	do {
		restore_bp();

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
		radare_command("!bt", 0);

	return 1;
}

int debug_inject_buffer(unsigned char *fil, int sz)
{
	int i;
	unsigned long eip = arch_pc(); //WS_PC();
	char *ptr;

	// XXX make this work on ARM
#warning debug_inject() only supports X86 architecture
	fil[sz]=0xcc; // breakpoint on X86 (XXX broken for arm)
	for(i=0;i<sz+1;i++){
		if (!(i%5))eprintf("\n");
		eprintf("%02x", fil[i]);
	}
	eprintf("\n");

	ptr = (char *)malloc(sz);
	// backup code
	debug_read_at(ps.tid, ptr, sz, (off_t)ps.entrypoint);
	// inject shellcode
	debug_write_at(ps.tid, fil, sz, (off_t)ps.entrypoint);
	// execute it
	arch_jmp(ps.entrypoint);
	debug_cont();
	// restore
	arch_jmp(eip);
	debug_write_at(ps.tid, ptr, sz, (off_t)eip);
	free(ptr);
}

int debug_inject(char *file)
{
	off_t sz = (off_t)0;
	int fd = open(file, O_RDONLY);
	unsigned char *fil = NULL;

	if (fd == -1) {
		eprintf("Cannot open '%s'\n", file);
		return 0;
	}

//	sz = (off_t)
	lseek(fd, (off_t)0, SEEK_END) + (off_t)4;
	lseek(fd, (off_t)0, SEEK_SET);
	if (sz>0xffff) {
		eprintf("File too big\n");
		close(fd);
		return 0;
	}
	// read the code
	fil = (char *)malloc(sz);
	read(fd, fil, sz);

	debug_inject_buffer(fil, sz);
	printf("%d bytes injected and executed from %s\n", (unsigned int)sz-1, file);

	free(fil);
	close(fd);

	return 1;
}

int debug_cont()
{
	if (!ps.opened) {
		eprintf("cont: No program loaded.\n");
		return -1;
	}

	do { 
		/* restore breakpoint */
		restore_bp();

		/* launch continue */
		debug_contp(ps.tid);

	} while(debug_dispatch_wait() == 1 && debug_contp(ps.tid) != -1);

	/* print status */
	debug_print_wait("cont");

	return 0;
}

int debug_pids()
{
	int i, fd;
	int n = 0;
	char cmdline[1025];

#if __UNIX__
	// TODO: use ptrace to get cmdline from esp like tuxi does
	for(i=2;i<999999;i++) {
		switch( kill(i, 0) ) {
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
#endif
	return n;
}

int debug_rm_bp(unsigned long addr, int type)
{
	struct bp_t *bp;
	int ret;

	if(type == 0) {
		bp = debug_get_bp(addr);	
	} else {
		if(addr < MAX_BPS)
			bp = &ps.bps[addr];
		else
			bp = NULL;
	}

	if(!bp)
		return -1;

	if(bp->hw)
		ret = arch_rm_bp_hw(bp);
	else
		ret = arch_rm_bp_soft(bp);

	if(ret < 0)
		return ret;

	flag_clear_by_addr((off_t)addr); //"breakpoint", addr, 1);

	bp->addr = 0;
	ps.bps_n--;

	return 0;
}

int inline debug_rm_bp_num(int num)
{
#warning XXX THIS IS BUGGY! num != addr, off_t != int !!
	return debug_rm_bp(num, 1);
}

inline unsigned long debug_get_regoff(regs_t *reg, int off)
{
	char *c = (char *)reg;
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

inline int debug_rm_bp_addr(unsigned long addr)
{
	return debug_rm_bp(addr, 0);
}

int debug_set_bp(struct bp_t *bp, unsigned long addr, int type)
{
	int i, ret, bp_free = -1;

	if (addr == 0)
		addr = arch_pc(); // WS_PC();

	/* all slots busy */
	if(ps.bps_n == MAX_BPS)
		return -2;

	/* search for breakpoint */
	for(i=0;i < MAX_BPS;i++) {

		/* breakpoint found it */
		if(ps.bps[i].addr == addr) {
			if(bp)
				bp = &ps.bps[i];
			return 0;
		}

		if(ps.bps[i].addr == 0)
			bp_free = i;
	}

	if(type == BP_SOFT) {
		ret = arch_set_bp_soft(&ps.bps[bp_free], addr);
		ps.bps[bp_free].hw = 0;
	} else if(type == BP_HARD) {
		ret = arch_set_bp_hw(&ps.bps[bp_free], addr);
		ps.bps[bp_free].hw = 1;
	} else {
		if((ret = arch_set_bp_hw(&ps.bps[bp_free], addr)) >= 0 )
			ps.bps[bp_free].hw = 1;
		else if((ret = arch_set_bp_soft(&ps.bps[bp_free], addr)) >= 0)
			ps.bps[bp_free].hw = 0;
	}

	if(ret < 0)
		return ret;

	ps.bps[bp_free].addr = addr;
	if(bp)
		bp = &ps.bps[bp_free];

	ps.bps_n++;

	return bp_free;
}

int debug_run()
{
	if (ps.opened) {
		if (getv())
			eprintf("To cleanly stop the execution, type: "
				       "\"^Z kill -STOP %d && fg\"\n", ps.tid);
		return debug_cont();
	}

	eprintf("No program loaded.\n");
	return 1;
}
