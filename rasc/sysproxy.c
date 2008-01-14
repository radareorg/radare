/*
 * Copyright (C) 2007
 *       pancake <youterm.com>
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

#include "../src/main.h"
#include "../src/plugin.h"
#include "syscalls.h"
#include <stdio.h>
#include <sys/types.h>

extern int rpc_init(char *host, int port);
extern int sysproxy_handle_fd(int fd);

static int opened = 0;
static int spfd  = -1; // syscall-proxy socket
static int spfd2 = -1; // remote file descriptor

int sysproxy_open(const char *file, int n, mode_t mode)
{
	char host[128];
	char *ptr;
	int port = 9999;

	/* connect and so */
	strcpy(host, file+11);
	ptr = strchr(host, ':');
	if (ptr) {
		ptr[0] = 0;
		port = atoi(ptr+1);
	}

	printf("syscall-proxy connecting to %s at port %d.. ", host, port);
	fflush(stdout);

	spfd = rpc_init(host,port);
	//spfd = socket_connect(host, port);
	printf("%s\n", (spfd!=-1)?"connected":"error");
	if (spfd != -1) opened = 1; else return -1;

	ptr = strchr(ptr+1,'/');
	if (ptr==0) {
		printf("No file specified. try: sysproxy://host:port/file\n");
		return 1;
	}
	printf("syscall-proxy: opening file %s.. ", ptr);
	spfd2 = sys_open(ptr, 0, 0644); // XXX read only
	if (spfd2 == -1) {
		printf("Cannot open remote file '%s'\n", ptr);
		close(spfd);
		return -1;
	}

	return (ssize_t)spfd;
}

ssize_t sysproxy_write(int fd, const void *buf, size_t count)
{
	if (sysproxy_handle_fd(fd))
		return sys_write(spfd2, (char *)buf, count);
	return 0;
}

ssize_t sysproxy_read(int fd, void *buf, size_t count)
{
	if (sysproxy_handle_fd(fd))
		return sys_read(spfd2, buf, count);
        return 0;
}

off_t sysproxy_lseek(int fd, off_t offset, int whence)
{
	if (sysproxy_handle_fd(fd))
		return sys_lseek(spfd2, (int) offset, whence);
        return offset;
}

ssize_t sysproxy_close(int fd)
{
	if (sysproxy_handle_fd(fd))
		return sys_close(spfd2);
	/* disconnect */
	close(spfd);
	return -1;
}

int sysproxy_handle_fd(int fd)
{
	return (opened && spfd == fd);
}

int sysproxy_handle_open(const char *file)
{
	if (!memcmp("sysproxy://", file, 11))
		return 1;
	return 0;
}

plugin_t sysproxy_plugin = {
	.name = "sysproxy",
	.desc = "IO redirected to a sysproxy server ( sysproxy://host:port )",
	.init = NULL,
	.system = NULL,
	.handle_fd = &sysproxy_handle_fd,
	.handle_open = &sysproxy_handle_open,
	.open = &sysproxy_open,
	.read = &sysproxy_read,
	.write = &sysproxy_write,
	.lseek = &sysproxy_lseek,
	.close = &sysproxy_close
};
