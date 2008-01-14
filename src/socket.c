/*
 * Copyright (C) 2006-2007
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

#include "main.h"
#include "socket.h"
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 4096

void socket_printf(int fd, const char *fmt, ...)
{
#if !__linux__
	char buf[BUFFER_SIZE];
#endif
	va_list ap;
	va_start(ap, fmt);

	if (fd <= 0)
		return;
#if __linux__
	dprintf(fd, fmt, ap); 
#else
	snprintf(buf,BUFFER_SIZE,fmt,ap); 
	write(fd, buf, strlen(buf));
#endif
	va_end(ap);
}

int socket_connect(char *host, int port)
{
	struct sockaddr_in sa;
	struct hostent *he;
	int s;

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == -1)
		return -1;

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	he = (struct hostent *)gethostbyname( host );
	if (he == (struct hostent*)0)
		return -1;

	sa.sin_addr = *((struct in_addr *)he->h_addr);
	sa.sin_port = htons( port );

	if (connect(s, (const struct sockaddr*)&sa, (socklen_t)sizeof(struct sockaddr)))
		return -1;

	return s;
}

int socket_listen(int port)
{
	int s;
	int ret;
	struct sockaddr_in sa;
	struct linger linger = { 0 };
	linger.l_onoff = 1;
	linger.l_linger = 1;

 	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s <0)
		return -1;

	setsockopt(s, 
		SOL_SOCKET, SO_LINGER, (const char *) &linger, sizeof(linger));

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons( port );

	ret = bind(s, (struct sockaddr *)&sa, sizeof(sa));
	if (ret < 0)
		return -1;

	ret = listen(s, 1);
	if (ret < 0)
		return -1;
	
	return s;
}

int socket_fgets(char *buf,  int size)
{
	int i = 0;
	int ret = 0;

	if (config.fd == -1) {
		close(config.fd);
		D printf("byebye\n");
		exit(0);
	}

	signal(SIGPIPE, SIG_IGN);

	while(i<size) {
		ret = read(config.fd, buf+i, 1);
		if (ret<=0 ) {
			shutdown(config.fd, SHUT_RDWR);
			close(config.fd);
			config.fd = -1;
		}
		/*if (buf[i]==4) { // ^D
			shutdown(config.fd,SHUT_RDWR);
			close(config.fd);
			config.fd = -1;
			break;
		}
		*/
		if (buf[i]=='\r'||buf[i]=='\n')
			break;
		i+=ret;
	}
	buf[i]='\0'; // XXX 1byte overflow

	return ret;
}

