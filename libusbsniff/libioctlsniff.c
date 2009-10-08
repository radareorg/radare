/*
 * Copyright (C) 2009
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
/*
 * LIBUSBSNF : Sniffer wrapper library for libusb
 * TODO: also dump the return value of the functions
 * support for replaying streams
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
//#include <stropts.h>
#include <stdlib.h>
#include "hexdump.h"
#include <unistd.h>

/* symbol descriptors */
int (*__ioctl)(int fd, int req, void *ptr);

/*
 * Library initialization
 */
static void _libwrap_init() __attribute__ ((constructor));
static void _libwrap_init()
{   
  __ioctl = dlsym(RTLD_NEXT, "ioctl");
}

static void _libwrap_fini() __attribute__ ((destructor));
static void _libwrap_fini()
{
	/* do something here ? */
}

//////////////////////////////////////////////////////////
/// this comes from linux/cdrom.h ////////////////////////
//////////////////////////////////////////////////////////

#define CDROM_SEND_PACKET       0x5393  /* send a packet to the drive */
#define CDROM_PACKET_SIZE       12

/* DATA DIRECTION */
#define CGC_DATA_WRITE          1
#define CGC_DATA_READ           2


/* for CDROM_PACKET_COMMAND ioctl */
struct cdrom_generic_command
{
        unsigned char           cmd[CDROM_PACKET_SIZE];
        unsigned char           *buffer;
        unsigned int            buflen;
        int                     stat;
        struct request_sense    *sense;
        unsigned char           data_direction;
        int                     quiet;
        int                     timeout;
        void                    *reserved[1];   /* unused, actually */
};

int ioctl(int fd, int req, void *ptr)
{
	struct cdrom_generic_command *cgc = ptr;
	int i, ret;

	if (req == CDROM_SEND_PACKET) {
		fprintf(stderr, "CDROM_SEND_PACKET:\nCMD: ");
		for(i=0;i<12;i++)
			fprintf(stderr, "%02x ", cgc->cmd[i]);
		fprintf(stderr, "\nDIR: %d\nLEN: %d\nBUF: ",
			cgc->data_direction, cgc->buflen);
		for(i=0;i<cgc->buflen;i++)
			fprintf(stderr, "%02x ", cgc->buffer[i]);
		fprintf(stderr, "\n");
	} else fprintf(stderr, " UNKNOWN TYPE ==> fd=%d, req=%d ptr=%p\n", fd, req, ptr);

	/* do real ioctl */
	ret = __ioctl(fd, req, ptr);

	/* show end packet */
	if (req == CDROM_SEND_PACKET) {
		fprintf(stderr, "BUF: ", cgc->buflen);
		for(i=0;i<cgc->buflen;i++)
			fprintf(stderr, "%02x ", cgc->buffer[i]);
		fprintf(stderr, "\n");
	}

	return ret;
}
