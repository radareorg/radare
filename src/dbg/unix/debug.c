/*
 * Copyright (C) 2007, 2008
 *       pancake <youterm.com>
 *       th0rpe <nopcode.org>
 *
 * libps2fd is part of the radare project
 *
 * libps2fd is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * libps2fd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libps2fd; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#define TRACE printf("%s:%d\n", __FILE__, __LINE__);

#include "../../config.h"
#include "../../main.h"
#include "../../code.h"
#include "../libps2fd.h"
#include "../wp.h"
#include "../mem.h"
#include "../thread.h"
#include "../signal.h"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#if __UNIX__
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif
#if __linux__
#include <sys/prctl.h>
#include <linux/ptrace.h>
#endif
#include <sys/stat.h>
#include "../events.h"
#include "../debug.h"
#include "procs.h"


// TODO: move to ps structure
//static int stepping_in_progress = 0;


#if __NetBSD__ || __OpenBSD__ || __APPLE__

#include <sys/param.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <sys/ktrace.h>

int debug_ktrace()
{
	int sta, ops, trp, fd, ret;
	int pd[2];
	char buf[1024];
	
	ops = KTROP_SET|KTRFLAG_DESCEND;
	trp = KTRFAC_SYSCALL|KTRFAC_SYSRET|KTRFAC_INHERIT;
	pipe(pd);

	if (ps.tid == 0)
		printf("no idea what pid=0 means. do you?\n");

	printf("KTracing process %d. Waiting for f00d...\n", ps.tid);
	setpriority(PRIO_PROCESS,getpid(),-18);
	ret = fktrace(pd[1], ops, trp, ps.tid);
	if (ret == -1 ) {
		printf("Cannot fktrace %d\n", ps.tid);
		return 1;
	}

	ret = arch_continue();
	ret = read(pd[0],buf,1);
	if (ret>0) {
		kill(ps.tid, SIGSTOP);
		printf("Have sweet dreamz my lil pon1e!!1!\n");

		ret = fktrace(pd[1], KTROP_CLEAR, trp, ps.tid);
		ret |= fktrace(pd[1], KTROP_CLEARFILE, trp, ps.tid);
		printf("<fktrace> %d\n", ret);

		ret = kill(ps.tid, 0);
		printf("<kill0> %d (alive)\n", ret);

		ret = wait(&sta);
		printf("<wait> %d has %08x\n", ret, sta);
		printf("<wait> %d has %08x\n", WIFEXITED(ret), WEXITSTATUS(sta));
	}
	close(pd[0]);
	close(pd[1]);

	return 0;
}
#endif


int debug_pstree(char *input)
{
	int tid = 0;

	if (input)
		tid = atoi(input);
	if (tid != 0) {
		ps.tid = tid;
		eprintf("Current selected thread id (pid): %d\n", ps.tid);
		// XXX check if exists or so
	}

	printf(" pid : %d\n", ps.pid);
	pids_sons_of_r(ps.pid,0,0);
	printf(" tid : %d\n", ps.tid);
	pids_sons_of_r(ps.tid,0,0);
	return 0;
}

int debug_print_wait(char *act)
{
	const char *cmd;
	if (act)
	switch(WS(event)) {
	case BP_EVENT:
		eprintf("%s: breakpoint stop (0x%x)\n", act, WS(bp)->addr);
		cmd = config_get("cmd.bp");
		if (cmd&&cmd[0])
			radare_cmd((char *)cmd, 0);
		break;
	default:
		if(WS(event) != EXIT_EVENT ) {
			/* XXX: update thread list information here !!! */
			eprintf("=== %s: tid: %d signal: %d (%s). stop at 0x%08llx\n", 
				act, ps.tid, WS_SI(si_signo),
				sig_to_name(WS_SI(si_signo)),
				arch_pc());
		}
	}
	return 0;
}


