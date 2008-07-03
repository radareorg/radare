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
#include <sys/mman.h>

static int mmap_fd = -1;
static unsigned char *mmap_buf = NULL;
static unsigned int mmap_bufsz = 32*1024*1024; /* 32MB */
static unsigned int mmap_bufread = 0;

static ssize_t mmap_write(int fd, const void *buf, size_t count)
{
	int ret;
	if (mmap_buf != NULL) {
		mmap_buf = mmap(NULL, 4096,  PROT_WRITE, MAP_SHARED, fd, (off_t)config.seek-config.seek%4096);
		if (((int)mmap_buf)==-1) {
			perror("mmap");
			return -1;
		}
		memcpy(mmap_buf+config.seek%4096, buf, count);
		munmap(mmap_buf, count*2);
		return ret;
	}
	return -1;
}

static ssize_t mmap_read(int fd, void *buf, size_t count)
{
	u8 data[32000];
	int sz;
	u64 s;

	if (config.seek + count > config.size) {
		//config.seek = 0; // ugly hack
		//count = config.seek+count-config.size;
		return -1;
	}
	if (config.seek +count > config.size)
		config.seek = config.size;

	mmap_buf = mmap(NULL, count*2,  PROT_READ, MAP_SHARED, fd, (off_t)config.seek-config.seek%4096);
	if (((int)mmap_buf)==-1) {
		perror("mmap");
		return -1;
	}
	memcpy(buf, mmap_buf+config.seek%4096, count);
	munmap(mmap_buf, count*2);
        return count;
}

static int mmap_close(int fd)
{
	if (mmap_buf == NULL)
		return -1;
	return close(mmap_fd);
}

extern u64 posix_lseek(int fildes, u64 offset, int whence);
static u64 mmap_lseek(int fildes, u64 offset, int whence)
{
	return posix_lseek(fildes,offset,whence);
}

static int mmap_handle_fd(int fd)
{
	return (fd == mmap_fd);
}

static int mmap_handle_open(const char *pathname)
{
	return (!memcmp(pathname, "mmap://", 7));
}

static int mmap_open(const char *pathname, int flags, mode_t mode)
{
	char buf[1024];
	char *ptr = buf;

	strncpy(buf, pathname, 1000);

	if (!memcmp(ptr , "mmap://", 7)) {
		ptr= ptr+6;
		// connect
		mmap_fd = open(ptr,flags,mode);

		if (((int)(mmap_fd)) == -1) {
			printf("Cannot open mmap file (%s)\n", ptr);
			mmap_buf = NULL;
			mmap_fd = -1;
		}
	}
	return mmap_fd;
}

plugin_t mmap_plugin = {
	.name        = "mmap",
	.desc        = "memory mapped device ( mmap://file )",
	.init        = NULL,
	.debug       = NULL,
	.system      = NULL,
	.handle_fd   = mmap_handle_fd,
	.handle_open = mmap_handle_open,
	.open        = mmap_open,
	.read        = mmap_read,
	.write       = mmap_write,
	.lseek       = mmap_lseek,
	.close       = mmap_close
};
#endif
