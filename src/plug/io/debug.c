/*
 * Copyright (C) 2007
 *       th0rpe <nopcode.org>
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

#include <plugin.h>
#include <dbg/libps2fd.h>

struct debug_t ps;

struct debug_t debug_debugt;

static ssize_t dbg_write(int fd, const void *buf, size_t count)
{
	if (ps.opened)
		if (ps.fd == fd)
			return debug_write(ps.tid, (long *)buf, count);
	return count;
}

static ssize_t dbg_read(int fd, void *buf, size_t count)
{
	if (ps.opened && ps.fd == fd)
		return debug_read(ps.tid, buf, count);

        return debug_read(fd, buf, count);
}

#define debug_close debug_close

static int dbg_handle_fd(int fd)
{
	return (ps.opened && ps.fd == fd);
}

static int dbg_handle_open(const char *file)
{
	if (!strncmp("pid://", file, 6))
		return 1;
	if (!strncmp("dbg://", file, 6))
		return 1;
	return 0;
}

plugin_t debug_plugin = {
	.name = "debug",
	.desc = "Debugs or attach to a process ( dbg://file or pid://PID )",
	.init = debug_init,
	.debug = &debug_debugt,
	.system = debug_system,
	.handle_fd = dbg_handle_fd,
	.handle_open = dbg_handle_open,
	.open = debug_open,
	.read = dbg_read,
	.write = dbg_write,
	.lseek = debug_lseek,
	.close = debug_close
};
