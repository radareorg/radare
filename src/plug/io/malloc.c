/*
 * Copyright (C) 2008
 *       pancake <@youterm.com>
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

#include <main.h>
#include <stdio.h>
#include <stdlib.h>
#include <plugin.h>
#include <sys/types.h>

#define MALLOC_FD 98479
static int malloc_fd = -1;
static unsigned char *malloc_buf = NULL;
static unsigned int malloc_bufsz = 0;

static ssize_t malloc_write(int fd, const void *buf, size_t count)
{
	if (malloc_buf == NULL)
		return 0;
	return (ssize_t)memcpy(malloc_buf+config.seek, buf, count);
}

static ssize_t malloc_read(int fd, void *buf, size_t count)
{
	if (malloc_buf == NULL)
		return 0;

	if (config.seek + count > config.size) {
		//config.seek = 0; // ugly hack
		//count = config.seek+count-config.size;
		return 0;
	}
	if (config.seek + count > config.size)
		config.seek = config.size;

	return (ssize_t)memcpy(buf, malloc_buf+config.seek, count);
}

static int malloc_close(int fd)
{
	if (malloc_buf == NULL)
		return -1;
	free(malloc_buf);
	malloc_buf = malloc(malloc_bufsz);
	return 0;
}

extern u64 posix_lseek(int fildes, u64 offset, int whence);
static u64 malloc_lseek(int fildes, u64 offset, int whence)
{
	switch(whence) {
	case SEEK_SET:
		return offset;
	case SEEK_CUR:
		return config.seek+offset;
	case SEEK_END:
		return malloc_bufsz;
	}
	return offset;
}

static int malloc_handle_fd(int fd)
{
	return (fd == malloc_fd);
}

static int malloc_handle_open(const char *pathname)
{
	return (!memcmp(pathname, "malloc://", 9));
}

static int malloc_open(const char *pathname, int flags, mode_t mode)
{
	char buf[1024];
	char *ptr = buf;

	strncpy(buf, pathname, 1000);

	if (!memcmp(ptr , "malloc://", 9)) {
		ptr = ptr+6;
		// connect
		malloc_fd = MALLOC_FD;
		malloc_bufsz = atoi(pathname+9);
		malloc_buf = malloc(malloc_bufsz);

		if (malloc_buf == NULL) {
			printf("Cannot allocate (%s)%d bytes\n", pathname+9, malloc_bufsz);
			malloc_buf = NULL;
			malloc_bufsz = 0;
			malloc_fd = -1;
		}
		memset(malloc_buf, '\0', malloc_bufsz);
	}
	return malloc_fd;
}

plugin_t malloc_plugin = {
	.name        = "malloc",
	.desc        = "memory allocation ( malloc://size )",
	.init        = NULL,
	.debug       = NULL,
	.system      = NULL,
	.handle_fd   = malloc_handle_fd,
	.handle_open = malloc_handle_open,
	.open        = malloc_open,
	.read        = malloc_read,
	.write       = malloc_write,
	.lseek       = malloc_lseek,
	.close       = malloc_close
};
