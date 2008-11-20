/*
 * Copyright (C) 2007, 2008
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "debug.h"
#include "../main.h"

int fdio_type = 0; // stream or file?
int fdio_enabled = 0;
int fdio_fd = 0;

static u64 bufaddr = 0;

// XXX: fill buffer with 0xff ?
int debug_fd_read_at(pid_t pid, u8 *buf, int length, u64 addr)
{
	int len;
	fdio_enabled = 0;
	if (bufaddr == 0)
		bufaddr = alloc_page(1024*32); // 32K
	if (bufaddr == 0) {
		eprintf("null addr\n");
		fdio_enabled = 1;
		return -1;
	}
	debug_fd_seek(pid, fdio_fd, addr, SEEK_SET);
	len = debug_fd_read(pid, fdio_fd, bufaddr, length);
	debug_os_read_at(pid, buf, len, bufaddr);
	fdio_enabled = 1;
	return len;
}

int debug_fd_write_at(pid_t pid, const u8 *buf, int length, u64 addr)
{
	int len;
	fdio_enabled = 0;
	if (bufaddr == 0)
		bufaddr = alloc_page(1024*32); // 32K
	if (bufaddr == 0) {
		fdio_enabled = 1;
		return -1;
	}
	debug_os_write_at(pid, buf, len, bufaddr);
	debug_fd_seek(pid, fdio_fd, addr, SEEK_SET);
	len = debug_fd_write(pid, fdio_fd, bufaddr, length);
	fdio_enabled = 1;
	return len;
}

int debug_fd_io_mode(int set, int fd)
{
	static u64 oseek = 0;
	fdio_enabled = set;
	fdio_fd = fd;
	if (set) {
		oseek = config.seek;
	} else {
		config.seek = oseek;
	}
	/* TODO: Autodetect fd type */
	return 0;
}

int debug_fd_dump()
{
	/* XXX */
	return radare_cmd("fd* > fd.state", 0);
}

int debug_fd_restore()
{
	/* TODO : close all fds before anything */
	return radare_cmd(".fd.state", 0);
}

int debug_fd_hijack(int fd, const char *file)
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

int debug_fd_close_all(int pid)
{
	int i=0;
	/* TODO : WTF*/
	for (i=0;i<200;i++)
		debug_fd_close(pid, i);
}

int debug_fd(char *cmd)
{
	char *ptr  = NULL,
	     *ptr2 = NULL,
	     *ptr3 = NULL;
	int whence = 0;
	int len = 0;

	if (cmd[0]=='?') {
		cons_printf("Usage: !fd[s|d] [-#] [file | host:port]\n"
		"  !fd                   ; list filedescriptors\n"
		"  !fd*                  ; list fds in radare commands\n"
		"  !fdd 2 7              ; dup2(2, 7)\n"
		"  !fds 3 0x840          ; seek filedescriptor\n"
		"  !fdr 3 0x8048000 100  ; read 100 bytes from fd=3 at 0x80..\n"
		"  !fdw 3 0x8048000 100  ; write 100 bytes from fd=3 at 0x80..\n"
		"  !fdio [fdnum]         ; enter fd-io mode on a fd, no args = back to dbg-io\n"
		"  !fd -1                ; close stdout\n"
		"  !fd -*                ; close all filedescriptors\n"
		"  !fd /etc/motd         ; open file at fd=3\n"
		"  !fd 127.0.0.1:9999    ; open socket at fd=5\n");
		return 0;
	}

	if ((ptr=strchr(cmd,'-'))) {
		if (ptr[1]=='*')
			debug_fd_close_all(ps.tid);
		else debug_fd_close(ps.tid, atoi(ptr+1));
	} else
	if (cmd[0]=='r') {
		ptr = strchr(cmd, ' ');
		if (ptr) {
			*ptr = '\0';
			ptr2 = strchr(ptr+1, ' ');
		}
		if (ptr2) {
			*ptr2 = '\0';
			ptr3 = strchr(ptr2+1, ' ');
		}
		if (ptr3) {
			*ptr3 = '\0';
			len = atoi(ptr3+1);
		}
		if (!ptr||!ptr2||!ptr3)
			eprintf("Usage: !fdr [fd] [offset] [len])\n");
		else debug_fd_read(ps.tid, atoi(ptr+1), get_math(ptr2+1), len);
	} else
	if (cmd[0]=='w') {
		ptr = strchr(cmd, ' ');
		if (ptr) {
			*ptr = '\0';
			ptr2 = strchr(ptr+1, ' ');
		}
		if (ptr2) {
			*ptr2 = '\0';
			ptr3 = strchr(ptr2+1, ' ');
		}
		if (ptr3) {
			*ptr3 = '\0';
			len = atoi(ptr3+1);
		}
		if (!ptr||!ptr2||!ptr3)
			eprintf("Usage: !fdw [fd] [offset] [len])\n");
		else debug_fd_write(ps.tid, atoi(ptr+1), get_math(ptr2+1), len);
	} else
	if (cmd[0]=='i') {
		ptr = strchr(cmd, ' ');
		if (ptr == NULL) {
			debug_fd_io_mode(0, 0);
			eprintf("FDIO disabled\n");
		} else {
			int num = atoi(ptr+1);
			if (num != 0) {
				eprintf("FDIO enabled on %d\n", num);
				debug_fd_io_mode(1, num);
			}
		}
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
	if (cmd[0]=='*') {
		debug_fd_list(ps.tid, 1);
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
		if (ptr[0]!='\0'&&ptr[0]==' ') {
			if (ptr)
				ptr2 = strchr(ptr, ':');
			if (ptr2)
				eprintf("SOCKET NOT YET\n");
			else eprintf("new fd = %d\n", debug_fd_open(ps.tid, ptr, 0));
		} else 
			debug_fd_list(ps.tid, 0);
	} else
		debug_fd_list(ps.tid, 0);

	return 0;
}

