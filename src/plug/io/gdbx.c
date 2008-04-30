/*
 * Copyright (C) 2008
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

#include <plugin.h>
#include <radare.h>
#include <socket.h>

static int gdbx_fd = -1;

int gdbx_handle_fd(int fd)
{
	return fd == gdbx_fd;
}

int gdbx_handle_open(const char *file)
{
	if (!memcmp(file, "gdbx://", 7))
		return 1;
	return 0;
}


static void gdbx_wait_until_prompt(int o)
{
	unsigned char buf;
	int off = 0;

	while(1) {
		read(config.fd, &buf, 1);
		if (o) write(2, &buf, 1);
		switch(off) {
		case 0: if (buf == 'g') off =1; break;
		case 1: if (buf == 'd') off =2; break;
		case 2: if (buf == 'b') off =3; break;
		case 3: if (buf == ')') off =4; break;
		case 4: if (buf == ' ') return; else off = 0; break;
		}
	}
}

ssize_t gdbx_write(int fd, const void *buf, size_t count)
{
	// TODO: not yet implemented (pfd command ?)
	return 0;
	socket_printf(config.fd, "set *%08llx = <expr>\n", config.seek);
	gdbx_wait_until_prompt(0);
        return 0;
}

ssize_t gdbx_read(int fd, unsigned char *buf, size_t count)
{
	char tmp[1024];
	unsigned char *pbuf = buf;
	unsigned char *ptr;
	int i,j=0,k;
	int size = count;

	if (size%16)
		size+=(size-(size%16));

	for(i=0;i<count;i+=4) {
		unsigned long *dword = buf+i;
		unsigned long dw;
		sprintf(tmp,"x/x 0x"OFF_FMTx"\n", config.seek+i);
//eprintf(">> %s\n", tmp);
		socket_printf(config.fd, tmp);
		tmp[0]='\0';
		socket_fgets(gdbx_fd, tmp, 1024);
//eprintf("<< %s\n", tmp);
		if (strstr(tmp, "Cannot"))
			return ((i==0)?0:i); // do not use -1:i
		ptr = strchr(tmp, ':');
		if (ptr) {
			sscanf(ptr+2, "0x%x", &dw);
			//printf(">> value >> (%s) 0x%08x\n", ptr+2, dw);
			endian_memcpy_e(dword, &dw, 4, 1);
		} else {
			memcpy(dword, "\x01\x02\x03\x04", 4);
		}
		gdbx_wait_until_prompt(0);
	}

        return count;
}

static void gdbx_wait_until_entrypoint()
{
	u64 entry=0;
	unsigned char buf;
	char entry_str[33];
	int off = 0;
	int found = 0;

	socket_printf(config.fd, "info target\n");
	while(!found) {
		socket_read(config.fd, &buf, 1);
		switch(off) {
		case 0: if (buf == 'i') off =1; break;
		case 1: if (buf == 'n') off =2; break;
		case 2: if (buf == 't') off =3; break;
		case 3: if (buf == ':') off =4; break;
		case 4: if (buf == ' ') {
				// get entrypoint address
				socket_fgets(gdbx_fd, entry_str, 32);
				entry = get_offset(entry_str);
fprintf(stderr, " => Entry point: 0x%08llx\n", entry);
#if __APPLE__
fprintf(stderr, "\n >> Type '!bp 0x%08llx' and '!run' <<\n\n", entry);
#endif
		found=1;
			} else {
				off = 0;
			}
			break;
		}
	}
	gdbx_wait_until_prompt(0);
	if (entry != 0LL) {
		socket_printf(config.fd, "break *0x%08llx\nrun\n", entry);
		gdbx_wait_until_prompt(0);
		config.seek = entry;
	}
}


int gdbx_open(const char *pathname, int flags, mode_t mode)
{
	int fd;
	char host[128];
	char tmp[4096];
	int port, i;

	srand(getpid());
	port = 9000+rand()%555;
	config.block_size = 30;

	config.debug = 1;
	config_set("cfg.debug","true");
	eprintf("Listening at port %d (telnet me to get the program output)\n", port);
	if (!fork()) {
		if (!fork()) {
			system("pkill gdb"); // XXX
			system("pkill tm"); // XXX
			sprintf(tmp, "tm -N 10 -w -n -p %d gdb --args %s", port, pathname+7);
			system(tmp);
			printf("(%s) has exited!\n", tmp);
			exit(1);
		}
		exit(0);
	}
	sleep(2);
	
	// waitpid and return -1 if not exist
	fd = socket_connect("localhost", port);
	if (fd != -1) {
		config.fd = fd;
		gdbx_fd = fd;
		//gdbx_wait_until_prompt();
	} else
		printf("Cannot connect to remote host.\n");
	strcpy(config.file, pathname); //+10);
	socket_printf(config.fd, "\n");

	gdbx_wait_until_prompt(1);
	gdbx_wait_until_entrypoint(1);

	return fd;
}

int gdbx_system(const char *cmd)
{
	char tmp[130];
	int i;

	if (cmd[0]=='=') {
		socket_printf(config.fd, cmd+1);
		socket_printf(config.fd, "\n");
		// TODO: print out data
		gdbx_wait_until_prompt(1);
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
		" !regs[*]         show or flag registers\n"
		" !=cmd            execute a gdbx command\n"
		" !!help           gdbx help\n");
	} else
	if (!memcmp(cmd, "bp ",3 )) {
		char buf[1024];
		// TODO: Support for removal in a radare-like way
		sprintf(buf, "break *0x%08llx\n", get_offset(cmd+3));
		socket_printf(config.fd, buf);
cons_printf("breakpoint set\n");
		gdbx_wait_until_prompt(0);
	} else
	if (!memcmp(cmd, "set ", 4)) {
		char *ptr = strchr(cmd+4,' ');
		char buf[1024];

		if (ptr == NULL) {
			printf("Usage: !set [reg] [value]\n");
		} else {
			ptr[0]='\0';
			sprintf(buf, "set $%s = %s\n", cmd+4, ptr+1);
			socket_printf(config.fd, buf);
			gdbx_wait_until_prompt(1);
		}
	} else
	if (!strcmp(cmd, "cont")) {
		socket_printf(config.fd, "cont\n");
		gdbx_wait_until_prompt(1);
	} else
	if (!strcmp(cmd, "run")) {
		socket_printf(config.fd, "run\n");
		gdbx_wait_until_prompt(1);
	} else
	if (!strcmp(cmd, "bt")) {
		socket_printf(config.fd, "bt\n");
		gdbx_wait_until_prompt(1);
	} else
	if (!strcmp(cmd, "pids")) {
		socket_printf(config.fd, "info process\n");
		gdbx_wait_until_prompt(1);
	} else
	if (!strcmp(cmd, "th")) {
		socket_printf(config.fd, "into thread\n");
		gdbx_wait_until_prompt(1);
	} else
	if (!strcmp(cmd, "maps")) {
		socket_printf(config.fd, "info maps\n");
		socket_printf(config.fd, "info regions\n");
		gdbx_wait_until_prompt(1);
	} else
	if (!strcmp(cmd, "step")) {
		socket_printf(config.fd, "stepi\n");
		gdbx_wait_until_prompt(0);
	} else
	if (!memcmp(cmd, "regs",4)) {
		unsigned char name[128];
		unsigned long value;
		unsigned long eip, esp,ebp,eflags,eax,ebx,ecx,edx,esi,edi;
		socket_printf(config.fd, "info registers\n");
		
		for(i=0;;i++) {
			tmp[0]='\0';
		do {
			socket_read(config.fd, tmp, 1);
		} while(tmp[0]=='\n');
			if (tmp[0]=='(') {
				gdbx_wait_until_prompt(0);
				break;
			}
			socket_fgets(gdbx_fd, tmp+1, 128);
//fprintf(stderr, " _ %s\n", tmp);
			if (strstr(tmp, "no registers")) {
				eprintf("The program is not running\n");
				return 0;
			}
			if (!memcmp(tmp, "(gdb)", 5))
				break;
			sscanf(tmp, "%s 0x%x", name, &value);
			if (cmd[4]=='*') {
				cons_printf("f %s @ 0x%08x\n", name, value);
			} else {
				cons_printf("%6s = 0x%08x   ", name, value);
				if ((i%4)==3) cons_strcat("\n");
			}
		}

		if (cmd[4]!='*')
			cons_strcat("\n");
		//cons_flush();

#if 0
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
		gdbx_wait_until_prompt(0);
#endif
	}
	return 0;
}

int gdbx_close(int fd)
{
	return close(fd);
}

u64 gdbx_lseek(int fildes, u64 offset, int whence)
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

plugin_t gdbx_plugin = {
	.name = "gdbx",
	.desc = "GDB shell interface 'gdbx://program.exe args' )",
	.init = NULL,
	.debug = NULL, // XXX
	.system = gdbx_system,
	.handle_fd = gdbx_handle_fd,
	.handle_open = gdbx_handle_open,
	.open = gdbx_open,
	.read = gdbx_read,
	.write = gdbx_write,
	.lseek = gdbx_lseek,
	.close = gdbx_close
};