int debug_init_maps(int rest)
{
	FILE *fd;
	MAP_REG *mr;
	char  path[1024];
	char  region[100], perms[5], null[16];
	char  line[1024];
	char  *pos_c;
	int   i, unk = 0;

	if(ps.map_regs_sz > 0)
		free_regmaps(rest);

	sprintf(path, "/proc/%d/maps", ps.tid);
	fd = fopen(path, "r");
	if(!fd) {
		perror("debug_init_maps");
		return -1;
	}

	while(!feof(fd)) {
		fgets(line, 1023, fd);
		path[0]='\0';
		sscanf(line, "%s %s %s %s %s %s",
			&region[2], perms,  null, null, null, path);
		if (path[0]=='\0')
			sprintf(path, "unk%d", unk++);

		pos_c = strchr(&region[2], '-');
		if(!pos_c)
			continue;

		*pos_c = (char)NULL;

		mr = malloc(sizeof(MAP_REG));
		if(!mr) {
			perror(":map_reg alloc");
			fclose(fd);
			return -1;
		}

		region[0] = '0';
		region[1] = 'x';

		mr->ini = get_math(region);

		pos_c[-1] = '0';
		pos_c[0] = 'x';

		mr->end = get_math(pos_c - 1);
		mr->size = mr->end - mr->ini;
		mr->bin = strdup(path);
		mr->perms = 0;
		if(!strcmp(path, "[stack]") || !strcmp(path, "[vdso]"))
			mr->flags = FLAG_NOPERM;
		else 
			mr->flags = 0;

		for(i = 0; perms[i] && i < 4; i++) {
			switch(perms[i]) {
				case 'r':
					mr->perms |= REGION_READ;
					break;
				case 'w':
					mr->perms |= REGION_WRITE;
					break;
				case 'x':
					mr->perms |= REGION_EXEC;
			}
		}

		add_regmap(mr);
	}

	fclose(fd);

	return 0;
}

int debug_os_init()
{
	/* do nothing */
	return 0;
}

int debug_status()
{
	system("cat /proc/${DPID}/status");
	return 0; //for warning message
}


static int is_alive(int pid)
{
	return kill(pid, 0)==0;
}

int debug_fork_and_attach()
{
	int wait_val;
	int i,pid;

	pid = vfork();
	switch(pid) {
	case -1:
		eprintf("Cannot fork.\n");
		return 0;
	case 0:
		ps.pid = pid;
		ps.tid = pid;
		ptrace(PTRACE_TRACEME, 0, 0, 0);

		if (config_get("cfg.verbose")) {
			printf("argv = ");
			for(i=0;ps.argv[i];i++)
				printf("'%s', ", ps.argv[i]);
			printf("]\n");
		}

		// TODO: USE TM IF POSSIBLE TO ATTACH IT FROM ANOTHER CONSOLE!!!
		debug_environment();
		execv(ps.argv[0], ps.argv);
		perror("fork_and_attach: execv");
		eprintf("[%d] %s execv failed.\n", getpid(), ps.filename);
		exit(0);
	default:
		ps.opened = 1;
		ps.pid = pid;
		ps.tid = pid;

		/* dupped kill check */
		debug_waitpid(-1, &wait_val);
		if (WIFEXITED(wait_val)) {
			perror("waitpid");
			debug_exit();
			return -1;
		}

		if (!is_alive(ps.pid)) {
			fprintf(stderr, "Oops the process is not alive?!?\n");
			return -1;
		}

		/* restore breakpoints */
		debug_reload_bps();

		if (config_get("dbg.stop")) 
			kill(ps.pid, SIGSTOP);
		ps.steps  = 1;
	}
	return 0;
}

