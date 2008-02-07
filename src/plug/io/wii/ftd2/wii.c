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

// wii://nand
// wii://vram
// wii://ram


#include "../plugin.h"
#include "../utils.h"
#include "../socket.h"
#include <netinet/in.h>
#include <signal.h>
#include <string.h>

enum {
	REMOTE_NONE=0,
	REMOTE_CLIENT=1,
	REMOTE_LISTEN=2
};
int wii_mode = REMOTE_NONE;

/* ftd2xx code */
#if __CYGWIN__ || __WINDOWS__
/* nothing */
#else
#include "WinTypes.h"
#endif

#include <ftd2xx.h>
//#include <ftd2xx/ftd2xx.h>
//#include <ftd2xx/packet.h>

#define cmd_sendbyte 0x01
#define cmd_receivebyte 0x02
#define cmd_sendpacket 0x03
#define cmd_receivepacket 0x04

static int what = 0;
static FT_HANDLE wii_fd = NULL;  // Handle of the device to be opened and used for all functions
static FT_STATUS status;         // Variable needed for FTDI Library functions
static DWORD TxSent;
static DWORD RxSent;
static DWORD RXqueue;
static DWORD RXbytes,TXbytes,Eventdword;


/* ftd2xx code */
#define RMT_OPEN   0x01
#define RMT_READ   0x02
#define RMT_WRITE  0x03
#define RMT_SEEK   0x04
#define RMT_CLOSE  0x05
#define RMT_SYSTEM 0x06
#define RMT_REPLY  0x80

#define uchar unsigned char

ssize_t wii_write(int fd, const void *buf, size_t count)
{
	unsigned char *tmp;
	unsigned int size = (int)count;
	int ret;

	tmp = (unsigned char *)malloc(count+5);

	tmp[0] = RMT_WRITE;
	endian_memcpy((uchar *)tmp+1, (uchar *)&size, 4);
	memcpy(tmp+5, buf, size);

	ret = gecko_write(wii_fd, tmp, size+5);
	free(tmp);

        return ret;
}

ssize_t wii_read(int fd, void *buf, size_t count)
{
	int i = (int)count;
	uchar tmp[5];

	// send
	tmp[0] = RMT_READ;
	endian_memcpy(tmp+1, (uchar*)&i, 4);
	write(wii_fd, tmp, 5);

	// recv
	gecko_read(wii_fd, tmp, 5);
	if (tmp[0] != (RMT_READ|RMT_REPLY)) {
		printf("Unexpected wii read reply (0x%02x)\n", tmp[0]);
		return -1;
	}
	endian_memcpy((uchar*)&i, tmp+1, 4);
	gecko_read(wii_fd, buf, i);

        return i; 
}

int wii_open(const char *pathname, int flags, mode_t mode)
{
	int i;
	char *file;

	if (strstr(pathname, "ram")) {
		what = 1;
	} else
	if (strstr(pathname, "vram")) {
		what = 2;
	} else
	if (strstr(pathname, "nand")) {
		what = 3;
	}

	// connect
	wii_fd = gecko_opendevice();
	if (wii_fd>0)
		eprintf("Connected to the gecko.\n");
	else
		eprintf("Cannot coonect to the gecko via USB\n");

	/* handshake */
	// send
	buf[0] = RMT_OPEN;
	buf[1] = flags;
	buf[2] = (uchar)strlen(file)-1;
	memcpy(buf+3, file+1, buf[2]);
	gecko_write(wii_fd, buf, 3+buf[2]);

	// read
	eprintf("waiting... ");
	gecko_read(wii_fd, buf, 5);
	if (buf[0] != (char)(RMT_OPEN|RMT_REPLY))
		return -1;

	endian_memcpy((uchar *)&i, (uchar*)buf+1, 4);
	if (i>0) eprintf("ok\n");
	config.fd = wii_fd;

	return wii_fd;
}

int wii_close(int fd)
{
	status = FT_Close(wii_fd);
	return 1;
}

off_t wii_lseek(int fildes, off_t offset, int whence)
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

int wii_handle_fd(int fd)
{
	return (fd == wii_fd);
}

int wii_handle_open(const char *file)
{
	if (!memcmp(file, "wii://", 6))
		return 1;
	return 0;
}

int wii_system(const char *command)
{
	uchar buf[1024];
	char *ptr;
	int i;

	if (command[0] == '!')
		return system(command+1);

	if (config.debug) {
		// send
		buf[0] = RMT_SYSTEM;
		i = strlen(command);
		endian_memcpy(buf+1, (uchar*)&i, 4);
		memcpy(buf+5, command, i);
		gecko_write(wii_fd, buf, i+5);

		// read
		gecko_read(wii_fd, buf, 5);
		if (buf[0] != (RMT_SYSTEM | RMT_REPLY)) {
			printf("Unexpected system reply\n");
			return -1;
		}
		endian_memcpy((uchar*)&i, buf+1, 4);
		if (i == -1) //0xFFFFFFFF) {
			return -1;
		ptr = (char *)malloc(i);
		gecko_read(wii_fd, ptr, i);
		gecko_write(1, ptr, i);
		free(ptr);
		return i;
	}
	return 0;
}

plugin_t wii_plugin = {
	.name        = "wii",
	.desc        = "Wii(R) connection via USB",
	.init        = NULL,
	.debug       = NULL,
	.system      = wii_system,
	.handle_fd   = wii_handle_fd,
	.handle_open = wii_handle_open,
	.open        = wii_open,
	.read        = wii_read,
	.write       = wii_write,
	.lseek       = wii_lseek,
	.close       = wii_close
};
