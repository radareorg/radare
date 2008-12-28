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
// TODO: add bitrate and hw/sw control flow and so
// trk:///dev/ttyS0:9600
// fixes on !lock !unlock and !log
// add !info


#include <main.h>
#include <plugin.h>
#include "../../socket.h"

#if __UNIX__
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

static int lock = 0;
static int trk_fd = -1;
static unsigned char *trk_buf = NULL;
static unsigned int trk_bufsz = 0;
static unsigned int trk_bufread = 0;

int trk_read_raw(int fd, u8 *b, int len);
ssize_t trk_write_raw(int fd, u8 *b, size_t count);

#if 0
/* handshake */
trk_write(trk_fd, "\x7e\x00\x00\xff\x7e", 5);
read(trk_fd, buf, 6); // < 7E 80 00 00 7F 7E
trk_write(trk_fd, "\x7e\x00\x00\xfd\x7e", 5);
read(trk_fd, buf, 7); // < 7E 80 01 00 7D 5E 7E

/* read memory */
//   > 7E 10 03 05 01 00 64_04_D1_E0 00 00 00 00 00 00 00 01 CC 7E

#endif

int trk_read(int fd, u64 addr, u8 *buf, int len)
{
	u8 reply[4096];
	u32 addr32 = (u32)addr;
	u32 len32 = (u32)len;
	u8 cmd[1024];

	cmd[0]=0x7e;
	cmd[1]=0x10;
	cmd[2]=0x03; // ____
	cmd[3]=0x05;
	cmd[4]=0x01;
	cmd[5]=0x00;
	endian_memcpy_e(cmd+5, &addr32, 4);
	cmd[9]=0x00;
	cmd[10]=0x00;
	cmd[11]=0x00;
	cmd[12]=0x00;
	endian_memcpy_e(cmd+12, &addr32, 4);
	cmd[16]=0xcc;
	cmd[17]=0x7e;
	trk_write_raw(fd, cmd, 17);

	return trk_read_raw(fd, buf, len);
}

int trk_read_raw(int fd, u8 *b, int len)
{
	u8 tmp[64];
	// READ ERROR: < 7e ff 04 05 f7 7e
	// READ MEM  7e 80 03   20 5c 7e 7e   80 03 20   5c   7e
	read(fd, b, 2);
	if (b[0] != 0x7e) {
		eprintf("protocol error: no header mark found\n");
		// TODO: call to skip until header
		return 0;
	}
	if (b[1] == 0x80){
		/* read ok */
		read(fd, b, 4); // skip 4 unknown bytes
		read(fd, b, len);
		read(fd, tmp, 2); // read checksum and footer
		
	} else {
		//if (b[1]==0xff) {
		eprintf("trk error received %02x %02x %02x\n",
			b[2],b[3],b[4]);
		return 1;
	}
	return 0;
}

ssize_t trk_write_raw(int fd, u8 *b, size_t count)
{
	char *buf = alloca(count*2);
	memcpy(buf, b, count);
	for(i=0;i<count;i++) {
		if (buf[i]==0x7e) {
			memcpy_r(buf+i, buf+i+1, count-i-1);
			buf[i] = 0x7d;
			buf[i+1] = 0x5e;
		} else
		if (buf[i]==0x7d) {
			memcpy_r(buf+i, buf+i+1, count-i-1);
			buf[i] = 0x7d;
			buf[i+1] = 0x5d;
		}
	}
        return write(fd, (u8 *)buf, count);
}

ssize_t trk_write_at(int fd, u32 addr, const void *buf, u32 count)
{
	u8 b[4096];
	/* TODO */
	b[0]=0x7e; // head
	b[1]=0x11; // write
	b[2]=0x04; // 
	b[3]=0x08; // 
	b[4]=0x00; // 
	b[5]=0x04; // 
	memcpy(b+5, &addr, 4);
	memset(b+5+4, '\0', 4);
	memcpy(b+5+4+4, &count, 4);
        b[5+4+4+4] = 0x00; // checksum
	b[5+4+4+4+1] = 0x7e;
   // > 7E 11 04 08 00 04 64_00_01_48 00_00_00_00 00_00_00_01 10 ...
	trk_write_raw(fd, b, count);
}

