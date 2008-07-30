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

#include "rabin/rabin.h"
#if 0
enum {
	FILETYPE_UNK = 0,
	FILETYPE_ELF,
	FILETYPE_MZ,
	FILETYPE_PE,
	FILETYPE_CLASS,
	FILETYPE_CSRFW,
};
#endif
// XXX duppend between rabin/rabin.c and rabin.c

static unsigned int pebase;

/* arch/java/javasm.c */
int java_classdump(const char *file);


u64 rabin_entrypoint(int filetype)
{
	unsigned long addr = 0;
	unsigned long base = 0;

	switch(filetype) {
	case FILETYPE_CSRFW:
		eprintf("filetype: CSR firmware. Not yet supported by rabin\n");
		return addr;
		break;
	case FILETYPE_ELF:
		io_lseek(config.fd, 0x18, SEEK_SET);
		io_read(config.fd, &addr, 4);
		io_lseek(config.fd, 0, SEEK_SET);

		/* FIX */
		if (addr>0x8048000)
			return addr-0x8048000;
		return addr;
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
		return (u64)(addr)-0xc00;
	}
	return 0;
}

int rabin_identify_header()
{
	int pe, filetype = FILETYPE_UNK;
	unsigned char buf[1024];

	io_lseek(config.fd, 0, SEEK_SET);
	io_read(config.fd, buf, 1024);

        if (!memcmp(buf, "\xCA\xFE\xBA\xBE", 4))
		if (buf[9])
                	filetype = FILETYPE_CLASS;
		else	filetype = FILETYPE_MACHO;
	else
	if (!memcmp(buf, "CSR-", 4)) {
		filetype = FILETYPE_CSRFW;
		config_set("asm.arch", "csr");
	} else
        if (!memcmp(buf, "\xFE\xED\xFA\xCE", 4)) {
		filetype = FILETYPE_MACHO;
		printf("endian = big\n");
	} else	
	if (!memcmp(buf, "dex\n009\0", 8))
		filetype = FILETYPE_DEX;
	else
	if (!memcmp(buf, "\x7F\x45\x4c\x46", 4))
		filetype = FILETYPE_ELF;
	else
	if (!memcmp(buf, "\x4d\x5a", 2)) {
		int pe = buf[0x3c];
		filetype = FILETYPE_MZ;
		if (buf[pe]=='P' && buf[pe+1]=='E') {
			filetype = FILETYPE_PE;
			pebase = pe;
		}
	} else {
		printf("Unknown filetype\n");
	}

	return filetype;
}
#if 0
int rabin_identify_header()
{
	int pe, filetype = FILETYPE_UNK;
	unsigned char buf[1024];

	io_lseek(config.fd, 0, SEEK_SET);
	io_read(config.fd, buf, 1024);
	if (!memcmp(buf, "\x7F\x45\x4c\x46", 4))
		filetype = FILETYPE_ELF;
	else
	if (!memcmp(buf, "CSR-", 4)) {
		filetype = FILETYPE_CSRFW;
		config_set("asm.arch", "csr");
	} else
	if (!memcmp(buf, "\xca\xfe\xba\xbe", 4))
		filetype = FILETYPE_CLASS;
	else
		if (!memcmp(buf, "\x4d\x5a", 2)) {
			pe = buf[0x3c];
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
#endif

int rabin_load()
{
	char buf[255];
	int header = rabin_identify_header();
	u64 entry = rabin_entrypoint(header);

	if (header == FILETYPE_UNK) {
		config_set("file.type", "unk");
		return 0;
	}

	config_set_i("file.entrypoint", entry);
	sprintf(buf, "fs symbols && f entrypoint @ %08llx", entry);
	radare_cmd(buf, 0);
	//flag_set("entrypoint", entry, 0);
	radare_cmd("s entrypoint",0);

	/* add autodetection stuff here */
	switch(header) {
	case FILETYPE_MACHO:
		config_set("file.type", "macho");
		config_set("asm.arch", "ppc");
		config_set("cfg.endian", "false");
		 break;
	case FILETYPE_ELF:
		config_set("file.type", "elf");
		config_set_i("file.baddr", (u64)0x8048000); // XXX doesnt works! :(
		#if __i386__
		config.baddr = 0x8048000;
		#else
		config.baddr = 0x8000; // ARM
		#endif
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
		config_set("cfg.endian", "true"); // we need big endian for proper java disassembly
		// loading class information
		java_classdump(config.file);
		//radare_cmd(".!javasm -rc ${FILE}");
		break;
	}

	// TODO: autodetect arch
	return 0;
}
