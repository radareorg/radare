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

static int winedbg_fd = -1;

int winedbg_handle_fd(int fd)
{
	return fd == winedbg_fd;
}

int winedbg_handle_open(const char *file)
{
	if (!memcmp(file, "winedbg://", 10))
		return 1;
	return 0;
}

ssize_t winedbg_write(int fd, const void *buf, size_t count)
{
	// TODO: not yet implemented (pfd command ?)
        return 0;
}


static void winedbg_wait_until_prompt(int o)
{
	unsigned char buf;
	int off = 0;

	while(1) {
		read(config.fd, &buf, 1);
		if (o) write(2, &buf, 1);
		switch(off) {
		case 0: if (buf == '-') off =1; break;
		case 1: if (buf == 'd') off =2; break;
		case 2: if (buf == 'b') off =3; break;
		case 3: if (buf == 'g') off =4; break;
		case 4: if (buf == '>') return; else off = 0; break;
		}
	}
}

ssize_t winedbg_read(int fd, unsigned char *buf, size_t count)
{
	char tmp[1024];
	unsigned char *pbuf = buf;
	unsigned char *ptr;
	int i,j=0,k;
	int size = count;
	int delta = config.seek%4;

	if (size%16)
		size+=(size-(size%16));

	// XXX memory is algned!!!
	for(i=0;i<count;i+=4) {
		unsigned long *dword = buf+i;
		unsigned long dw;
		sprintf(tmp,"x 0x"OFF_FMTx"\n", config.seek-delta+i);
		socket_printf(config.fd, tmp);
		tmp[0]='\0';
		socket_fgets(tmp, 1024);
		sscanf(tmp, "%08x", &dw);
		endian_memcpy_e(dword, &dw, 4, 1);
		winedbg_wait_until_prompt(0);
	}

        return count;
}
int winedbg_open(const char *pathname, int flags, mode_t mode)
{
	int fd;
	char host[128];
	char tmp[4096];
	int port, i;

	srand(getpid());
	port = 9000+rand()%555;

	config.debug = 1;
	if (!fork()) {
		system("pkill winedbg"); // XXX
		system("pkill tm"); // XXX
		sprintf(tmp, "tm -N 10 -w -n -p %d winedbg %s", port, pathname+10);
		system(tmp);
		printf("(%s) has exited!\n", tmp);
		exit(1);
	}
	sleep(2);
	
	// waitpid and return -1 if not exist
	fd = socket_connect("localhost", port);
	if (fd != -1) {
		config.fd = fd;
		winedbg_fd = fd;
		//winedbg_wait_until_prompt();
	} else
		printf("Cannot connect to remote host.\n");
	strcpy(config.file, pathname); //+10);
	socket_printf(config.fd, "\n");
	winedbg_wait_until_prompt(0);

	return fd;
}

int winedbg_system(const char *cmd)
{
	char tmp[130];
	if (cmd[0]=='!') {
		socket_printf(config.fd, cmd+1);
		socket_printf(config.fd, "\n");
		// TODO: print out data
		winedbg_wait_until_prompt(1);
	} else
	if (!strcmp(cmd, "help")) {
		cons_printf("WineDbg help\n"
		" !step [N]     steps one or N instructions\n"
		" !regs[*]      show or flag registers\n"
		" !!cmd         execute a winedbg command\n"
		" !!help        winedbg help\n");
	} else
	if (!strcmp(cmd, "step")) {
		socket_printf(config.fd, "stepi\n");
		winedbg_wait_until_prompt(0);
	} else
	if (!memcmp(cmd, "regs",4)) {
		unsigned long eip, esp,ebp,eflags,eax,ebx,ecx,edx,esi,edi;
		socket_printf(config.fd, "info regs\n");
		
		tmp[0]='\0';
		socket_fgets(tmp, 128);
		//  CS:0073 SS:007b DS:007b ES:007b FS:0033 GS:003b
		tmp[0]='\0';
		socket_fgets(tmp, 128);
		//  EIP:7ee95b9e ESP:0034ff20 EBP:0034ffe8 EFLAGS:00200202(   - 00      - - I1)
		tmp[0]='\0';
		socket_fgets(tmp, 128);
		sscanf(tmp, " EIP:%08x ESP:%08x EBP:%08x EFLAGS:%08x", &eip, &esp, &ebp, &eflags);
		//  EAX:00000000 EBX:7eecb8a8 ECX:00000000 EDX:00000000
		tmp[0]='\0';
		socket_fgets(tmp, 128);
		sscanf(tmp, " EAX:%08x EBX:%08x ECX:%08x EDX:%08x", &eax, &ebx, &ecx, &edx);
		//  ESI:7ffdf000 EDI:00403166
		tmp[0]='\0';
		socket_fgets(tmp, 128);
		sscanf(tmp, " ESI:%08x EDI:%08x", &esi, &edi);
		if (cmd[4]=='*') {
			cons_printf("f eip @ 0x%08x\n", eip);
			cons_printf("f esp @ 0x%08x\n", esp);
			cons_printf("f ebp @ 0x%08x\n", ebp);
			cons_printf("f esi @ 0x%08x\n", esi);
			cons_printf("f edi @ 0x%08x\n", edi);
			cons_printf("f eax @ 0x%08x\n", eax);
			cons_printf("f ebx @ 0x%08x\n", ebx);
			cons_printf("f ecx @ 0x%08x\n", ecx);
			cons_printf("f edx @ 0x%08x\n", edx);
			cons_printf("f eflags @ 0x%08x\n", eflags );
		} else {
			cons_printf(" eip = 0x%08x  eax = 0x%08x\n", eip, eax);
			cons_printf(" esp = 0x%08x  ebx = 0x%08x\n", esp, ebx);
			cons_printf(" ebp = 0x%08x  ecx = 0x%08x\n", ebp, ecx);
		}
		winedbg_wait_until_prompt(0);
	}
	return 0;
}

int winedbg_close(int fd)
{
	return close(fd);
}

u64 winedbg_lseek(int fildes, u64 offset, int whence)
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

// TODO: add field .is_debugger

plugin_t winedbg_plugin = {
	.name = "winedbg",
	.desc = "Wine Debugger interface ( winedbg://program.exe )",
	.init = NULL,
	.debug = NULL, // XXX
	.system = winedbg_system,
	.handle_fd = winedbg_handle_fd,
	.handle_open = winedbg_handle_open,
	.open = winedbg_open,
	.read = winedbg_read,
	.write = winedbg_write,
	.lseek = winedbg_lseek,
	.close = winedbg_close
};