int debug_attach(int pid)
{
	int wait_val;

	if (-1 == ptrace(PTRACE_ATTACH, pid, 0, 0)) {
		perror("ptrace_attach");
		return -1;
	}

	ps.opened = 1;
	ps.pid = pid;
	ps.tid = pid;
	ps.steps = 1;

	debug_waitpid(pid, &wait_val);

	if (WIFEXITED(wait_val)) {
		debug_exit();
		return -1;
#if __linux__ && !__x86_64__
	} else {
		if(ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACECLONE) == -1) {
			perror("ptrace_setoptions");
			return -1;
		}
#endif
	}


	if (!is_alive(pid))
		return -1;

	return 0;
}

int debug_detach()
{
	// XXX tid ps.tid, ps.pid ?!?
	ptrace(PTRACE_DETACH, ps.pid, 0, 0);
	if (-1 == ptrace(PTRACE_DETACH, ps.tid, 0, 0))
		perror("ptrace_dettach");

	ps.pid = 0;
	ps.tid = 0;
	ps.opened = 0;
	ps.steps  = 0;

	return 0;
}
#define debug_read_raw(x,y) ptrace(PTRACE_PEEKTEXT, x, y, 0)

/* copied from patan */
extern int errno;

static int ReadMem(int pid,  addr_t addr, size_t sz, void *buff)
{
	unsigned long words = sz / sizeof(long) ;
	unsigned long last = sz % sizeof(long) ;
	long x, lr ;
	int ret ;

	if (addr==-1)
		return 0;

	for(x=0;x<words;x++) {
		((long *)buff)[x] = debug_read_raw(pid, (void *)(&((long*)(long )addr)[x]));

		if (((long *)buff)[x] == -1 && errno)
			goto err;
	}

	if (last) {
		//lr = ptrace(PTRACE_PEEKTEXT,pid,&((long *)addr)[x],0) ;
		lr = debug_read_raw(pid, &((long*)(long)addr)[x]);

		if (lr == -1 && errno)
			goto err;

		memcpy(&((long *)buff)[x],&lr,last) ;
	}

	return sz;
err:
	ret = --x * sizeof(long);

	return ret ;
}

#define ALIGN_SIZE 4096
int debug_read_at(pid_t pid, void *buf, int length, u64 addr)
{
	return ReadMem(pid, (addr_t)addr, length, buf);
#if 0
	long dword;
        int len, i = length;
	int align = ALIGN_SIZE-at%ALIGN_SIZE;


	if (align!=0) {
		unsigned char four[ALIGN_SIZE];
		//if (debug_read_at(pid, &four, ALIGN_SIZE, (unsigned long)(at-align))==-1)
		//	return 0;
		//memcpy(addr, four+align, ALIGN_SIZE-align);
	}

	for(i=4-align; i<length; i+=4) {
		dword = debug_raw_read(pid, (unsigned long)(at+i));
		if (dword == -1)
			return 0;
		len = (i+4>length)?length-i:4;
		memcpy(addr+i, &dword, len); // XXX padding control?
	}

        return i;
#endif
}

ssize_t WriteMem(int pid,  addr_t addr, size_t sz, const unsigned char *buff)
{
        long words = sz / sizeof(long) ;
        long last = (sz % sizeof(long))*CHAR_BIT ;
        long  lr ;
	int x;

/*
	long *word=&buf;
	char buf[4];
        En los fuentes del kernel se encuentra un #ifdef para activar el soporte de escritura por procFS.
        Por razones de seguridad se encuentra deshabilitado, pero nunca esta de mas intentar ;)
*/
#if 0
	word = ptrace(PTRACE_PEEKDATA, pid, (void *)addr, (void *)buf);
	if (word==-1)
		word = ptrace(PTRACE_PEEKTEXT, pid, (void *)addr, (void *)buf);
	buf[0]=buff[0];
	ptrace(PTRACE_POKEDATA, (pid_t)pid, (void *)addr, (void *)buf);
	ptrace(PTRACE_POKETEXT, pid, (void *)addr, (void *)buf);
	return sz;
#endif
//eprintf("%d ->%d (0x%x)\n",pid, (int)sz, (long)addr);


	for(x=0;x<words;x++)
		if (ptrace(PTRACE_POKEDATA,pid,&((long *)(long)addr)[x],((long *)buff)[x]))
			goto err ;

	if (last)
	{
		lr = ptrace(PTRACE_PEEKTEXT,pid,&((long *)(long)addr)[x], 0) ;

		/* Y despues me quejo que lisp tiene muchos parentesis... */
		if ((lr == -1 && errno) ||
		    (
			ptrace(PTRACE_POKEDATA,pid,&((long *)(long)addr)[x],((lr&(-1L<<last)) |
			(((long *)buff)[x]&(~(-1L<<last)))))
		    )
		   )
                goto err;
	}

	return sz ;

        err:
                return --x * sizeof(long) ;

        //return ret ;
}


