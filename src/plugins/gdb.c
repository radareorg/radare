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

#include "../plugin.h"
#include "../socket.h"
//#include "../dbg/debug.h"
#include "gdb.h"

//struct debug_t ps;
struct gdbps gdbps;

/* mod255 algorithm */
unsigned char gdb_hash(char *str)
{
	unsigned char digest;
	int i;
	digest = 0;
	for (i = 0; str[i]; i++)
		digest += str[i];
	return digest;
}

char hexstring[] = "0123456789abcdef";

int gdb_send(char *cmd)
{
	unsigned char digest;
	unsigned char buf[1024];
	int len;

	buf[0]='$';
	strcpy((char*) (buf+1), cmd);
	digest = gdb_hash((char*) (buf+1));
	len=strlen((char*)buf);
	buf[len]='#';
	buf[len+1]=hexstring[((digest>>4)&0xf)];
	buf[len+2]=hexstring[(digest&0xf)];
	buf[len+3]='\n';

	write(gdbps.fd, buf, len+3);
	
	return 0;
}

int hex_to_int(char ch)
{
	return strchr(hexstring, ch)-hexstring;
}

int gdb_read_bytes(char *buf, int bytes)
{
	char tmp[8192];
	unsigned char byte;
	int i=0,n=0;
	int ret;

	i = 0;
	n = 0;
	
	while(1) {
		ret = read(gdbps.fd, tmp+i, 1);
		write(1,tmp+i,1);
		
		switch(tmp[i]) {
			case '$':
			case '+':
				write(1,"mark",4);
				continue;
			case '#':
				write(1,"hash",4);
				ret = read(gdbps.fd, tmp+i,2);
				return n;
			default:
				if (n>=bytes)
					return bytes;
				write(1,"byte++\n",7);
				buf[n++] = tmp[i++]; // XXX
				break;
		}
	}

	return n;

	if (tmp[0]!='+') {
		ret = read(gdbps.fd, tmp,1);
		// should be '$'
		if (tmp[0]!='$')
			printf("invalid byets\n");
		printf("xx\n");
	}

	while(1) {
		printf("[1] byte= 0x%02x (%c)\n", tmp[i], tmp[i]);
		
		if (tmp[i]=='#') {
			/* skip checksum */
			printf("ridin\n");
			ret = read(gdbps.fd, tmp+i+1,1);
			printf("[2] cksum= 0x%02x\n", tmp[i+1]);
			ret = read(gdbps.fd, tmp+i+2,1);
			printf("[2] cksum= 0x%02x\n", tmp[i+2]);
			/* skip dollar */
			//ret = read(gdbps.fd, tmp+i+2,1);
			return bytes; // return bytes;
		}
		
		ret = read(gdbps.fd, tmp+i,1);
		printf("pass\n");
		
		if (i==1 && tmp[1]=='E') {
			printf("error\n");
			read(gdbps.fd, tmp, 5);
			return bytes;
		}
		
		if (ret==-1)
			return -1;
			
		if (i==0 && tmp[i]=='$')
			continue;
		
		printf("ee\n");
		if (tmp[i]=='#') {
			/* skip checksum */
			printf("ridin\n");
			ret = read(gdbps.fd, tmp+i+1,1);
			printf("[2] cksum= 0x%02x\n", tmp[i+1]);
			ret = read(gdbps.fd, tmp+i+2,1);
			printf("[2] cksum= 0x%02x\n", tmp[i+2]);
			/* skip dollar */
			//ret = read(gdbps.fd, tmp+i+2,1);
			continue; // return bytes;
		}
		
		if (ret==1)
			i++;
			
		if (i>bytes)
			return i;
		
		if (i>1) {
			printf("iiee\n");
			ret = read(gdbps.fd, tmp+i,1);
			if (tmp[i-1]=='*') {
				int j, end = tmp[i]-29;
				byte = buf[i-2];
				
				for(j=0;j<end;j++)
					buf[i+j] = byte;
				
				i+=end;
			} else {
				//printf("-> read 0x%02x (%c)\n", tmp[i-1], tmp[i-1]);
				//printf("-> read 0x%02x (%c)\n", tmp[i], tmp[i]);
				byte=((hex_to_int(tmp[i-1])<<4) | hex_to_int(tmp[i]));
				buf[i-1] = byte;
			}
			
			//printf("[4] BYTE++ =  0x%02x\n",byte);
		}
	}

	printf("RETURN READ %d\n", i);
	return i;
}

ssize_t gdb_write(int fd, const void *buf, size_t count)
{
#if 0
	if (gdbps.opened)
		if (gdbps.fd == fd)
			return ug_write(gdbps.pid, (long *)buf, count);
#endif
	return count;
}

ssize_t gdb_read(int fd, void *buf, size_t count)
{
	if (gdbps.opened && gdbps.fd == fd) {
		int ret;
		char buf[1024];
		sprintf(buf, "m%x,%d", (unsigned int)gdbps.offset, (int) count);
		gdb_send(buf);
		memset(buf,'\0',count);
		ret = gdb_read_bytes(buf, count);
if (ret!=-1)
write(1,buf,ret);
printf("RET: %d\n", ret);
		return ret;
	}

        return -1;
}

int gdb_handle_fd(int fd)
{
	return (gdbps.opened && gdbps.fd == fd);
}

int gdb_open(char *file, int mode, int flags)
{
	char *port;
	if (!gdb_handle_open(file)) {
		fprintf(stderr, "Invalid uri\n");
		return -1;
	}
	file = file + 6;
	port = strchr(file,':');
	if (port) {
		port[0]='\0';
		gdbps.fd = socket_connect(file, atoi(port + 1));
		if (gdbps.fd == -1)
			fprintf(stderr, "Cannot connect\n");
		else
			gdbps.opened = 1;
		return gdbps.fd;
	} else {
		fprintf(stderr, "No port specified.\n ATM only gdbserver connection is supported\n");
		return -1;
	}
}

int gdb_handle_open(const char *file)
{
	if (!strncmp("gdb://", file, 6))
		return 1;
	return 0;
}

int gdb_seek(int fd, int offset, int whence)
{
	if (gdbps.opened && gdbps.fd == fd)
		switch(whence) {
		case SEEK_SET:
			if (offset!=0)
				gdbps.offset = offset;
			return gdbps.offset;
		case SEEK_CUR:
			gdbps.offset = (off_t)((unsigned long long)gdbps.offset+(unsigned long long)offset);
			return gdbps.offset;
		case SEEK_END:
			return gdbps.offset = (off_t)((unsigned long long)(-1));
		default:
			return (off_t)(unsigned long long)-1;
		}

	return -1;
}

void gdb_system(char *str)
{
	if (!strstr(str, "regs")
	&&  !strstr(str, "info")
	&&  !strstr(str, "maps"))
		gdb_send(str);
}

void gdb_init()
{
}

void gdb_close()
{
	close(config.fd);
}

plugin_t gdb_plugin = {
	.name = "gdb",
	.desc = "Debugs/attach with gdb (gdb://file, gdb://PID, gdb://host:port)",
	.init = gdb_init,
	.debug = NULL,
	.system = gdb_system,
	.handle_fd = gdb_handle_fd,
	.handle_open = gdb_handle_open,
	.open = gdb_open,
	.read = gdb_read,
	.write = gdb_write,
	.lseek = gdb_seek,
	.close = gdb_close
};
