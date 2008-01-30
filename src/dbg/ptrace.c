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

#include "libps2fd.h"
#include "../plugin.h"

struct debug_t ps;

ssize_t ptrace_write(int fd, const void *buf, size_t count)
{
	if (ps.opened)
		if (ps.fd == fd)
			return debug_write(ps.tid, (long *)buf, count);
	return count;
}

ssize_t ptrace_read(int fd, void *buf, size_t count)
{
	if (ps.opened && ps.fd == fd)
		return debug_read(ps.tid, buf, count);

        return debug_read(fd, buf, count);
}

#define ptrace_close debug_close

int ptrace_handle_fd(int fd)
{
	return (ps.opened && ps.fd == fd);
}

int ptrace_handle_open(const char *file)
{
	if (!strncmp("pid://", file, 6))
		return 1;
	if (!strncmp("dbg://", file, 6))
		return 1;
	return 0;
}

plugin_t ptrace_plugin = {
	.name = "ptrace",
	.desc = "Debugs or attach to a process ( dbg://file or pid://PID )",
	.init = ptrace_init,
	.system = ptrace_system,
	.handle_fd = ptrace_handle_fd,
	.handle_open = ptrace_handle_open,
	.open = ptrace_open,
	.read = ptrace_read,
	.write = ptrace_write,
	.lseek = ptrace_lseek,
	.close = ptrace_close
};