int putdata(pid_t child, unsigned long addr, char *data, int len)
{
	char val_aux[sizeof(long)];
	char val_res[sizeof(long)];
	int  len_w;
	long pos  = addr;
	long last = addr + len - sizeof(long);
	long *val = (long *)data;

	if(len <= 0)
		return 0;
/* todo align in memory */
//	eprintf("one\n");

	for(;pos < last; pos += sizeof(long), val++) {
		if(ptrace(PTRACE_POKEDATA, child, pos, *val) == -1) {
			//perror(":putdata ptrace_pokedata");
			return 1;
		}
	}

	/* aligned length */
	if(pos == last) {
		if(ptrace(PTRACE_POKEDATA, child, pos, *val) == -1) {
			//perror(":putdata ptrace_pokedata");
			return 1;
		}
		
	/* not aligned length */
	} else {
		debug_read_at(ps.tid, val_aux, sizeof(long), pos);

		if(len < sizeof(long))
			len_w = len;
		else	len_w = (last + sizeof(long)) - pos;

		memcpy(val_res, (char *)val, len_w); 
		memcpy(val_res + len_w, val_aux + len_w, sizeof(long) - len_w);

		if(ptrace(PTRACE_POKEDATA, child,
			pos, *(long *)val_res) == -1) {
			//perror(":putdata ptrace_pokedata");
			return 1;
		}
	}

	return 0;
}

int debug_write_at(pid_t pid, void *data, int length, u64 addr)
{
//	return putdata(pid,(unsigned long)addr, data, length);
	return WriteMem(pid, addr, length, data);
}

inline int debug_getregs(pid_t pid, regs_t *reg)
{
#if __linux__
	return ptrace(PTRACE_GETREGS, pid, NULL, reg);
#else
	return ptrace(PTRACE_GETREGS, pid, reg, sizeof(regs_t));
#endif
}

inline int debug_setregs(pid_t pid, regs_t *reg)
{
#if __linux__
        return ptrace(PTRACE_SETREGS, pid, NULL, reg);
#else
        return ptrace(PTRACE_SETREGS, pid, reg, sizeof(regs_t));
#endif
}

int debug_getsignal(siginfo_t *si)
{
#if __linux__
        // 0x4202 = PTRACE_GETSIGINFO // kernel>=2.4.16
        if(ptrace(0x4202,ps.tid,NULL, si) < 0) {
		perror("ptrace_getsiginfo");
		return -1;
	}
#else
	int status = 0;
        si->si_signo = WSTOPSIG(status);
#endif
	return 0;
}

inline int debug_contp(int pid)
{
	return ptrace(PTRACE_CONT, pid, PTRACE_PC, 0);
}

inline int debug_contscp()
{
	return ptrace(PTRACE_SYSCALL, ps.tid, PTRACE_PC, 0);
}

inline int debug_os_steps()
{
	return ptrace(PTRACE_SINGLESTEP, ps.tid, PTRACE_PC, 0);
}

