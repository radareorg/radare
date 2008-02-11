/* 
 * (C) Copyright 1992, ..., 2007 the "DOSEMU-Development-Team".
 *
 * for details see file COPYING.DOSEMU in the DOSEMU distribution
 */

/*
 * DOSEMU debugger,  1995 Max Parke <mhp@light.lightlink.com>
 *
 * This is file dosdebug.c
 *
 * Terminal client for DOSEMU debugger v0.2
 * by Hans Lermen <lermen@elserv.ffm.fgan.de>
 * It uses /var/run/dosemu.dbgXX.PID for connections.
 *
 * The switch-console code is from Kevin Buhr <buhr@stat.wisc.edu>
 */

//#include "config.h"
#include "libps2fd.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>	/* for struct timeval */
#include <time.h>	/* for CLOCKS_PER_SEC */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>

#include <sys/vt.h>
#include <sys/ioctl.h>

#define    TMPFILE_VAR		"/var/run/dosemu."
#define    TMPFILE_HOME		".dosemu/run/dosemu."

#define MHP_BUFFERSIZE 8192

#define FOREVER ((((unsigned int)-1) >> 1) / CLOCKS_PER_SEC)
#define KILL_TIMEOUT 2

fd_set readfds;
struct timeval timeout;
int kill_timeout=FOREVER;

static  char *pipename_in, *pipename_out;
int fdout, fdin;

static int check_pid(int pid);

#define GETCOLOR int color = getv("COLOR");
#define TITLE if (color) cons_printf("\e[36m");
#define TITLE_END if (color) cons_printf("\e[0m");

int help_message()
{
	GETCOLOR
	cons_printf("Usage: :!<cmd> (args)\n");
	TITLE
	cons_printf(" Information\n");
	TITLE_END
#if 0
	printf("  info            show debugger and process status\n");
	printf("  status          show the contents of /proc/pid/status\n");
	printf("  pids            show the pids of all the attachable processes\n");
#endif
	cons_printf("  bt              backtrace stack frames\n");
	cons_printf("  maps[*]         flag ldt\n");
	cons_printf("  syms            flags all syms of the debugged program\n");
	cons_printf("  fd[?][-#] [arg] file descriptors (fd? for help)\n");
	cons_printf("  th[?]           threads control command\n");
	TITLE
	cons_printf(" Memory allocation\n");
	TITLE_END
	cons_printf("  alloc [N]       allocates N bytes (no args = list regions)\n");
	cons_printf("  mmap [F] [off]  mmaps a file into a memory region\n");
	cons_printf("  free            free allocated memory regions\n");
	cons_printf("  imap            map input stream to a memory region (DEPRECATED)\n");
	TITLE
	cons_printf(" Loader\n");
	TITLE_END
	cons_printf("  run             load and start execution\n");
	cons_printf("  load            load a program in memory\n");
	cons_printf("  unload          unload a program or kill\n");
	cons_printf("  kill [-S] [pid] sends a signal to a process\n");
	cons_printf("  attach [pid]    attach target pid\n");
	cons_printf("  detach          detach debugged process\n");
	TITLE
	cons_printf(" Flow Control\n");
	TITLE_END
	cons_printf("  jmp [addr]      force program counter\n");
	cons_printf("  call [addr]     enters a subroutine\n");
	cons_printf("  ret             emulates a return from subroutine\n");
	cons_printf("  step [N]        (s) step (N=1) instruction(s)\n");
	cons_printf("  stepo           (so) step over calls and repz\n");
	cons_printf("  trace [N]       trace until bp or eof at N debug level\n");
	cons_printf("  wtrace          trace until a watchpoint is matched\n");
	cons_printf("  stepret         continue until ret (TODO)\n");
	cons_printf("  cont            continue until bp, eof\n");
	cons_printf("  contu ([addr])  continue until user code, bp, eof (or addr if defined)\n");
	cons_printf("  contsc          continue until next syscall\n");
	cons_printf("  contfork        continue until fork\n");
	TITLE
	cons_printf(" Breakpoints\n");
	TITLE_END
	cons_printf("  bp [addr]       put a breakpoint at addr (no arg = list)\n");
	TITLE
	cons_printf(" Registers\n");
	TITLE_END
	cons_printf("  [o]regs[*]      show registers (o=old, *=radare)\n");
	cons_printf("  oregs[*]        show old registers information (* for radare)\n");
	cons_printf("  set [reg] [val] set a value for a register\n");
#if __i386__
	cons_printf("  dr[rwx-]        DR registers control (dr? for help) (x86 only)\n");
#endif
	TITLE
	cons_printf(" Actions\n");
	TITLE_END
	cons_printf("  dump [N]        dump pages (and registers) to disk\n");
	cons_printf("  restore [N]     restore pages (and registers) from disk\n");
	cons_printf("  hack [N]        Make a hack.\n");
	cons_printf("  inject [bin]    inject code inside child process\n");

	return 0;
}

