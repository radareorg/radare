/*
 * Copyright (C) 2007, 2008
 *       pancake <pancake@youterm.com>
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ptrace.h>

struct debug_t ps;

static void libps2fd_init() __attribute__ ((constructor));
static void libps2fd_init()
{
	ptrace_init();

	__open    = dlsym(RTLD_NEXT, "open");
	__open64  = dlsym(RTLD_NEXT, "open64");
	__read    = dlsym(RTLD_NEXT, "read");
	//__read64  = dlsym(RTLD_NEXT, "read64");
	__write   = dlsym(RTLD_NEXT, "write");
	__write64 = dlsym(RTLD_NEXT, "write64");
	__lseek   = dlsym(RTLD_NEXT, "lseek");
	__lseek64 = dlsym(RTLD_NEXT, "lseek64");
	___llseek = dlsym(RTLD_NEXT, "_llseek");
	__close   = dlsym(RTLD_NEXT, "close");
	__system  = dlsym(RTLD_NEXT, "system");

	if ((__open  == NULL)
	|| (__read   == NULL)
	|| (__write  == NULL)
	|| (__close  == NULL)
	|| (__system == NULL)) {
		fprintf(stderr, "Library initialization failed.\n");
		exit(1);
	}
}

static void libps2fd_fini() __attribute__ ((destructor));
static void libps2fd_fini()
{
	if (ps.opened) {
		debug_os_kill(ps.pid, 9);
		debug_close(ps.fd);
		eprintf("Process killed.\n");
	}
}

int open(const char *pathname, int flags, int mode)
{
	return ptrace_open(pathname, flags, mode);
}

int close(int fd)
{
	debug_close(fd);
}

ssize_t write(int fd, const void *buf, size_t count)
{
	return debug_write(fd, buf, count);
}

ssize_t read(int fd, void *buf, size_t count)
{
	return debug_read(fd,buf,count);;
}

int system(const char *command)
{
	return ptrace_system(command);
}
