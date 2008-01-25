/*
 * Copyright (C) 2007
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

#include "libps2fd.h"
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
					&tmp, tmp2, &tmp3, &mola);
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
					&tmp, tmp2, &tmp3, &mola);
				if (mola == pid) {
					pids_cmdline(p, tmp2);
					//for(i=0; i<recursive*2;i++)
					//	printf(" ");
					printf(" `- %d : %s\n", p, tmp2);
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

int pids_sons_of(int pid)
{
	char buf[512];
	pids_cmdline(pid, buf);
	printf("-+- %d : %s\n",pid, buf);
	return pids_sons_of_r(pid,0,999);
}

int pids_list()
{
	int i, fd;
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