int debug_registers(int rad)
{
	char buf[1024];
	if (!args) {
		eprintf("Usage: !set [reg] [value]\n");
		return 1;
	}
	write(outfd, "r\n", 2);
}

int debug_set_register(char *args)
{
	char buf[1024];
	if (!args) {
		eprintf("Usage: !set [reg] [value]\n");
		return 1;
	}
	snprintf(buf, 1000, "r %s\n", args);
	write(outfd, buf, strlen(buf));
}

static struct commads_t {
	char *name;
	int type;
	int (*callback)(char *);
} commands[] = {
	{ "help", CB_NOARGS, &help_message },
	{ "?", CB_NOARGS, &help_message },
	{ "regs", CB_ASTERISK, &debug_registers },
	{ "oregs", CB_ASTERISK, &debug_oregisters },
	{ "set", CB_NORMAL, &debug_set_register },
#if 0
	{ "run", CB_NOARGS, &debug_run },
	{ "stepo", CB_NOARGS, &debug_stepo },
	{ "step", CB_INT, &debug_step },
	{ "bp",  CB_NORMAL, &debug_bp },
	{ "bt",  CB_NORMAL, &debug_bt },
	{ "fd",  CB_NORMAL, &debug_fd },
	{ "th",  CB_NORMAL, &debug_th },
	{ "hack",  CB_SPACE, &arch_hack },
	{ "maps", CB_NORMAL, &debug_print_maps },
	{ "syms", CB_NORMAL, &debug_syms },
	{ "alloc", CB_SPACE, &debug_alloc },
	{ "mmap", CB_SPACE, &debug_mmap },
	{ "free", CB_SPACE, &debug_free},
	{ "imap", CB_SPACE, &debug_imap },
	{ "core", CB_NOARGS, &debug_dumpcore },
	{ "dump", CB_SPACE, &page_dumper },
	{ "restore", CB_SPACE, &page_restore },
	{ "status",   CB_NOARGS, &debug_status },
	{ "pids", CB_NOARGS , &debug_pids },
	{ "pstree", CB_NOARGS, &debug_pstree },
	{ "attach", CB_INT, &debug_attach },
	{ "detach", CB_NOARGS, &debug_detach },
	{ "load", CB_NOARGS, &debug_load },
	{ "unload", CB_NOARGS, &debug_unload },
	{ "ret", CB_NOARGS, &arch_ret },
	{ "jmp ", CB_NORMAL, &arch_jmp },
	{ "call ", CB_NORMAL, &arch_call },
	{ "info", CB_NORMAL, &debug_info },
	{ "set", CB_NORMAL, &debug_set_register },
	{ "wp", CB_NORMAL, &debug_wp},
	{ "inject ", CB_NORMAL, &debug_inject },
	{ "trace", CB_NORMAL, &debug_trace },
	{ "wtrace", CB_NOARGS, &debug_wtrace },
	{ "signal", CB_SPACE, &debug_signal },
	{ "contsc", CB_NORMAL, &debug_contsc },
	{ "contfork", CB_NOARGS, &debug_contfork },
	{ "contu", CB_NOARGS, &debug_contu },
	{ "cont", CB_NOARGS, &debug_cont },
#endif
	{ NULL, 0 }
};

