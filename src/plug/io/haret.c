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

#include <radare.h>
#include <plugin.h>
#include <socket.h>

static int haret_fd = -1;

int haret_handle_fd(int fd)
{
	return fd == haret_fd;
}

int haret_handle_open(const char *file)
{
	if (!memcmp(file, "haret://", 8))
		return 1;
	return 0;
}

ssize_t haret_write(int fd, const void *buf, size_t count)
{
	// TODO: not yet implemented (pfd command ?)
        return 0;
}


static void haret_wait_until_prompt()
{
	unsigned char buf;
	int off = 0;

	while(1) {
		read(config.fd, &buf, 1);
		switch(off) {
		case 0: if (buf == ')') off =1; break;
		case 1: if (buf == '#') return; else off = 0; break;
		}
	}
}

ssize_t haret_read(int fd, unsigned char *buf, size_t count)
{
	char tmp[1024];
	unsigned char *pbuf = buf;
	unsigned char *ptr;
	int i,j=0,k;
	int size = count;
	int delta = config.seek%4;

	if (size%16)
		size+=(size-(size%16));

	sprintf(tmp,"pdump 0x"OFF_FMTx" %d\r", config.seek-delta, size);
	//printf("SEND(%s);\n", tmp);
	socket_printf(config.fd, tmp);
	//printf("OFFSET: %lld\n", (unsigned long long)config.seek);
	//printf("LEN: %lld\n", (unsigned long long)count);

	// skip echo
	socket_fgets(fd, tmp, 1024);
	socket_fgets(fd, tmp, 1024);
	for(k=j=i=0;i<100;i++) {
		socket_fgets(fd, tmp, 1024);
		ptr = (unsigned char *)tmp+11;
		ptr[36]='\0';
		if (j+16>=size) break;
		//printf(" READ ==> %s\n", ptr);
		hexstr2binstr((char *)ptr, ptr);
		memcpy(buf+j, ptr, 16);
		socket_fgets(fd, tmp, 1024);
		for(k=0;k<16;k+=4) {
			unsigned char ch = pbuf[k+j];
			pbuf[k+j] = pbuf[k+j+3];
			pbuf[i+j+3] = ch;
			ch = pbuf[k+j+1];
			pbuf[k+j+1] = pbuf[k+j+2];
			pbuf[k+j+2] = ch;
		}
		j+=16;
	}
	haret_wait_until_prompt();

	// shift simulator
	if (delta)
		memcpy(buf, buf+delta, count-delta);

        return count;
}
int haret_open(const char *pathname, int flags, mode_t mode)
{
	int fd;
	char host[128];
	char tmp[4096];
	char *port;
	int i;
	int iport = 9999;

	strncpy(host, pathname+8, 127); host[127]='\0';
	port = strchr(host, ':');
	if (port) {
		port[0]='\0';
		iport = atoi(port+1);
	}
	
	fd = socket_connect(host, iport);
	if (fd != -1) {
		config.fd = fd;
		for(i=0;i<10;i++) { // 12
			socket_fgets(fd, tmp, 100);
			tmp[strlen(tmp)-1]='\0';
			printf(";-- %s\n", tmp);
		}
		haret_fd = fd;
		haret_wait_until_prompt();
	} else
		printf("Cannot connect to remote host.\n");

	return fd;
}

int haret_close(int fd)
{
	return close(fd);
}

u64 haret_lseek(int fildes, u64 offset, int whence)
{
	switch(whence) {
	case SEEK_SET:
		return offset;
	case SEEK_CUR:
		return config.seek+offset;
	case SEEK_END:
		return 0xffffffff;
	}
	return offset;
}

plugin_t haret_plugin = {
	.name = "haret",
	.desc = "Read WCE memory ( haret://host:port )",
	.init = NULL,
	.system = NULL,
	.debug = NULL,
	.handle_fd = haret_handle_fd,
	.handle_open = haret_handle_open,
	.open = haret_open,
	.read = haret_read,
	.write = haret_write,
	.lseek = haret_lseek,
	.close = haret_close
};
