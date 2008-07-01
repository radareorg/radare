/*
 * Copyright (C) 2007
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

static ssize_t posix_write(int fd, const void *buf, size_t count)
{
        return write(fd, buf, count);
}

static ssize_t posix_read(int fd, void *buf, size_t count)
{
        return read(fd, buf, count);
}

static int posix_open(const char *pathname, int flags, mode_t mode)
{
	return open(pathname, flags, mode);
}

static int posix_close(int fd)
{
	return close(fd);
}

static u64 posix_lseek(int fildes, u64 offset, int whence)
{
#if __WINDOWS__ 
	return _lseek(fildes,(long)offset,whence);
#else
#if __linux__
	return lseek64(fildes,(off_t)offset,whence);
#else
	return lseek(fildes,(off_t)offset,whence);
#endif
#endif
}

static int posix_handle_fd(int fd)
{
	return 1;
}

static int posix_handle_open(const char *file)
{
	return 1;
}

plugin_t posix_plugin = {
	.name        = "posix",
	.desc        = "plain posix file access",
	.init        = NULL,
	.debug       = NULL,
	.system      = NULL,
	.handle_fd   = posix_handle_fd,
	.handle_open = posix_handle_open,
	.open        = posix_open,
	.read        = posix_read,
	.write       = posix_write,
	.lseek       = posix_lseek,
	.close       = posix_close
};