int debug_dispatch_wait()
{
	int 		status;
#if __i386__
	//regs_t		regs;
#endif
	struct bp_t	*bp;
	TH_INFO		*th;
	pid_t tid = 0;
	int ret = 0;

	WS(event) = UNKNOWN_EVENT;

	ps.tid = debug_waitpid(-1, &status);

#if __i386__
	debug_getregs(ps.pid, &(WS(regs)));
#endif
	events_get();

	if(!TH_MAIN(ps.tid)) {
		if ( ( th = get_th(ps.tid) ) ) {
			ps.th_active = th;
			TH_ADDR(th, arch_pc());
		} else {
			eprintf("pid: %d. new process created!\n"
			" - Use !pstree to list, and !th to change current pid (tid)\n", ps.tid);
			if(!(th = init_th(tid, status))) { // STOPPED?
				perror("init_th");
				return -1;
			}
			th->tid = ps.tid;
			th->addr = arch_pc();
			add_th(th);
			ps.th_active = th;

			ret = 1;
		}
	} else
		ps.th_active = 0;

	if(WIFEXITED(status)) {
		eprintf("\n\n______________[ process finished ]_______________\n\n");
		//ps.opened = 0;
		kill(ps.tid, SIGKILL);
		debug_load();
sleep(1);
	} else if(WIFSTOPPED(status)) {
		if(debug_getsignal(&WS(si)) == 0) {

	if (event_is_ignored(WS_SI(si_signo))) {
		eprintf("signal %d ignored\n",WS_SI(si_signo));
		return 1;
	}
			// 0 is for bsd?
			if(
#if __linux__
			WS_SI(si_signo) ==  SIGTRAP
#else
			WS_SI(si_signo) == 0 /* bsd */
#endif
			) {
#if __linux__ && !__x86_64__
				/* linux threads support */
				if (ps.pid == ps.tid  && status >> 16 == PTRACE_EVENT_CLONE) {

					if(ptrace (PTRACE_GETEVENTMSG,
						ps.pid, 0, &tid) == -1) {
						perror("ptrace_geteventmsg");
						return -1;	
					}
				
					printf("____[ New thread created ]____\n");
					printf("tid: %d\n", tid);

					ret = debug_waitpid(tid, &status);

				        if (ret == -1)
						eprintf(":error waiting for new child\n");
          				else if (ret != tid)
						eprintf(":error return tid %d != %d\n", ret, tid);
#if 0
          				else if (!WIFSTOPPED (status) ||
						 WSTOPSIG (status) != SIGSTOP)
						eprintf(":error unknown state thread %d\n", tid);
#endif
					else {
						if(!(th = init_th(tid, status))) {
							perror("init_th");
							return -1;
						}
						add_th(th);

						ps.tid = tid;
						ret = 1;
					}
					return ret;
				}
#endif
				/*  stopped by? */
				bp = (struct bp_t*) arch_stopped_bp();
				if(bp) {
					WS(event) = BP_EVENT;
					WS(bp) = bp;
				} 
			} else if(WS_SI(si_signo) ==  SIGSEGV) {
				/* search if changed permissions at region */
				//printf("CHANGE REGIONS: 0x%x\n", WS_SI(si_addr));
				/* TODO: manage access to protected pages */
				;
				eprintf("Segmentation fault!\n");
			} else
				eprintf("Unknown signal %d received\n", WS_SI(si_signo));
		}
	} else
		eprintf("What?\n");

	return 0; /* XXX: should stop */
}

/* HACK: save a hardware/software breakpoint */
int debug_contfork(int tid)
{
	int ret;

	// XXX ignore tid
	if (ps.opened) {
		//arch_reset_breakpoint(1);
		ret = ptrace(PTRACE_SYSCALL, ps.tid, 0, 0);
		if (ret != 0) {
			eprintf("ptrace_syscall error\n");
			return 1;
		}

		debug_dispatch_wait();
		//wait_ptrace(0);

		return 0;
	}

	eprintf(":contsc No program loaded.\n");
	return 1;
}
