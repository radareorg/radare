/*
 * Copyright (C) 2008
 *       pancake <youterm.com>
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

#include "../../config.h"
#include "../../main.h"
#include "../../code.h"
#include "../libps2fd.h"
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int pids_cmdline(int pid, char *cmdline)
{
	int fd;
	sprintf(cmdline, "/proc/%d/cmdline", pid);
	fd = open(cmdline, O_RDONLY);
	cmdline[0] = '\0';
	if (fd != -1) {
		read(fd, cmdline, 1024);
		cmdline[1024] = '\0';
		close(fd);
	}

	return 0;
}

/* ptrace attaches to all sons of a pid */
int pids_ptrace_all(int pid)
{
// XXX lot of c&p shit here lol!1
	int p,i;
	int n = 0;
	int mola;
	char buf[128];
	int tmp;
	char tmp2[1024];
	char tmp3[8];
	struct dirent *file;
	FILE *fd;
	DIR *dh = opendir("/proc/");

	if (dh == NULL)
		return 0;

	while((file=readdir(dh)) ) {
		p = atoi(file->d_name);
		if (p) {
			sprintf(buf,"/proc/%s/stat", file->d_name);
			fd = fopen(buf, "r");
			if (fd) {
				mola = 0;
				fscanf(fd,"%d %s %s %d",
					&tmp, tmp2, tmp3, &mola);
				if (mola == pid) {
					//pids_cmdline(p, tmp2);
					//for(i=0; i<recursive*2;i++)
					//	printf(" ");
					//printf(" `- %d : %s\n", p, tmp2);
					/* do not change -attach- stuff here, only make the kernel more conscient about us */
					ptrace(PTRACE_ATTACH, p, 0, 0);
					n++;
				}
			}
			fclose(fd);
		}
	}
	return n;
}

int pids_sons_of_r(int pid, int recursive, int limit)
{
	int p;
	int n = 0;
	int mola;
	char buf[128];
	int tmp;
	char tmp2[1024];
	char tmp3[8];
	struct dirent *file;
	FILE *fd;
	DIR *dh = opendir("/proc/");

	if (pid == 0 || dh == NULL)
		return 0;

	while((file=readdir(dh)) ) {
		p = atoi(file->d_name);
		if (p) {
			sprintf(buf,"/proc/%s/stat", file->d_name);
			fd = fopen(buf, "r");
			if (fd) {
				mola = 0;
				fscanf(fd,"%d %s %s %d",
					&tmp, tmp2, tmp3, &mola);
				if (mola == pid) {
					pids_cmdline(p, tmp2);
					//for(i=0; i<recursive*2;i++)
					//	printf(" ");
					cons_printf(" `- %d : %s (%s)\n", p, tmp2, (tmp3[0]=='S')?"sleeping":(tmp3[0]=='T')?"stopped":"running");
					n++;
					if (recursive<limit)
						n+=pids_sons_of_r(p, recursive+1, limit);
				}
			}
			fclose(fd);
		}
	}
	return n;
}

#if 0
int pids_sons_of(int pid)
{
	char buf[512];
	pids_cmdline(pid, buf);
	printf("-+- %d : %s\n",pid, buf);
	return pids_sons_of_r(pid,0,999);
}
#endif

int pids_list()
{
	int i;
	char cmdline[1025];

	// TODO: use ptrace to get cmdline from esp like tuxi does
	for(i=2;i<999999;i++) {
		switch( kill(i, 0) ) {
		case 0:
			pids_cmdline(i, cmdline);
			printf("%d : %s\n", i, cmdline);
			break;
//		case -1:
//			if (errno == EPERM)
//				printf("%d [not owned]\n", i);
//			break;
		}
	}
}


#if 0
main(int argc, char **argv)
{
	printf("(%d)\n", pids_sons_of(atoi(argv[1])));
	return 0;
}
#endif

static const char *debug_unix_pid_running  = "running";
static const char *debug_unix_pid_stopped  = "stopped";
static const char *debug_unix_pid_sleeping = "sleeping";
static const char *debug_unix_pid_unknown  = "unknown";
static const char *debug_unix_pid_zombie   = "zombie";

const char *debug_unix_pid_status(int pid)
{
	const char *str = debug_unix_pid_unknown;
	char buf[129];
	FILE *fd;

	sprintf(buf, "/proc/%d/status", pid);
	fd = fopen(buf, "r");
	if (fd) {
		while(1) {
			buf[0]='\0';
			fgets(buf, 128, fd);
			if (buf[0]=='\0')
				break;
			if (strstr(buf, "State:")) {
				if (strstr(buf, "sleep"))
					str = debug_unix_pid_sleeping;
				else
				if (strstr(buf, "running"))
					str = debug_unix_pid_running;
				else
				if (strstr(buf, "stop"))
					str = debug_unix_pid_stopped;
				else
				if (strstr(buf, "zombie")) // XXX ?
					str = debug_unix_pid_zombie;
			}
		}
		fclose(fd);
	}
		
	return str;
}

int debug_pstree(const char *input)
{
	int foo, tid = 0;
	const char *ptr;

	if (input)
		tid = atoi(input);

	if (tid != 0) {
		if (strstr(input, "stop")) {
			kill(tid, SIGSTOP);
			eprintf("stop for %d\n", tid);
		} else
		if (strstr(input, "cont")) {
			kill(tid, SIGCONT);
			eprintf("running for %d\n", tid);
		} else
		if (strstr(input, "segv")) {
			kill(tid, SIGSEGV);
			eprintf("segv for %d\n", tid);
		} else
		if (strstr(input, "kill")) {
			kill(tid, SIGKILL);
			eprintf("Current selected thread id (pid): %d\n", ps.tid);
		} else
		if (input && strstr(input, "?")) {
			cons_printf("Usage: !pid [pid] [stop|cont|segv|kill]\n");
		} else {
			ps.tid = tid;
			eprintf("Current selected thread id (pid): %d\n", ps.tid);
		}
		// XXX check if exists or so
		return 0;
	}

	foo = ps.pid;
	eprintf(" pid : %d 0x%08llx (%s)\n", foo, arch_pc(foo), debug_unix_pid_status(foo));
	pids_sons_of_r(foo, 0, 0);
	if (ps.pid != ps.tid) {
		foo = ps.tid;
		eprintf(" pid : %d 0x%08llx (%s)\n", foo, arch_pc(foo), debug_unix_pid_status(foo));
		pids_sons_of_r(foo, 0, 0);
	}

	return 0;
}

