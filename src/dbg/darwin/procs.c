/*
 * Copyright (C) 2008
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>

// XXX
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

// XXX
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

	while((file=(struct dirent *)readdir(dh)) ) {
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
