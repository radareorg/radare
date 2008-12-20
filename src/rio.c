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

#if DEBUGGER
int radare_dump_section(char *tmpfile)
{
	u64 f, t, s;
	int ret = radare_get_region(&f, &t);
	s = t-f;

	if (ret == 0 || f == 0 || t == 0) {
		cons_printf("Cannot get region range\n");
		return 1;
	}
	cons_printf("Current section is: 0x%08llx 0x%08llx\n", f, t);
	make_tmp_file(tmpfile);
	radare_dump(tmpfile, s);

	return 0;
}
#endif

int radare_read_at(u64 offset, u8 *data, int len)
{
	int ret;
	u64 cur = config.seek;
	radare_seek(offset,SEEK_SET);
	ret = io_read(config.fd, data, len);
	radare_seek(cur, SEEK_SET);
	return ret;
}

static u8 *write_mask = NULL;
static int write_mask_len = 0;

int radare_write_mask(const u8 *mask, int len)
{
	if (mask == NULL) {
		free(write_mask);
		write_mask = NULL;
		write_mask_len = 0;
		return 0;
	}

	write_mask = (u8*)malloc(len);
	write_mask_len = len;
	memcpy(write_mask, mask, len);
	return 1;
}

int radare_write_mask_str(const char *str)
{
	int i, len;
	u8 mask[1024]; // XXX
	switch(str[0]) {
	case '\0':
	case '?':
		cons_strcat("Usage: wm[-] [hexpair-string]\n"
		" wm ff00ff  ; set binary mask for write ops\n"
		" wm-        ; unset current write binmask\n"
		"Current mask: ");
		for(i=0;i<write_mask_len;i++)
			cons_printf(" %02x", write_mask[i]);
		cons_newline();
		return 0;
	case '-':
		return radare_write_mask(NULL, 0);
	}
	len = hexstr2binstr(str, (u8 *)mask);
	if (len == -1)
			return 0;
	return radare_write_mask(mask, len);
}

int radare_write_at(u64 offset, const u8 *data, int len)
{
	int i;
	u64 cur = config.seek;
	radare_seek(offset,SEEK_SET);
	undo_write_new(offset, data, len);

	/* Apply write binary mask here */
	if (write_mask != NULL) {
		u8 *data2 = alloca(len);
		radare_read_at(offset, data2, len);
		for(i=0;i<len;i++) {
			data2[i] = data[i] & write_mask[i%write_mask_len];
		}
		data = data2;
	}

	len = io_write(config.fd, data, len);
	radare_seek(cur, SEEK_SET);
	radare_read(0);
	return len;
}

int radare_write_op(const char *arg, char op)
{
	char *str;
	u8 *buf;
	int i,j;
	int ret;
	int len;

	// XXX we can work with config.block instead of dupping it
	buf = (char *)malloc(config.block_size);
	str = (char *)malloc(strlen(arg));
	if (buf == NULL || str == NULL) {
		free(buf);
		free(str);
		return 0;
	}
	memcpy(buf, config.block, config.block_size);
	len = hexstr2binstr(arg, (unsigned char *)str);

	for(i=j=0;i<config.block_size;i++) {
		switch(op) {
		case 'x': buf[i] ^= str[j]; break;
		case 'a': buf[i] += str[j]; break;
		case 's': buf[i] -= str[j]; break;
		case 'm': buf[i] *= str[j]; break;
		case 'd': buf[i] /= str[j]; break;
		case 'r': buf[i] >>= str[j]; break;
		case 'l': buf[i] <<= str[j]; break;
		case 'o': buf[i] |= str[j]; break;
		case 'A': buf[i] &= str[j]; break;
		}
		j++; if (j>=len) j=0; /* cyclic key */
	}

	ret = radare_write_at(config.seek, buf, config.block_size);

	free(buf);
	return ret;
}

