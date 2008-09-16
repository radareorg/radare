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
#include "../signals.h"
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

#if __FreeBSD__
int th_init_freebsd(int pid)
{
	struct ptrace_lwpinfo lwps[128];
	int i,n = 0;
	ptrace(PT_GETLWPLIST, pid, &lwps, sizeof(lwps));
	ptrace(PT_GETNUMLWPS, pid, &n, sizeof(n));
	for(i=0;i<n;i++) {
		cons_printf("lwp(pid=%d)%d id=%d event=%d\n", 
			pid, i, lwps[i].pl_lwpid, lwps[i].pl_event);
		/* TODO: add threads here ??? */
	}
}
#endif

#if __BSD__
int th_info_bsd(int pid)
{
#if __FreeBSD__ || __NetBSD__
	struct ptrace_lwpinfo lwps;
	int i,n = 0;
	lwps.pl_lwpid = pid;
	if (ptrace(PT_LWPINFO, pid, &lwps, sizeof(lwps)) != -1)
		cons_printf("lwp(pid=%d) id=%d event=%d\n",pid,
			lwps.pl_lwpid, lwps.pl_event);
#endif
	return 0;
}
#endif

int debug_os_kill(int pid, int sig)
{
	//return 0;
	/* prevent killall selfdestruction */
	if (pid < 1)
		return -1;
	return kill(pid, sig);
}

/* TODO: OpenBSD have no fktrace..only ktrace */
#if __NetBSD__ || __FreeBSD__ || __OpenBSD__ || __APPLE__

#include <sys/param.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <sys/ktrace.h>

int debug_ktrace()
{
#if __OpenBSD__ || __FreeBSD__
	eprintf("No fktrace for OpenBSD. Needs to be implemented with ktrace(2)\n");
#else
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
#endif
	return 0;
}
#endif
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
		debug_bp_restore(-1);
		break;
	default:
		if(WS(event) != EXIT_EVENT ) {
			/* XXX: update thread list information here !!! */
			eprintf("=== %s: tid: %d event: %d, signal: %d (%s). stop at 0x%08llx\n", 
				act, ps.tid, 
				WS(event),
				WS_SI(si_signo),
				sig_to_name(WS_SI(si_signo)),
				arch_pc(ps.tid));
#if __mips__
			debug_status();
#endif
		}
	}
	return 0;
}

