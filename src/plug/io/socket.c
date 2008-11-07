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
#include <plugin.h>
#include "../../socket.h"

static int socket_fd = -1;
static unsigned char *socket_buf = NULL;
static unsigned int socket_bufsz = 0;
static unsigned int socket_bufread = 0;

ssize_t zocket_write(int fd, const void *buf, size_t count)
{
        return socket_write(fd, (u8 *)buf, count);
}

ssize_t zocket_read(int fd, void *buf, size_t count)
{
	u8 data[32000];
	int sz;
	u64 s;

	if (config.seek > socket_bufsz)
		config.seek = socket_bufsz;

	if (fd == socket_fd) {
		if (socket_ready(fd, 0, 10)>0) {
			sz = socket_read(fd, data, 32000);
			if (sz == -1) {
				eprintf("Connection closed\n");
				// XXX close foo
			}
			if (sz>0) {
				if (socket_buf)
					socket_buf = (u8 *)realloc(socket_buf, socket_bufsz+sz);
				else 	socket_buf = (u8 *)malloc(socket_bufsz+sz);
				memcpy(socket_buf+(int)socket_bufsz, data, sz);
				sprintf((char *)data, "_sockread_%d", socket_bufread++);
				flag_set((char *)data, socket_bufsz, 0);
				flag_set("_sockread_last", socket_bufsz, 0);
				socket_bufsz += sz;
			}
		}
		if (config.seek < socket_bufsz) {
			s = count;
			if (count+config.seek > socket_bufsz)
				s = socket_bufsz-config.seek;
			memcpy(buf, socket_buf+config.seek, s);
			return s;
		}
	}
        return 0;
}

int zocket_close(int fd)
{
	return close(fd);
}

u64 zocket_lseek(int fildes, u64 offset, int whence)
{
	switch(whence) {
	case SEEK_SET:
		return offset;
	case SEEK_CUR:
		if (config.seek+offset>socket_bufsz)
			return socket_bufsz;
		return config.seek+offset;
	case SEEK_END:
		return 0xffffffff;
	}
	return 0;
}

int zocket_handle_fd(int fd)
{
	return (fd == socket_fd);
}

int zocket_handle_open(const char *pathname)
{
	return (!memcmp(pathname, "socket://", 9));
}

int zocket_open(const char *pathname, int flags, mode_t mode)
{
	char buf[1024];
	char *ptr = buf;

	strncpy(buf, pathname, 1000);

	if (!memcmp(ptr , "socket://", 9)) {
		ptr= ptr+9;
		// port
		char *port = strchr(ptr, ':');
		if (port == NULL) {
			printf("No port defined.\n");
			return -1;
		}
		port[0] = '\0';
		if (strlen(ptr)==0) {
			// LISTEN HERE
			return -1;
		}
#if 0
		file   = strchr(pathname+12,'/');
		if (file == NULL) {
			printf("No remote file specified.\n");
			return -1;
		}
#endif
		// connect
		socket_fd = socket_connect((char*)ptr, atoi(port+1));
		if (socket_fd>=0)
			printf("Connected to: %s at port %d\n", ptr, atoi(port+1));
		else	printf("Cannot connect to '%s' (%d)\n", ptr, atoi(port+1));
		socket_buf = (unsigned char *)malloc(1);
		socket_bufsz = 0;
		config_set("file.write", "true");
		buf[0]='\0';
	}
	return socket_fd;
}

plugin_t socket_plugin = {
	.name        = "socket",
	.desc        = "socket stream access ( socket://host:port )",
	.init        = NULL,
	.debug       = NULL,
	.system      = NULL,
	.handle_fd   = zocket_handle_fd,
	.handle_open = zocket_handle_open,
	.open        = zocket_open,
	.read        = zocket_read,
	.write       = zocket_write,
	.lseek       = zocket_lseek,
	.close       = zocket_close
};