int radare_write(const char *argro, int mode)
{
	int fmt = last_print_format;
	u64 oseek = config.seek;
	u64 seek = config.seek;
	int times = config_get_i("cfg.count");
	int i, bytes = 0;
	int len   = 0;
	char *str, *tmp, *arg;

	arg = strdup(argro);

	if (times<=0)
		times = 1;

	if (!config_get("file.write")) {
		eprintf("Not in write mode. Type 'eval file.write = true'.\n");
		return 0;
	}

	str = strdup(arg);
	if (arg[0]=='0' && arg[1]=='x')
		mode = WMODE_HEX;

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
		free(arg);
		return 0;
	}

	arg[len]='\0';

	radare_seek(seek, SEEK_SET);

	if (config_get("file.insert")) {
		u64 rest;
		/* resize file here */
		if (config.size == -1) {
			eprintf("Cannot use file.insert: unknown file size\n");
		} else {
			/* TODO must take care about search mode for replacements */
			/* TODO check cfg.running or so? */
			/* TODO: SUPPORT WRITE WITH DELTA HERE!!! */
			if (config_get("file.insertblock")) {
				eprintf("file.insertblock: not yet implemented\n");
			} else {
				rest = config.size - seek; // + (len*times));
				if (rest > 0) {
					u8 *str = malloc(rest);
					io_read(config.fd, str, rest);
					io_lseek(config.fd, seek+(len*times), SEEK_SET);
					//undo_write_new(seek+(len*times), str, rest);
					config.size += len*times;
					io_write(config.fd, str, rest);
					free(str);
					io_lseek(config.fd, seek, SEEK_SET);
				}
			}
		}
	}
	for(bytes=0;times--;) {
		u8 *mystr = str;
		/* Apply write binary mask here */
		if (write_mask != NULL) {
			u8 *str2 = alloca(len);
			radare_read_at(config.seek, str2, len);
			for(i=0;i<len;i++) {
				str2[i] = str[i] & write_mask[i%write_mask_len];
			}
			mystr = str2;
		}
		undo_write_new(seek, mystr, len);
		bytes += io_write(config.fd, mystr, len);
		config.seek += len;
	}

	if (!config.debug)
	if (!config.unksize && seek + len > config.size)
		radare_open(1);

	radare_seek(oseek, SEEK_SET);
	radare_read(0);

	last_print_format = fmt;
	free(arg);
	free(str);

	return 1;
}

void radare_poke(const char *arg)
{
	int fd;
	char key;
	int otimes, times = config_get_i("cfg.count");
	char *buf = NULL;
	u64 ret = 0;

	if (times<1)
		times = 1;
	otimes = times;

	if (!config_get("file.write")) {
		eprintf("You are not in read-write mode. Use 'eval file.write = true'\n");
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
		print_data(config.seek, "", (unsigned char *)buf, ret, FMT_HEXB);
		printf("\nPoke %d bytes from %s %d times? (y/N)",
			config.block_size, arg, times); fflush(stdout);
		cons_set_raw(1); read(0, &key, 1); printf("\n");
		cons_set_raw(0); } else key='y';

		if (key=='y' || key=='Y') {
			memcpy(config.block, buf, ret);

			undo_write_new(config.seek, buf, ret);
			radare_seek(config.seek, SEEK_SET);
			while(times--) {
				undo_write_new(config.seek, buf, ret);
				io_write(config.fd, buf, ret);
			}

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

int radare_dump(const char *arg, int size)
{
	int fd;
	u64 ret = 0;
	int bs = config.block_size;

	if (arg[0]=='\0') {
		eprintf("Usage: dump [filename]\n");
	} else {
		fd = open(arg, O_CREAT|O_WRONLY|O_TRUNC, 0644);
		if (fd < 0) {
			eprintf("Cannot open for write '%s'\n", arg);
			return 0;
		}

		radare_set_block_size_i(size);
		if ((ret = radare_read(0)) < 0) {
			eprintf("Error reading: %s\n", strerror(errno));
			return 0;
		}

		undo_write_new(config.seek, config.block, size);
		ret = io_write(fd, config.block, size);

		io_close(fd);
		radare_set_block_size_i(bs);
	}
	return 1;
}

u64 radare_seek(u64 offset, int whence)
{
	u64 preoffset = 0;
	u64 seek = 0;
	int bip = 0;

	if (offset==-1)
		return (u64)-1;

#if 1
	if (whence == SEEK_SET && config.vaddr && offset>=config.vaddr)//&& offset >= config.vaddr)
	{
		preoffset = offset;
		offset = section_align(offset, config.vaddr, config.paddr);
		bip = 1;
		//offset-=config.vaddr;
	}
#endif
	
	seek = io_lseek(config.fd, offset, whence);
	if (seek==-1)
		return -1;

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
		} else config.seek = config.size;
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

	if (bip) {
		if (whence == SEEK_SET)
			config.seek = seek;
		else	config.seek+= preoffset;
	} else {
		if (whence == SEEK_SET)
			config.seek = seek;
		else	config.seek+= offset;
	}

	if (config.seek <0)
		config.seek = 0;

//eprintf("RAL SEEK %llx\n", config.seek);
	io_lseek(config.fd, config.seek, SEEK_SET);

	//undo_push();

	return seek;
}

/* read a block from current or next seek */
int radare_read(int next)
{
	int ret = 0;

	if (config.fd == -1)
		return 0;

	if (config.seek == -1) {
		config.seek = 0;
		ret = config.block_size;
	} else {
		if (next)
			radare_seek(config.seek+config.block_size, SEEK_SET);
		else	radare_seek(config.seek, SEEK_SET);

		memset(config.block, '\xff', config.block_size);
		ret = io_read(config.fd, config.block, config.block_size);
	}

	return ret;
}

