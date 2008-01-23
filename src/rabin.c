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

enum {
	FILETYPE_UNK = 0,
	FILETYPE_ELF,
	FILETYPE_MZ,
	FILETYPE_PE,
	FILETYPE_CLASS
};

static unsigned int pebase;

off_t rabin_entrypoint(int filetype)
{
	unsigned long addr = 0;
	unsigned long base = 0;

	switch(filetype) {
		case FILETYPE_ELF:
			io_lseek(config.fd, 0x18, SEEK_SET);
			io_read(config.fd, &addr, 4);
			io_lseek(config.fd, 0, SEEK_SET);
			return addr-0x8048000; // XXX FIX
			//pprintf("0x%08x memory\n", addr);
			//pprintf("0x%08x disk\n", addr - 0x8048000);
			break;
		case FILETYPE_MZ:
			break;
		case FILETYPE_PE:
			io_lseek(config.fd, pebase+0x28, SEEK_SET);
			io_read(config.fd, &addr, 4);
			//printf("0x%08x disk offset for ep\n", pebase+0x28);
			//printf("0x%08x disk\n", addr);

			io_lseek(config.fd, pebase+0x45, SEEK_SET);
			io_read(config.fd, &base, 4);
			eprintf("entry disk: 0x%08x\n", addr);
			eprintf("base address: 0x%08x\n", base);
			eprintf("entry in memory\n", base+addr);

			io_lseek(config.fd, 0, SEEK_SET);
			return (off_t)(addr)-0xc00;
	}
	return 0;
}

int rabin_identify_header()
{
	int filetype = FILETYPE_UNK;
	unsigned char buf[1024];

	io_lseek(config.fd, 0, SEEK_SET);
	io_read(config.fd, buf, 1024);
	if (!memcmp(buf, "\x7F\x45\x4c\x46", 4))
		filetype = FILETYPE_ELF;
	else
		if (!memcmp(buf, "\xca\xfe\xba\xbe", 4))
			filetype = FILETYPE_CLASS;
		else
			if (!memcmp(buf, "\x4d\x5a", 2)) {
				int pe = buf[0x3c];
				filetype = FILETYPE_MZ;
				if (buf[pe]=='P' && buf[pe+1]=='E') {
					filetype = FILETYPE_PE;
					pebase = pe;
				}
			} else {
				D eprintf("Unknown filetype\n");
			}

	return filetype;
}

int rabin_load()
{
	int header = rabin_identify_header();
	off_t entry = rabin_entrypoint(header);

	if (header == FILETYPE_UNK) {
		config_set("file.type", "unk");
		return 0;
	}

	config_set_i("file.entrypoint", entry);
	flag_set("entrypoint", entry, 0);
	radare_command("s entrypoint",0);

	/* add autodetection stuff here */
	switch(header) {
		case FILETYPE_ELF:
			config_set("file.type", "elf");
			//config_set_i("file.baddr", 0x8048000);
			config_set_i("file.baddr", (off_t)0x8048000); // XXX doesnt works! :(
			config.baddr = 0x8048000;
			break;
		case FILETYPE_MZ:
			config_set("file.type", "mz");
			break;
		case FILETYPE_PE:
			config_set("file.type", "pe");
			break;
		case FILETYPE_CLASS:
			config_set("file.type", "class");
			config_set("asm.arch", "java");
			break;
	}
	// TODO: autodetect arch
}