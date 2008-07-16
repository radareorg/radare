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
#if __UNIX__
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

static int shm_fd = -1;
static unsigned char *shm_buf = NULL;
static unsigned int shm_bufsz = 32*1024*1024; /* 32MB */
static unsigned int shm_bufread = 0;

static ssize_t rshm_write(int fd, const void *buf, size_t count)
{
	if (shm_buf != NULL)
        	return memcpy(shm_buf+config.seek, buf, count);
	return -1;
}

static ssize_t rshm_read(int fd, void *buf, size_t count)
{
	if (shm_buf == NULL)
		return -1;
	if (config.seek > shm_bufsz)
		config.seek = shm_bufsz;
	memcpy(buf, shm_buf+config.seek, count);
        return 0;
}

static int rshm_close(int fd)
{
	if (shm_buf == NULL)
		return -1;
	return shmdt(shm_buf);
}

static u64 rshm_lseek(int fildes, u64 offset, int whence)
{
	if (shm_buf == NULL)
		return -1;
	switch(whence) {
	case SEEK_SET:
		return offset;
	case SEEK_CUR:
		if (config.seek+offset>shm_bufsz)
			return shm_bufsz;
		return config.seek+offset;
	case SEEK_END:
		return 0xffffffff;
	}
	return 0;
}

static int rshm_handle_fd(int fd)
{
	return (fd == shm_fd);
}

static int rshm_handle_open(const char *pathname)
{
	return (!memcmp(pathname, "shm://", 6));
}

static int rshm_open(const char *pathname, int flags, mode_t mode)
{
	char buf[1024];
	char *ptr = buf;

	strncpy(buf, pathname, 1000);

	if (!memcmp(ptr , "shm://", 6)) {
		ptr= ptr+6;
		// connect
		shm_buf= shmat(atoi(ptr), 0, 0);

		if (((int)(shm_buf)) != -1) {
			printf("Connected to shared memory 0x%08x\n", atoi(ptr));
			shm_fd = &shm_buf;
		} else	{
			printf("Cannot connect to shared memory (%d)\n", atoi(ptr));
			shm_buf = NULL;
			shm_fd = -1;
		}
	}
	return shm_fd;
}

plugin_t shm_plugin = {
	.name        = "shm",
	.desc        = "shared memory ( shm://key )",
	.init        = NULL,
	.debug       = NULL,
	.system      = NULL,
	.handle_fd   = rshm_handle_fd,
	.handle_open = rshm_handle_open,
	.open        = rshm_open,
	.read        = rshm_read,
	.write       = rshm_write,
	.lseek       = rshm_lseek,
	.close       = rshm_close
};
#endif
