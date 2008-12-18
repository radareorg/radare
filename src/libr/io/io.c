/*
 * Copyright (C) 2008
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

#include "r_io.h"
#include <stdio.h>

static struct r_io_handle_t *plugin;
static int cache_fd;
u64 r_io_seek = 0;

int r_io_init()
{
	r_io_map_init();
	r_io_handle_init();
	return 0;
}

int r_io_open(const char *file, int flags, int mode)
{
	struct r_io_handle_t *plugin = r_io_handle_resolve(file);
	if (plugin) {
		int fd = plugin->open(file, flags, mode);
		if (fd != -1)
			r_io_handle_open(fd, plugin);
		return fd;
	}
	return open(file, flags, mode);
}

int r_io_read(int fd, u8 *buf, int len)
{
	if (r_io_map_read_at(r_io_seek, buf, len) != 0)
		return len;
	if (fd != cache_fd)
		plugin = r_io_handle_resolve_fd(fd);
	if (plugin) {
		cache_fd = fd;
		return plugin->read(fd, buf, len);
	}
	return read(fd, buf, len);
}

int r_io_resize(const char *file, int flags, int mode)
{
#if 0
	/* TODO */
	struct r_io_handle_t *plugin = r_io_handle_resolve(file);
	if (plugin) {
		int fd = plugin->open(file, flags, mode);
		if (fd != -1)
			r_io_handle_open(fd, plugin);
		return fd;
	}
#endif
	return -1;
}

int r_io_write(int fd, const u8 *buf, int len)
{
	if (r_io_map_write_at(r_io_seek, buf, len) != 0)
		return len;
	if (fd != cache_fd)
		plugin = r_io_handle_resolve_fd(fd);
	if (plugin) {
		cache_fd = fd;
		return plugin->write(fd, buf, len);
	}
	return write(fd, buf, len);
}

u64 r_io_lseek(int fd, u64 offset, int whence)
{
	/* pwn seek value */
	switch(whence) {
	case R_IO_SEEK_SET:
		r_io_seek = offset;
		break;
	case R_IO_SEEK_CUR:
		r_io_seek += offset;
		break;
	case R_IO_SEEK_END:
		r_io_seek = 0xffffffff;
		break;
	}

	if (fd != cache_fd)
		plugin = r_io_handle_resolve_fd(fd);
	if (plugin) {
		cache_fd = fd;
		return plugin->lseek(fd, offset, whence);
	}
	// XXX can be problematic on w32..so no 64 bit offset?
	return lseek(fd, offset, whence);
}

int r_io_system(int fd, const char *cmd)
{
	if (fd != cache_fd)
		plugin = r_io_handle_resolve_fd(fd);
	if (plugin) {
		cache_fd = fd;
		return plugin->system(cmd);
	}
	return system(cmd);
}

int r_io_close(int fd)
{
	if (fd != cache_fd)
		plugin = r_io_handle_resolve_fd(fd);
	if (plugin) {
		cache_fd = fd;
		r_io_handle_close(fd, plugin);
		return plugin->close(fd);
	}
	return close(fd);
}
