/*
 * Copyright (C) 2009
 *       pancake <nopcode.org>
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

#include <plugin.h>
#include <radare.h>
#include <socket.h>

// TODO: see socket.c and io/serial.c to see how to do async reads

static int windbg_fd = -1;

int windbg_handle_fd(int fd)
{
	return fd == windbg_fd;
}

int windbg_handle_open(const char *file)
{
	return (!memcmp(file, "windbg://", 10))?1:0;
}

ssize_t windbg_write(int fd, const void *buf, size_t count)
{
	// TODO: implement write op
        return 0;
}

ssize_t windbg_read(int fd, void *buf, size_t count)
{
	// TODO: implement read operation
	printf("read at 0x%08llx\n", config.seek);
	memset(buf, 0, count);
        return count;
}

int windbg_open(const char *pathname, int flags, mode_t mode)
{
	int fd;
	char tmp[4096];

#if __UNIX__
	// TODO: detect if pathname is socket file or serial port
	// TODO: atm only socket file is supported
	// waitpid and return -1 if not exist
	fd = socket_unix_connect(pathname);
	if (fd == -1) {
		fprintf(stderr, "Cannot connect to remote host.\n");
		return -1;
	}
#endif
	config.fd = fd;
	windbg_fd = fd;
	config.debug = 1;
	
	//free(config.file);
	config.file = strdup(pathname);
	/* TODO: implement handshake with windbg */

	return fd;
}

int windbg_system(const char *cmd)
{
	char tmp[130];
	if (cmd[0]=='!') {
		socket_printf(config.fd, cmd+1);
		socket_printf(config.fd, "\n");
		// TODO: print out data
	} else
	if (!strcmp(cmd, "help")) {
		cons_printf("WineDbg help\n"
		" !pids            show all running processes\n"
		" !maps            show maps information\n"
		" !bt              list backtrace\n"
		" !th              show threads information\n"
		" !set [reg] [val] set a value for a register\n"
		" !step [N]        steps one or N instructions\n"
		" !cont            continue program execution\n"
		" !bp <addr>       set breakpoint at address\n"
		" !reg[*]          show or flag registers\n"
		" !!cmd            execute a windbg command\n"
		" !!help           windbg help\n");
	} else
	if (!memcmp(cmd, "bp ", 3)) {
		char buf[1024];
		// TODO: Support for removal in a radare-like way
		sprintf(buf, "break %08llx\n", get_offset(cmd+3));
		socket_printf(config.fd, buf);
	} else
	if (!memcmp(cmd, "set ", 4)) {
		char *ptr = strchr(cmd+4,' ');
		char buf[1024];

		if (ptr == NULL) {
			printf("Usage: !set [reg] [value]\n");
		} else {
			// TODO
#if 0
			ptr[0]='\0';
			sprintf(buf, "set $%s = %s\n", cmd+4, ptr+1);
			socket_printf(config.fd, buf);
#endif
		}
	} else
	if (!strcmp(cmd, "cont")) {
		// TODO 
	} else
	if (!strcmp(cmd, "bt")) {
		// TODO
	} else
	if (!strcmp(cmd, "pids")) {
		// TODO
	} else
	if (!strcmp(cmd, "th")) {
		// TODO: show threads
	} else
	if (!strcmp(cmd, "maps")) {
		// TODO
	} else
	if (!strcmp(cmd, "step")) {
		// TODO
	} else
	if (!memcmp(cmd, "reg",3)) {
		// TODO
#if 0
		unsigned int eip, esp,ebp,eflags,eax,ebx,ecx,edx,esi,edi;
		socket_printf(config.fd, "info regs\n");
		
		tmp[0]='\0';
		socket_fgets(config.fd, tmp, 128);
		//  CS:0073 SS:007b DS:007b ES:007b FS:0033 GS:003b
		tmp[0]='\0';
		socket_fgets(config.fd, tmp, 128);
		//  EIP:7ee95b9e ESP:0034ff20 EBP:0034ffe8 EFLAGS:00200202(   - 00      - - I1)
		tmp[0]='\0';
		socket_fgets(config.fd, tmp, 128);
		sscanf(tmp, " EIP:%08x ESP:%08x EBP:%08x EFLAGS:%08x", &eip, &esp, &ebp, &eflags);
		//  EAX:00000000 EBX:7eecb8a8 ECX:00000000 EDX:00000000
		tmp[0]='\0';
		socket_fgets(config.fd, tmp, 128);
		sscanf(tmp, " EAX:%08x EBX:%08x ECX:%08x EDX:%08x", &eax, &ebx, &ecx, &edx);
		//  ESI:7ffdf000 EDI:00403166
		tmp[0]='\0';
		socket_fgets(config.fd, tmp, 128);
		sscanf(tmp, " ESI:%08x EDI:%08x", &esi, &edi);
		if (cmd[3]=='*') {
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
			cons_printf(" eip = 0x%08x  eax = 0x%08x  edx = 0x%08x\n", eip, eax, edx);
			cons_printf(" esp = 0x%08x  ebx = 0x%08x  esi = 0x%08x\n", esp, ebx, esi);
			cons_printf(" ebp = 0x%08x  ecx = 0x%08x  edi = 0x%08x  efl = %x\n",
				ebp, ecx, edi, eflags);
		}
#endif
	}
	return 0;
}

int windbg_close(int fd)
{
	return close(fd);
}

ut64 windbg_lseek(int fildes, ut64 offset, int whence)
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

// TODO: add field .is_debugger?

plugin_t windbg_plugin = {
	.name = "windbg",
	.desc = "Windbg serial port debugger ( windbg:///path/to/socket )",
	.init = NULL,
	.debug = NULL, // XXX
	.system = windbg_system,
	.handle_fd = windbg_handle_fd,
	.handle_open = windbg_handle_open,
	.open = windbg_open,
	.read = windbg_read,
	.write = windbg_write,
	.lseek = windbg_lseek,
	.close = windbg_close
};
