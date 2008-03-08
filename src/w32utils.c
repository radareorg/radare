/*
 * Copyright (C) 2008
 *       pancake <youterm.com>
 *
 * + 2006-05-12 Lluis Vilanova xscript <gmx.net>
 * 	Code refactorization and unbounded search
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

#include "main.h"
#ifdef __WINDOWS__

#include <sys/types.h>
#include <windows.h>
#include <winsock.h>

static char buf[1024];

char *getenv(const char *str)
{
	GetEnvironmentVariable(str, &buf, 1023);
	return &buf;
}

int unsetenv(const char *foo)
{
	SetEnvironmentVariable(foo, NULL);
	return 0;
}

int setenv(const char *foo, const char *bar)
{
	SetEnvironmentVariable(foo, bar);
	return 0;
}

int fork()
{
	fprintf(stderr, "fork: not available for w32\n");
}

int sleep(int s)
{
	Sleep(s*1000);
}

int usleep(int s)
{
	Sleep(s);
}

int mkstemp(char *path)
{
	int fd;
	strcpy(path, "foo.tmp");
	fd = open(path, O_RDONLY|O_CREAT, 0666);
	return fd;
}

WINSOCK_API_LINKAGE u_short PASCAL ntohs(u_short foo)
{
	// swap endian here !!
	return 0;
}

WINSOCK_API_LINKAGE u_short PASCAL htons(u_short foo)
{
	// swap endian here !!
	return 0;
}

WINSOCK_API_LINKAGE u_long PASCAL ntohl(u_long foo)
{
	// swap endian here !!
	return 0;
}

WINSOCK_API_LINKAGE u_long PASCAL htonl(u_long foo)
{
	// swap endian here !!
	return 0;
}

WINSOCK_API_LINKAGE SOCKET PASCAL accept(SOCKET fd,struct sockaddr* sa,int* le)
{
	return -1;
}

#endif