static int find_dosemu_pid(char *tmpfile, int local)
{
  DIR *dir;
  struct dirent *p;
  char *dn, *id;
  int i,j,pid=-1;
  static int once =1;

  dn = strdup(tmpfile);
  j=i=strlen(dn);
  while (dn[i--] != '/');  /* remove 'dosemu.dbgin' */
  i++;
  dn[i++]=0;
  id=dn+i;
  j-=i;
  
  dir = opendir(dn);
  if (!dir) {
    free(dn);
    if (local) return -1;
    fprintf(stderr, "can't open directory %s\n",dn);
    exit(1);
  }
  i = 0;
  while((p = readdir(dir))) {
          
    if(!strncmp(id,p->d_name,j) && p->d_name[j] >= '0' && p->d_name[j] <= '9') {
      pid = strtol(p->d_name + j, 0, 0);
      if(check_pid(pid)) {
        if(once && i++ == 1) {
          fprintf(stderr,
            "Multiple dosemu processes running or stalled files in %s\n"
            "restart dosdebug with one of the following pids as first arg:\n"
            "%d", dn, pid
          );
          once = 0;
        }
      }
      if (i > 1) fprintf(stderr, " %d", pid);
    }
  }
  free(dn);
  closedir(dir);
  if (i > 1) {
    fprintf(stderr, "\n");
    if (local) return -1;
    exit(1);
  }
  if (!i) {
    if (local) return -1;
    fprintf(stderr, "No dosemu process running, giving up.\n");
    exit(1);
  }

  return pid;
}

static int check_pid(int pid)
{
  int fd;
  char buf[32], name[128];
  
  memset(buf, 0, sizeof(buf));
  sprintf(name, "/proc/%d/stat", pid);
  if ((fd = open(name, O_RDONLY)) ==-1) return 0;
  if (read(fd, buf, sizeof(buf)-1) <=0) {
    close(fd);
    return 0;
  }
  close(fd);
  return strstr(buf,"dos") ? 1 : 0;
}

static int switch_console(char new_console)
{
  int newvt;
  int vt;

  if ((new_console < '1') || (new_console > '8')) {
    fprintf(stderr,"wrong console number\n");
    return -1;
  }

  newvt = new_console & 15;
  vt = open( "/dev/tty1", O_RDONLY );
  if( vt == -1 ) {
    perror("open(/dev/tty1)");
    return -1;
  }
  if( ioctl( vt, VT_ACTIVATE, newvt ) ) {
    perror("ioctl(VT_ACTIVATE)");
    return -1;
  }
  if( ioctl( vt, VT_WAITACTIVE, newvt ) ) {
    perror("ioctl(VT_WAITACTIVE)");
    return -1;
  }

  close(vt);
  return 0;
}


void handle_system( char *cmd)
{
	int i;
	for(i=0;commands[i].name;i++) {
		int len = strlen(commands[i].name);
		if (!memcmp(command, commands[i].name, len)) {
			switch(commands[i].type) {
			case CB_ASTERISK:
				return commands[i].callback((int)strchr(command+len,'*'));
			case CB_NORMAL:
				return commands[i].callback(command+len);
			case CB_NOARGS:
				return commands[i].callback(NULL);
			case CB_SPACE:
				return commands[i].callback((int)strchr(command+len,' '));
			case CB_INT:
				return commands[i].callback((int)atoi(command+len));
			}
		}
	}
	return -1;
}

static void read_dbg_input(void)
{
  char buf[MHP_BUFFERSIZE], *p;
  int n;
  do {
    n=read(fdin, buf, sizeof(buf));
  } while (n < 0 && errno == EAGAIN);
  if (n >0) {
    if ((p=strchr(buf,1))!=NULL) n=p-buf;
    write(1, buf, n);
    if (p!=NULL) exit(0);
  }
  if (n == 0)
    exit(1);
}

static void handle_dbg_input(void)
{
  char buf[MHP_BUFFERSIZE], *p;
  int n;
  do {
    n=read(fdin, buf, sizeof(buf));
  } while (n < 0 && errno == EAGAIN);
  if (n >0) {
    if ((p=strchr(buf,1))!=NULL) n=p-buf;
    write(1, buf, n);
    if (p!=NULL) exit(0);
  }
  if (n == 0)
    exit(1);
}


static void handle_console_input(void)
{
  char buf[MHP_BUFFERSIZE];
  static char sbuf[MHP_BUFFERSIZE]="\n";
  static int sn=1;
  int n;
  
  n=read(0, buf, sizeof(buf));
  if (n>0) {
    if (n==1 && buf[0]=='\n') write(fdout, sbuf, sn);
    else {
      if (!strncmp(buf,"console ",8)) {
        switch_console(buf[8]);
        return;
      }
      if (!strncmp(buf,"kill",4)) {
        kill_timeout=KILL_TIMEOUT;
      }
buf[n-1]='\0';
handle_system(buf);
/*
      write(fdout, buf, n);
*/
      memcpy(sbuf, buf, n);
      sn=n;
      if (buf[0] == 'q') exit(1);
    }
  }
}

