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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#define __addr_t_defined

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../../main.h"

u64 debug_fd_seek(int pid, int fd, u64 addr, int whence)
{
	return 0;
}

u64 debug_fd_read(int pid, int fd, u64 addr, int len)
{
	return 0;
}

u64 debug_fd_write(int pid, int fd, u64 addr, int len)
{
	return 0;
}

int debug_fd_list(int pid)
{
	return 0;
}

int debug_fd_dup2(int pid, int oldfd, int newfd)
{
	return 0;
}

int debug_fd_open(int pid, char *file, int mode)
{
	return 0;
}

int debug_fd_close(int pid, int fd)
{
	return 0;
}
