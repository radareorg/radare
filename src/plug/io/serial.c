/*
 * Copyright (C) 2008, 2009
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
// serial:///dev/ttyS0:9600
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
static int serial_fd = -1;
static unsigned char *serial_buf = NULL;
static unsigned int serial_bufsz = 0;
static unsigned int serial_bufread = 0;

ssize_t serial_write(int fd, const void *buf, size_t count)
{
        return write(fd, (u8 *)buf, count);
}

ssize_t serial_read(int fd, void *buf, size_t count)
{
	u8 data[32000];
	int sz;
	u64 s;

	if (config.seek > serial_bufsz)
		config.seek = serial_bufsz;

	if (fd == serial_fd) {
		if (socket_ready(fd, 0, 10)>0) {
			sz = read(fd, data, 32000);
			if (sz == -1) {
				eprintf("Connection closed\n");
				// XXX close foo
			}
			if (sz>0) {
				if (serial_buf)
					serial_buf = (u8 *)realloc(serial_buf, serial_bufsz+sz);
				else 	serial_buf = (u8 *)malloc(serial_bufsz+sz);
				if (lock)
					memcpy(serial_buf, data, sz);
				else
					memcpy(serial_buf+(int)serial_bufsz, data, sz);
				sprintf((char *)data, "_read_%d", serial_bufread++);
				flag_set((char *)data, serial_bufsz, 0);
				flag_set("_read_last", serial_bufsz, 0);
				serial_bufsz += sz;
			}
		}
		if (config.seek < serial_bufsz) {
			s = count;
			if (count+config.seek > serial_bufsz)
				s = serial_bufsz-config.seek;
			memcpy(buf, serial_buf+config.seek, s);
			return s;
		}
	}
        return 0;
}

int serial_close(int fd)
{
	return close(fd);
}

u64 serial_lseek(int fildes, u64 offset, int whence)
{
	switch(whence) {
	case SEEK_SET:
		return offset;
	case SEEK_CUR:
		if (config.seek+offset>serial_bufsz)
			return serial_bufsz;
		return config.seek+offset;
	case SEEK_END:
		return 0xffffffff;
	}
	return 0;
}

static int serial_handle_fd(int fd)
{
	return (fd == serial_fd);
}

static int serial_handle_open(const char *pathname)
{
	return (!memcmp(pathname, "serial://", 9));
}

static int opendev(const char *device, int speed)
{

        struct termios tc;     // 115200 baud, 8n1, no flow control
        int fd = open (device, O_RDWR | O_NOCTTY);
        if (fd == -1)
		return -1;

#if 0
        // Get serial device parameters 
        tcgetattr(fd, &tc);

        tc.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
                              | INLCR | IGNCR | ICRNL | IXON );
        tc.c_oflag &= ~OPOST;
        tc.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
        tc.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
        tc.c_cflag |= CS8 | CREAD | CLOCAL ;
        tc.c_cc[VMIN] = 1;
        tc.c_cc[VTIME] = 3;
#endif
	memset(&tc, 0, sizeof(tc));
	tc.c_cflag = CS8 | CLOCAL | CREAD;
	tc.c_iflag = IGNPAR; /* parity */
	tc.c_oflag = 0;
	tc.c_lflag = 0;
        tc.c_cc[VMIN] = 1;
        tc.c_cc[VTIME] = 0;

        /* Set port speed */
	switch(speed) {
	case 0:
		tc.c_cflag |= B0;
		break;
	case 1200:
		tc.c_cflag |= B1200;
		break;
	case 4800:
		tc.c_cflag |= B4800;
		break;
	case 9600:
		tc.c_cflag |= B9600;
		break;
	case 19200:
		tc.c_cflag |= B19200;
		break;
	case 38400:
		tc.c_cflag |= B38400;
		break;
	default:
		tc.c_cflag |= B19200;
		break;
	}

        // Set TCSANOW mode of serial device 
        tcsetattr(fd, TCSANOW, &tc);
	tcflush(fd, TCIOFLUSH);
	return fd;
}

int serial_open(const char *pathname, int flags, mode_t mode)
{
	char buf[1024];
	int speed = 9600;
	char *ptr = buf;

	strncpy(buf, pathname, 1000);

	if (!memcmp(ptr , "serial://", 9)) {
		ptr= ptr+9;
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
		
		serial_fd = opendev(ptr, speed);
		if (serial_fd>=0)
			printf("Serial connection to %s (speed=%d) done\n", ptr, speed);
		else	printf("Cannot open serial '%s'\n", ptr);
		serial_buf = (unsigned char *)malloc(1);
		serial_bufsz = 0;
		config_set("file.write", "true"); // ???
		buf[0]='\0';
	}
	return serial_fd;
}

int serial_show_log()
{
	eprintf("TODO\n");
}

int serial_system(const char *cmd)
{
	if (!strcmp(cmd, "lock")) {
		lock = 1;
	} else
	if (!strcmp(cmd, "unlock")) {
		lock = 0;
	} else
	if (!strcmp(cmd, "log")) {
		serial_show_log();
	} else
	if (!strcmp(cmd, "help")) {
		printf("serial:// IO plugin help\n");
		printf(" !help    show this help\n");
		printf(" !lock    lock internal read pointer\n");
		printf(" !unlock  unlock internal read pointer\n");
		printf(" !log     show io conversation\n");
	}
}

plugin_t serial_plugin = {
	.name        = "serial",
	.desc        = "serial port access ( serial://path/to/dev:speed )",
	.init        = NULL,
	.debug       = NULL,
	.system      = serial_system,
	.handle_fd   = serial_handle_fd,
	.handle_open = serial_handle_open,
	.open        = serial_open,
	.read        = serial_read,
	.write       = serial_write,
	.lseek       = serial_lseek,
	.close       = serial_close
};

#endif
