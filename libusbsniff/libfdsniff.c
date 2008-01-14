/*
 * Copyright (C) 2007
 *       pancake <pancake@youterm.com>
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
/*
 * LIBFDSNF : Sniffer wrapper library for open/close/read/write
 * TODO: implement ioctl() -- need to handle varargs
 *
 * @author: pancake <pancake@youterm.com>
 */

#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "hexdump.h"

/* symbol descriptors */
int (*__open)(const char *pathname, int flags);
int (*__close)(int fd);
ssize_t (*__read)(int fd, void *buf, size_t count);
ssize_t (*__write)(int fd, const void *buf, size_t count);
size_t (*__fread)(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t (*__fwrite)(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int __fdsniff;

/*
 * Library initialization
 */
static void _libwrap_init() __attribute__ ((constructor));
static void _libwrap_init()
{   
  char *fdsniff = getenv("FDSNIFF");

  if (fdsniff == NULL) {
  	fprintf(stderr, "FDSNIFF environment not defined.\n");
  	exit(1);
  }

  __open    = dlsym(RTLD_NEXT, "open");
  __close   = dlsym(RTLD_NEXT, "close");
  __read    = dlsym(RTLD_NEXT, "read");
  __write   = dlsym(RTLD_NEXT, "write");
  __fread   = dlsym(RTLD_NEXT, "fread");
  __fwrite  = dlsym(RTLD_NEXT, "fwrite");
  __fdsniff = atoi(fdsniff);
  fprintf(stderr, "FDSNF initialized hooking filedescriptor %d.\n", __fdsniff);
}

static void _libwrap_fini() __attribute__ ((destructor));
static void _libwrap_fini()
{
	/* do something here */
}

// TODO: handle varargs
int open(const char *pathname, int flags, ...)
{
	int ret = __open(pathname, flags);
	if (ret == __fdsniff) {
		fprintf(stderr, "FDSNF: open(\"%s\", %d);\n", pathname, flags);
		fprintf(stderr, "FDSNF:   return       = %x\n", ret);
	}
	return ret;
}

int close(int fd)
{
	int ret = __close(fd);
	if (fd == __fdsniff) {
		fprintf(stderr, "FDSNF: close()\n");
		fprintf(stderr, "FDSNF:   handle       = %x\n", fd);
		fprintf(stderr, "FDSNF:   return       = %d\n", ret);
	}
	return ret;
}

ssize_t write(int fd, const void *buf, size_t count)
{
	ssize_t ret = __write(fd, buf, count);

	if (fd == __fdsniff) {
		fprintf(stderr, "FDSNF: write()\n");
		fprintf(stderr, "FDSNF:   handler      = %x\n", fd);
		fprintf(stderr, "FDSNF:   return       = %d\n", ret);
		fprintf(stderr, "FDSNF:   size         = %d\n", count);
	//	dump_bytes((unsigned char *)buf, count);
	}

	return ret;
}

ssize_t read(int fd, void *buf, size_t count)
{
	ssize_t ret;

	ret = __read(fd, buf, count);
	if (fd == __fdsniff) {
		fprintf(stderr, "FDSNF: read()\n");
		fprintf(stderr, "FDSNF:   handler      = %x\n", fd);
		fprintf(stderr, "FDSNF:   size         = %d\n", count);
		dump_bytes((unsigned char *)buf, count);
		fprintf(stderr, "FDSNF:   return       = %d\n", ret);
	}
	return ret;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t ret = __fwrite(ptr, size, nmemb, stream);

#if __GLIBC__
	if ( stream->_fileno == __fdsniff) {
		fprintf(stderr, "FDSNF: fread()\n");
		fprintf(stderr, "FDSNF:   handler      = %x\n", __fdsniff);
		fprintf(stderr, "FDSNF:   size         = %d\n", size);
		fprintf(stderr, "FDSNF:   nmemb        = %d\n", nmemb);
		dump_bytes((unsigned char *)ptr, size*nmemb);
		fprintf(stderr, "FDSNF:   return       = %d\n", ret);
	}
#endif
	return ret;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t ret = __fwrite(ptr, size, nmemb, stream);

#if __GLIBC__
	if ( stream->_fileno == __fdsniff) {
		fprintf(stderr, "FDSNF: fwrite()\n");
		fprintf(stderr, "FDSNF:   handler      = %x\n", __fdsniff);
		fprintf(stderr, "FDSNF:   size         = %d\n", size);
		fprintf(stderr, "FDSNF:   nmemb        = %d\n", nmemb);
		dump_bytes((unsigned char *)ptr, size*nmemb);
		fprintf(stderr, "FDSNF:   return       = %d\n", ret);
	}
#endif
	return ret;
}