int debug_init_maps(int rest)
{
	FILE *fd;
	MAP_REG *mr;
	char  path[1024], unkstr[1024];
	char  region[100], region2[100], perms[5], null[16];
	char  line[1024];
	char  *pos_c;
	int   i, ign, unk = 0;

#if 0
 /* FOR SUN */
  08045000      12K rwx--    [ stack ]
  08050000    1212K r-x--  /usr/bin/vim
  0818E000     156K rwx--  /usr/bin/vim
  081B5000     120K rwx--    [ heap ]
  D2F70000      64K rwx--    [ anon ]
  D33FE000       4K rwx--  /lib/ld.so.1

radare -x /proc/<pid>/map
   offset   0 1  2 3  4 5  6 7  8 9  A B  C D  E F  0 1  2 3  4 5  6 7  8 9  A B  C D  E F  0 1  0123456789ABCDEF0123456789ABCDEF01
0x00000000, 0050 0408 0030 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 .P...0............................
0x00000022  0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 ..................................
0x00000044, 0000 0000 00e0 ffff ffff ffff 6700 0000 0010 0000 ffff ffff 0000 0000 0000 0508 00f0 ............g.....................
0x00000066  1200 612e 6f75 7400 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 ..a.out...........................
0x00000088, 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 ..................................
0x000000AA  0000 0000 0000 0500 0000 0010 0000 ffff ffff 0000 0000 00e0 1808 0070 0200 612e 6f75 ...........................p..a.ou
0x000000CC, 7400 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 t.................................
0x000000EE  0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 00e0 1200 0000 0000 ..................................
0x00000110, 0700 0000 0010 0000 ffff ffff 0000 0000 0050 1b08 00e0 0100 0000 0000 0000 0000 0000 .................P................
0x00000132  0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 ..................................
0x00000154, 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 5700 0000 0010 ............................W.....
0x00000176  0000 ffff ffff 0000 0000 0000 f7d2 0000 0100 0000 0000 0000 0000 0000 0000 0000 0000 ..................................
0x00000198, 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 ..................................
0x000001BA  0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 4700 0000 0010 0000 ffff ffff ......................G...........
0x000001DC, 0000 0000 0000 f9d2 00e0 0000 7a66 732e 3138 322e 3635 3533 382e 3332 3135 3500 0000 ............zfs.182.65538.32155...
0x000001FE  0000                                                                                 ..                                

0x00000000  0050 0408 -> base addr
0x00000004  0030 0000 -> 0x3000 = 12K of size

0x00000060  0000 0508 -> base addr
0x00000064  00f0 1200 -> size ...

#endif

#if __sun
	/* TODO: On solaris parse /proc/%d/map */
	sprintf(path, "pmap %d > /dev/stderr", ps.tid);
	system(path);
	return 0;
#endif

	if(ps.map_regs_sz > 0)
		free_regmaps(rest);
#if __FreeBSD__
	sprintf(path, "/proc/%d/map", ps.tid);
#else
	sprintf(path, "/proc/%d/maps", ps.tid);
#endif
	fd = fopen(path, "r");
	if(!fd) {
		perror("debug_init_maps");
		return -1;
	}

	while(!feof(fd)) {
		line[0]='\0';
		fgets(line, 1023, fd);
		if (line[0]=='\0')
			break;
		path[0]='\0';
		line[strlen(line)-1]='\0';
#if __FreeBSD__
	// 0x8070000 0x8072000 2 0 0xc1fde948 rw- 1 0 0x2180 COW NC vnode /usr/bin/gcc
		sscanf(line, "%s %s %d %d 0x%s %3s %d %d",
			&region[2], &region2[2], &ign, &ign, unkstr, perms, &ign, &ign);
		pos_c = strchr(line, '/');
		if (pos_c) strcpy(path, pos_c);
		else path[0]='\0';
#else
		sscanf(line, "%s %s %s %s %s %s",
			&region[2], perms,  null, null, null, path);

		pos_c = strchr(&region[2], '-');
		if(!pos_c)
			continue;

		pos_c[-1] = (char)'0';
		pos_c[ 0] = (char)'x';
		strcpy(region2, pos_c-1);
#endif
		region[0] = region2[0] = '0';
		region[1] = region2[1] = 'x';

		if (path[0]=='\0')
			sprintf(path, "unk%d", unk++);

		mr = malloc(sizeof(MAP_REG));
		if(!mr) {
			perror(":map_reg alloc");
			fclose(fd);
			return -1;
		}

		mr->ini = get_offset(region);
		mr->end = get_offset(region2);
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
#if __mips__
	int cause;
	// XXX : is this ok?
	radare_cmd(".!regs*",0);
       	cause = get_offset("cause") & 0xff;
	cons_printf("cause: (%d) ", cause);
	switch(cause) {
	case  0: cons_printf("external interrupt\n"); break;
	case  4: cons_printf("address exception on load or instruction fetch\n"); break;
	case  5: cons_printf("address exception on store\n"); break;
	case  6: cons_printf("bus error on instruction fetch\n"); break;
	case  7: cons_printf("bus error on data load\n"); break;
	case  8: cons_printf("syscall exception\n"); break;
	case  9: cons_printf("breakpoint exception\n"); break;
	case 10: cons_printf("reserved instruction\n"); break;
	case 12: cons_printf("arithmetic overflow\n"); break;
	case 16: cons_printf("unaligned memory load access? (look at pc-4 opcode)\n");
		 radare_cmd("pd 4 @ pc-8", 0); break;//(pd 4 @ pc-8)\n"); break;
	case 32: cons_printf("end of execution\n"); break;
	default: cons_printf("unknown %d\n", (int)cause); break;
	}
	//cons_printf("badvaddr: 0x%08llx (t0)\n", get_offset("t0"));
	cons_printf("badvaddr: 0x%08llx (bad)\n", get_offset("bad"));
#if 0
	cons_printf("status:   0x%08llx (t4)\n", get_offset("t4"));
	cons_printf("cause:    0x%08llx (t5)\n", get_offset("t5"));
	cons_printf("excaddr:  0x%08llx (t6)\n", get_offset("t6"));
#endif
#else
	system("cat /proc/${DPID}/stat");
	// verbose way //
	//system("cat /proc/${DPID}/status");
#endif

	// TODO: run !status here ??? and move the following code to arch_specific one?
	#if ARCH_MIPS
	// r8  = badvaddr - memory address at which address exception ocurred (t0)
	// r12 = status - interrupt mask and enable bits                      (t4)
	// r13 = cause - exception type and pending itnerrupt bits            (t5)
	// r14 = address of instruction that caused exception                 (t6)
	// 
	// CAUSE VALUE: (cause & 0xff)
	// 0 : int - external interrupt
	// 4 : addrl - address error exception (load or instruction fetch)
	// 5 : addrs - address error exception (store)
	// 6 : ibus - bus error on instruction fetch
	// 7 : dbus - bus error on data load or store
	// 8 : syscall - syscall exception
	// 9 : bkpt - breakpoint exception
	// 10: ri   - reserved instruction exception
	// 12: ovf  - arithmetic overflow exception
	#endif
	return 0; //for warning message
}


static int is_alive(int pid)
{
	return kill(pid, 0)==0;
}

#define MAGIC_EXIT 0x33
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
		exit(MAGIC_EXIT); /* error */
	default:
		ps.opened = 1;
		ps.pid = pid;
		ps.tid = pid;

		/* dupped kill check */
		debug_waitpid(pid, &wait_val);
		if (WIFEXITED(wait_val)) {
		//if ((WEXITSTATUS(wait_val)) != 0) {
			perror("waitpid");
			debug_exit();
			return -1;
		}

		if (!is_alive(ps.pid)) {
			fprintf(stderr, "Oops the process is not alive?!?\n");
			return -1;
		}

		/* restore breakpoints */
		debug_bp_reload_all();

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
	if (length<0)
		return -1;
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

int WriteMem(int pid,  u64 addr, int sz, u8 *buff)
{
        long words = sz / sizeof(long) ;
        long last = (sz % sizeof(long))*8;
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

int debug_getregs(pid_t pid, regs_t *reg)
{
#if __sun
	return ptrace(PTRACE_GETREGS, pid, 0, reg);
#elif __linux__
	return ptrace(PTRACE_GETREGS, pid, NULL, reg);
#else
	memset(reg, sizeof(regs_t));
	return ptrace(PTRACE_GETREGS, pid, reg, sizeof(regs_t));
#endif
}

int debug_getxmmregs(pid_t pid, regs_t *reg)
{
#if 0
/* FREEBSD */
int cpu_ptrace(struct thread *td, int req, void *addr, int data)
{

	struct savexmm *fpstate;
	int error;

	if (!cpu_fxsr)
		return (EINVAL);

	fpstate = &td->td_pcb->pcb_save.sv_xmm;
	switch (req) {
	case PT_GETXMMREGS:
		error = copyout(fpstate, addr, sizeof(*fpstate));
		break;

	case PT_SETXMMREGS:
		error = copyin(addr, fpstate, sizeof(*fpstate));
		fpstate->sv_env.en_mxcsr &= cpu_mxcsr_mask;
		break;

	default:
		return (EINVAL);
	}

	return (error);
}
#endif
	return 0;
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

int debug_contp(int pid)
{
	return ptrace(PTRACE_CONT, pid, (u32)arch_pc(pid), 0);
}

int debug_contscp()
{
#if __linux__ || ( __BSD__ && defined(PT_SYSCALL))
	return ptrace(PTRACE_SYSCALL, ps.tid, (u32)arch_pc(ps.tid), 0);
#else
	eprintf("contscp: not implemented for this platform\n");
	return 0;
#endif
}

int debug_os_steps()
{
	return ptrace(PTRACE_SINGLESTEP, ps.tid, (u32)arch_pc(ps.tid), 0);
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
			TH_ADDR(th, arch_pc(ps.tid));
		} else {
			// TODO: get parent PID HERE!!!
			debug_msg_set("new pid: %d. from parent %d\n", ps.tid,0);
			eprintf("pid: %d. new process created!\n"
			" - Use !pid for processes, and !th for threads\n", ps.tid);
			if(!(th = init_th(tid, status))) { // STOPPED?
				perror("init_th");
				return -1;
			}
			th->tid = ps.tid;
			th->addr = arch_pc(ps.tid);
			add_th(th);
			ps.th_active = th;

			ret = 1;
		}
	} else
		ps.th_active = 0;

	if(WIFEXITED(status)) {
		debug_msg_set("process %d finished\n", ps.tid);
		eprintf("\n\n______________[ process finished ]_______________\n\n");
		//ps.opened = 0;
		kill(ps.tid, SIGKILL);
		debug_load();
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
				if(ptrace (PTRACE_GETEVENTMSG, ps.pid, 0, &tid) == -1) {
					perror("ptrace_geteventmsg");
					return -1;	
				}
			
				eprintf("____[ New thread created ]____\ntid: %d\n", tid);

				ret = debug_waitpid(tid, &status);

				if (ret == -1)
					eprintf(":error waiting for new child\n");
				else if (ret != tid)
					eprintf(":error return tid %d != %d\n", ret, tid);
				else if (!WIFSTOPPED (status) ||
					 WSTOPSIG (status) != SIGSTOP)
					eprintf(":error unknown state thread %d\n", tid);
				else {
					if(!(th = init_th(tid, status))) {
						perror("init_th");
						return -1;
					}
					add_th(th);

					ps.tid = tid;
					ret = 1;
				}
					debug_msg_set("pid %d New thread created\n", ps.tid);
				return ret;
			}
	#endif
			debug_bp_restore_after();
#if 0
				/*  stopped by? */
				bp = (struct bp_t*) arch_stopped_bp();
				if(bp) {
					WS(event) = BP_EVENT;
					WS(bp) = bp;
					debug_msg_set("pid %d Stopped by breakpoint\n", ps.tid);
				}
#endif
			} else if(WS_SI(si_signo) == SIGSEGV) {
				/* search if changed permissions at region */
				//printf("CHANGE REGIONS: 0x%x\n", WS_SI(si_addr));
				/* TODO: manage access to protected pages */
				;
				debug_msg_set("pid %d Segmentation fault!\n", ps.tid);
				eprintf("Segmentation fault!\n");
			} else {
				switch(WS_SI(si_signo)){
				case 2:
					debug_msg_set("pid %d has received keyboard interrupt\n", ps.tid);
					eprintf("CHILD PROCESS HAS RECEIVED A KEYBOARD INTERRUPT. REPEAT LAST COMMAND\n");
					break;
				case 3:
					debug_msg_set("pid %d has received a quit from keyboard\n", ps.tid);
					eprintf("CHILD PROCESS HAS RECEIVED A QUIT FROM KEYBOARD\n");
					break;
				case 8:
					debug_msg_set("pid %d floating point exception\n", ps.tid);
					eprintf("FLOATING POINT EXCEPTION\n");
					break;
				case 4:
					debug_msg_set("pid %d illegal instruction\n", ps.tid);
					eprintf("ILLEGAL INSTRUCTION\n");
					break;
				case 18:
					debug_msg_set("pid %d process stopped\n", ps.tid);
					eprintf("THE CHILD IS STOPPED: REPEAT LAST COMMAND AGAIN\n");
					break;
				case 19:
					WS(event) = CLONE_EVENT;
					debug_msg_set("pid %d clone()\n", ps.tid);
					eprintf("CLONE HAS BEEN INVOKED\n");
					break;
				default:
					debug_msg_set("pid %d unknown signal received (%d)\n", ps.tid, WS_SI(si_signo));
					eprintf("Unknown signal %d received\n", WS_SI(si_signo));
				}
			}
		}
	} else
		eprintf("What?\n");

	return 0; /* XXX: should stop */
}

/* HACK: save a hardware/software breakpoint */
int debug_contfork(int tid)
{
	int ret;

#if __linux__ || ( __BSD__ && defined(PT_SYSCALL))
	// XXX ignore tid
	if (ps.opened) {
		//arch_reset_breakpoint(1);
		do {
			ret = ptrace(PTRACE_SYSCALL, ps.tid, 0, 0);
			if (ret != 0) {
				debug_msg_set("ptrace syscall error\n");
				eprintf("ptrace_syscall error\n");
				return 1;
			}

			debug_dispatch_wait();
			debug_print_wait("contsc");

		} while(!arch_is_fork() && ( WS(event) != EXIT_EVENT ) );
		
		debug_print_wait("contfork");

		return 0;
	}
#else
	eprintf("contfork: oops\n");
#endif

	eprintf(":contsc No program loaded.\n");
	return 1;
}
