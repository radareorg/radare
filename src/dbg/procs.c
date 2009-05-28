/*
 * Copyright (C) 2007, 2008, 2009
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
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if __WINDOWS__
int debug_pids()
{
	eprintf("debug_pids(): Not implement for this platform\n");
}
#else
int pids_sons_of(int pid)
{
	char buf[512];
	pids_cmdline(pid, buf); /* this is OS specific */
	printf("-+- %d : %s\n",pid, buf);
	return pids_sons_of_r(pid,0,999);
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
#endif