ssize_t trk_read(int fd, void *buf, size_t count)
{
	u8 data[32000];
	int sz;
	u64 s;

	if (config.seek > trk_bufsz)
		config.seek = trk_bufsz;

	if (fd == trk_fd) {
		if (socket_ready(fd, 0, 10)>0) {
			sz = read(fd, data, 32000);
			if (sz == -1) {
				eprintf("Connection closed\n");
				// XXX close foo
			}
			if (sz>0) {
				if (trk_buf)
					trk_buf = (u8 *)realloc(trk_buf, trk_bufsz+sz);
				else 	trk_buf = (u8 *)malloc(trk_bufsz+sz);
				if (lock)
					memcpy(trk_buf, data, sz);
				else
					memcpy(trk_buf+(int)trk_bufsz, data, sz);
				sprintf((char *)data, "_read_%d", trk_bufread++);
				flag_set((char *)data, trk_bufsz, 0);
				flag_set("_read_last", trk_bufsz, 0);
				trk_bufsz += sz;
			}
		}
		if (config.seek < trk_bufsz) {
			s = count;
			if (count+config.seek > trk_bufsz)
				s = trk_bufsz-config.seek;
			memcpy(buf, trk_buf+config.seek, s);
			return s;
		}
	}
        return 0;
}

int trk_close(int fd)
{
	return close(fd);
}

u64 trk_lseek(int fildes, u64 offset, int whence)
{
	switch(whence) {
	case SEEK_SET:
		return offset;
	case SEEK_CUR:
		if (config.seek+offset>trk_bufsz)
			return trk_bufsz;
		return config.seek+offset;
	case SEEK_END:
		return 0xffffffff;
	}
	return 0;
}

static int trk_handle_fd(int fd)
{
	return (fd == trk_fd);
}

static int trk_handle_open(const char *pathname)
{
	return (!memcmp(pathname, "trk://", 6));
}

static int opendev(const char *device, int speed)
{

        struct termios tc;     // 115200 baud, 8n1, no flow control
        int fd = open (device, O_RDWR | O_NOCTTY);
        if (fd == -1)
		return -1;

        // Get trk device parameters 
        tcgetattr(fd, &tc);

        tc.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
		      | INLCR | IGNCR | ICRNL | IXON );
        tc.c_oflag &= ~OPOST;
        tc.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
        tc.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
        tc.c_cflag |= CS8 | CREAD | CLOCAL ;
        tc.c_cc[VMIN] = 1;
        tc.c_cc[VTIME] = 3;

        // Set port speed to 9600 baud 
	switch(speed) {
	default:
		cfsetospeed(&tc, B19200);
		cfsetispeed (&tc, B0);
	}

        // Set TCSANOW mode of trk device 
        tcsetattr(fd, TCSANOW, &tc);
	return fd;
}

int trk_open(const char *pathname, int flags, mode_t mode)
{
	char buf[1024];
	int speed = 9600;
	char *ptr = buf;

	strncpy(buf, pathname, 1000);

	if (!memcmp(ptr , "trk://", 6)) {
		ptr= ptr+6;
		// port
		char *spd = strchr(ptr, ':');
		if (spd == NULL) {
			printf("No speed defined.\n");
			return -1;
		}
		speed = atoi(spd+1);
		spd[0] = '\0';
		if (strlen(ptr)==0)
			return -1;
		
		trk_fd = opendev(ptr, speed);
		if (trk_fd>=0)
			printf("Serial connection to %s (speed=%d) done\n", ptr, speed);
		else	printf("Cannot open trk '%s'\n", ptr);
		trk_buf = (unsigned char *)malloc(1);
		trk_bufsz = 0;
		config_set("file.write", "true"); // ???
		buf[0]='\0';
	}
	return trk_fd;
}

int trk_show_log()
{
	eprintf("TODO\n");
}

int trk_system(const char *cmd)
{
	if (!strcmp(cmd, "lock")) {
		lock = 1;
	} else
	if (!strcmp(cmd, "unlock")) {
		lock = 0;
	} else
	if (!strcmp(cmd, "log")) {
		trk_show_log();
	} else
	if (!strcmp(cmd, "help")) {
		printf("trk:// IO plugin help\n");
		printf(" !help    show this help\n");
		printf(" !lock    lock internal read pointer\n");
		printf(" !unlock  unlock internal read pointer\n");
		printf(" !log     show io conversation\n");
	}
}

plugin_t trk_plugin = {
	.name        = "trk",
	.desc        = "trk debugger access ( trk://path/to/dev:speed )",
	.init        = NULL,
	.debug       = NULL,
	.system      = trk_system,
	.handle_fd   = trk_handle_fd,
	.handle_open = trk_handle_open,
	.open        = trk_open,
	.read        = trk_read,
	.write       = trk_write,
	.lseek       = trk_lseek,
	.close       = trk_close
};

#endif