int main (int argc, char **argv)
{
  int numfds,dospid;
  char *home_p;
  
  FD_ZERO(&readfds);

  home_p = config_get("dir.home");
  
  if (!argv[1]) {
    dospid = -1;
    if (home_p) {
      char *dosemu_tmpfile_pat;
      asprintf(&dosemu_tmpfile_pat, "%s/" TMPFILE_HOME "dbgin.", home_p);
      dospid=find_dosemu_pid(dosemu_tmpfile_pat, 1);
      free(dosemu_tmpfile_pat);
    }
    if (dospid == -1) {
      dospid=find_dosemu_pid(TMPFILE_VAR "dbgin.", 0);
    }
  }  
  else dospid=strtol(argv[1], 0, 0);

  if (!check_pid(dospid)) {
    fprintf(stderr, "no dosemu running on pid %d\n", dospid);
    exit(1);
  }
  
  /* NOTE: need to open read/write else O_NONBLOCK would fail to open */
  fdout = -1;
  if (home_p) {
    asprintf(&pipename_in, "%s/%sdbgin.%d", home_p, TMPFILE_HOME, dospid);
    asprintf(&pipename_out, "%s/%sdbgout.%d", home_p, TMPFILE_HOME, dospid);
    fdout = open(pipename_in, O_RDWR | O_NONBLOCK);
    if (fdout == -1) {
      free(pipename_in);
      free(pipename_out);
    }
  }
  if (fdout == -1) {
    /* if we cannot open pipe and we were trying $HOME/.dosemu/run directory,
       try with /var/run/dosemu directory */
    asprintf(&pipename_in, TMPFILE_VAR "dbgin.%d", dospid);
    asprintf(&pipename_out, TMPFILE_VAR "dbgout.%d", dospid);
    fdout = open(pipename_in, O_RDWR | O_NONBLOCK);
  }
  if (fdout == -1) {
    perror("can't open output fifo");
    exit(1);
  }
  if ((fdin = open(pipename_out, O_RDONLY | O_NONBLOCK)) == -1) {
    close(fdout);
    perror("can't open input fifo");
    exit(1);
  }

  write(fdout,"r0\n",3);
  do {
    FD_SET(fdin, &readfds);
    FD_SET(0, &readfds);   /* stdin */
    timeout.tv_sec=kill_timeout;
    timeout.tv_usec=0;
    numfds=select( fdin+1 /* max number of fds to scan */,
                   &readfds,
                   NULL /*no writefds*/,
                   NULL /*no exceptfds*/, &timeout);
    if (numfds > 0) {
      if (FD_ISSET(0, &readfds)) handle_console_input();
      if (FD_ISSET(fdin, &readfds)) handle_dbg_input();
    }
    else {
      if (kill_timeout != FOREVER) {
        if (kill_timeout > KILL_TIMEOUT) {
          struct stat st;
          int key;
          if (stat(pipename_in,&st) != -1) {
            fprintf(stderr, "...oh dear, have to do kill SIGKILL\n");
            kill(dospid, SIGKILL);
            fprintf(stderr, "dosemu process (pid %d) is killed\n",dospid);
            fprintf(stderr, "If you want to switch to an other console,\n"
                            "then enter a number between 1..8, else just type enter:\n");
            key=fgetc(stdin);
            if ((key>='1') && (key<='8')) switch_console(key);
            fprintf(stderr,
              "dosdebug terminated\n"
              "NOTE: If you had a totally locked console,\n" 
              "      you may have to blindly type in 'kbd -a; texmode'\n"
              "      on the console you switched to.\n"
            );
          }
          else fprintf(stderr, "dosdebug terminated, dosemu process (pid %d) is killed\n",dospid);
          exit(1);
        }
        fprintf(stderr, "no reaction, trying kill SIGTERM\n");
        kill(dospid, SIGTERM);
        kill_timeout += KILL_TIMEOUT;
      }
    }
  } while (1);
  return 0;
}
