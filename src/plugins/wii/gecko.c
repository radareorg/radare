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

// radare gecko://

#include "../../plugin.h"
#include "../../utils.h"
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include "grecko.h"

static int gecko_fd = -33;

#define GECKO_MAGIC_FD 0x4830

#define uchar unsigned char

ssize_t gecko_write(int fd, const void *buf, size_t count)
{
	return grecko_write(config.seek, buf, (int)count);
}

ssize_t gecko_read(int fd, void *buf, size_t count)
{
	return grecko_read(config.seek, buf, (int)count);
}

int gecko_open(const char *pathname, int flags, mode_t mode)
{
	if (grecko_open())
		return -1;
	return GECKO_MAGIC_FD;
}

int gecko_close(int fd)
{
	return grecko_close();
}

off_t gecko_lseek(int fildes, off_t offset, int whence)
{
	switch(whence) {
	case SEEK_SET:
		return offset;
	case SEEK_CUR:
		return config.seek+offset;
	case SEEK_END:
		return config.size; //0xffffffff;
	}
	return offset;
}

int gecko_handle_fd(int fd)
{
	return (fd == gecko_fd);
}

int gecko_handle_open(const char *file)
{
	if (!memcmp(file, "gecko://", 6))
		return 1;
	return 0;
}

int gecko_system(const char *cmd)
{
	uchar buf[1024];
	char *ptr;
	int i;

	if (cmd[0] == '!')
		return system(cmd+1);

	if (strstr(cmd, "help") || strchr(cmd, '?')) {
		printf("Usb-Gecko plugin help:\n"
		"!load           - load game from dvd\n"
		"!stop           - stops console (freeze)\n"
		"!cont           - continue console execution\n"
		"!regs           - (TODO) get cpu register information.\n"
		"!bp[rwx] [addr] - (TODO) set breakpoint (or wp) for read, write or exec.\n"
		"!shot [file]    - (TODO) dump screenshot to bmp file.\n"
		);
	} else
	if (strstr(cmd, "stop")) {
		grecko_stop();
	} else
	if (strstr(cmd, "cont")) {
		grecko_continue();
	} else
	if (strstr(cmd, "load")) {
		grecko_load();
	}
	return 0;
}

plugin_t gecko_plugin = {
	.name        = "gecko",
	.desc        = "Wii(R) connection via USB-Gecko",
	.init        = NULL,
	.debug       = NULL,
	.system      = gecko_system,
	.handle_fd   = gecko_handle_fd,
	.handle_open = gecko_handle_open,
	.open        = gecko_open,
	.read        = gecko_read,
	.write       = gecko_write,
	.lseek       = gecko_lseek,
	.close       = gecko_close
};
