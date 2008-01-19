/*
 * Copyright (C) 2007, 2008
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

int radare_read_at(off_t offset, unsigned char *data, int len)
{
	off_t cur = config.seek;
	radare_seek(offset,SEEK_SET);
	io_read(config.fd, data, len);
	radare_seek(cur, SEEK_SET);
}

int radare_write_at(off_t offset, unsigned char *data, int len)
{
	off_t cur = config.seek;
	radare_seek(offset,SEEK_SET);
	io_write(config.fd, data, len);
	radare_seek(cur, SEEK_SET);
	radare_read(0);
}

int radare_write(char *arg, int mode)
{
	int fmt = last_print_format;
	off_t oseek = config.seek;
	off_t seek = config.seek;
	int times = config_get_i("cfg.count");
	int i,bytes = 0;
	int len   = 0;
	char *str, *tmp;

	if (times<=0)
		times = 1;

	if (!config_get("cfg.write")) {
		eprintf("Not in write mode. Type 'eval cfg.write = true'.\n");
		return 0;
	}

	str = strdup(arg);
	if (arg[0]=='0' && arg[1]=='x'){
		mode = WMODE_HEX;
		strcpy(str, str+2);
	}

	switch(mode) {
	case WMODE_WSTRING:
		tmp = malloc((len = escape_buffer(str)<<1));
		for(i=0;i<len;i++) {
			if (i%2) tmp[i] = 0;
			else tmp[i] = str[i>>1];
		}
		free(str); str = tmp;
		break;
	case WMODE_STRING:
 		len = escape_buffer(str);
		break;
	case WMODE_HEX:
		len = hexstr2binstr(str, (unsigned char *)str);
		break;
	}
	
	if (len == 0) {
		D eprintf("warning: zero length string.\n");
		free(str);
		return 0;
	}

	arg[len]='\0';

	if (config.cursor_mode) {
		config.ocursor = seek;
		seek += config.cursor;
		config.cursor = seek;
	}

#if 0
	D data_print(seek, (unsigned char *)str, len, FMT_HEXB, MD_ALWAYS);
	if (yesno('n', "Write this %d bytes buffer %d times (aka %d)? (y/N)",
	                (int)len, (int)times, (int)(len*times)))
	{
#endif
		radare_seek(seek, SEEK_SET);

		for(bytes=0;times--;)
			bytes += io_write(config.fd, str, len);

		if (!config.debug)
		if (!config.unksize && seek + len > config.size)
			radare_open(1);

#if 0
		D eprintf("%d bytes written.\n", bytes);
	} else
		D eprintf("nothing changed.\n");
#endif

	radare_seek(oseek, SEEK_SET);
	radare_read(0);

	last_print_format = fmt;
	free(str);

	return 1;
}

void radare_poke(char *arg)
{
	int fd;
	char key;
	int otimes, times = config_get_i("cfg.count");
	char *buf = NULL;
	off_t ret = 0;

	if (times<1)
		times = 1;
	otimes = times;

	if (!config_get("cfg.write")) {
		eprintf("You'r not in read-write mode.\n");
		return;
	}

	if (arg[0]=='\0') {
		eprintf("Usage: Poke [filename]\n");
		return;
	}
	fd = io_open(arg, O_RDONLY, 0644);
	if (fd > -1) {
		buf = malloc(config.block_size);
		ret = io_read(fd, buf, config.block_size);

		D { printf("\n");
		data_print(config.seek, (unsigned char *)buf, ret, FMT_HEXB, MD_ALWAYS);
		printf("\nPoke %d bytes from %s %d times? (y/N)",
			config.block_size, arg, times); fflush(stdout);
		terminal_set_raw(1); read(0, &key, 1); printf("\n");
		terminal_set_raw(0); } else key='y';

		if (key=='y' || key=='Y') {
			memcpy(config.block, buf, ret);

			radare_seek(config.seek, SEEK_SET);
			while(times--)
				io_write(config.fd, buf, ret);

			if ((config.seek + (ret * otimes)) > config.size) {
				if (config.limit == config.size)
					config.limit = config.seek + ret * otimes;
				config.size = config.seek + ret * otimes;
				D eprintf("file has growed.\n");
			}

			radare_seek(config.seek, SEEK_SET);

			if (config.size != -1)
				if (ret > config.size)
					config.size = ret;

			radare_read(0);
			D eprintf("file poked.\n");
		} else {
			D eprintf("nothing changed.\n");
		}
		io_close(fd);
	} else {
		D eprintf("Cannot open for read '%s'\n", arg);
	}
	free(buf);
}

void radare_dump(char *arg, int size)
{
	int fd;
	off_t ret = 0;

	if (arg[0]=='\0') {
		eprintf("Usage: dump [filename]\n");
	} else {
		fd = open(arg, O_CREAT|O_WRONLY|O_TRUNC, 0600);
		if (fd < 0) {
			eprintf("Cannot open for write '%s'\n", arg);
			return;
		}

		if ((ret = radare_read(0)) < 0) {
			eprintf("Error reading: %s\n", strerror(errno));
			return;
		}

		ret = io_write(fd, config.block, size);

		io_close(fd);
	}
}

off_t radare_seek(off_t offset, int whence)
{
	off_t seek = 0;

	if (offset==-1)
		return (off_t)-1;

	if (whence == SEEK_SET && config.baddr && offset>=config.size && offset >= config.baddr)
		offset-=config.baddr;
	
	seek = io_lseek(config.fd, offset, whence);

	switch(whence) {
	case SEEK_SET:
		if ((config.seek + offset) < 0)
			seek = 0;
		break;
	case SEEK_CUR:
		if ((config.seek + offset) < 0)
			offset = config.seek = 0;
		break;
	case SEEK_END:
		if (seek == -1 || config.size == -1) {
			D printf("Warning: file size is unknown\n");
			return -1;
		} else
			config.seek = config.size;
		break;
	}
	
	seek = io_lseek(config.fd, offset, whence);
	if (seek == -1)
		return config.seek = offset;

	if (whence != SEEK_END) {
		seek = io_lseek(config.fd, offset, whence);
		if (seek < 0) {
			seek = 0;
			return -1;
		}
	}

	if (whence == SEEK_SET)
		config.seek = seek;
	else	config.seek+= offset;

	io_lseek(config.fd, config.seek, SEEK_SET);

	return seek;
}

/* read a block from current or next seek */
int radare_read(int next)
{
	int ret = 0;

	if (config.fd == -1)
		return 0;

	if (config.seek==-1) {
		config.seek = 0;
		ret = config.block_size;
	} else {
		if (next)
			radare_seek(config.seek+config.block_size, SEEK_SET);
		else	radare_seek(config.seek, SEEK_SET);

		memset(config.block, '\xff', config.block_size);
		ret = io_read(config.fd, config.block, config.block_size);
		// XXX wrong aligned memory read !!! first 4 bytes are trash
		if (ret!=-1 && ret<config.block_size) {// aligned read
			radare_seek(config.seek+config.seek%4, SEEK_SET);
			ret = io_read(config.fd, config.block+(config.seek%4), config.block_size);
		}
	}

	if (ret == -1)
		perror("radare_read()==-1");

	return ret;
}
