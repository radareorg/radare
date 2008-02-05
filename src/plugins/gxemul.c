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

#include "../main.h"
#include "../plugin.h"
#include "../socket.h"

// TODO handle SIGINT to stop socket blocks

static int gxemul_fd = -1;

int gxemul_handle_fd(int fd)
{
	return fd == gxemul_fd;
}

int gxemul_handle_open(const char *file)
{
	if (!memcmp(file, "gxemul://", 9))
		return 1;
	return 0;
}

ssize_t gxemul_write(int fd, const void *buf, size_t count)
{
	// TODO: not yet implemented (pfd command ?)
        return 0;
}


static void gxemul_wait_until_prompt(int o)
{
	unsigned char buf;
	int off = 0;

	while(1) {
		read(config.fd, &buf, 1);
		if (o) write(2, &buf, 1);
		switch(off) {
		// "GXemul> "
		case 0: if (buf == 'e') off =1; break;
		case 1: if (buf == 'm') off =2; break;
		case 2: if (buf == 'u') off =3; break;
		case 3: if (buf == 'l') off =4; break;
		case 4: if (buf == '>') return; else off = 0; break;
		}
	}
}

ssize_t gxemul_read(int fd, unsigned char *buf, size_t count)
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
		unsigned long dw, tm;
		sprintf(tmp, "dump 0x%08x 0x%08x\n",
			(unsigned long) config.seek+i,
			(unsigned long) config.seek+i+4);
		socket_printf(config.fd, tmp);
		tmp[0]='\0';
		socket_fgets(tmp, 1024); // dup
		socket_fgets(tmp, 1024);
		sscanf(tmp, "0x%08x %08x", &tm, &dw);
		endian_memcpy_e(dword, &dw, 4, 1);
		gxemul_wait_until_prompt(0);
	}

        return count;
}
int gxemul_open(const char *pathname, int flags, mode_t mode)
{
	int fd;
	char host[128];
	char tmp[4096];
	int port, i;

	srand(getpid());
	port = 9000+rand()%555;

	config.debug = 1;
	if (!fork()) {
		system("pkill gxemul"); // XXX
		system("pkill tm"); // XXX
		sprintf(tmp, "tm -N 10 -n -w -p %d gxemul -E netwinder -i -q -p 4 '%s'", port, pathname+9);
		system(tmp);
		printf("(%s) has exited!\n", tmp);
		system("pkill tm"); // XXX
		exit(1);
	}
	sleep(1);
printf("port: %d\n", port);
	
	// waitpid and return -1 if not exist
	fd = socket_connect("localhost", port);
	if (fd != -1) {
		config.fd = fd;
		gxemul_fd = fd;
		//gxemul_wait_until_prompt();
	} else
		printf("Cannot connect to remote host.\n");
	strcpy(config.file, pathname); //+10);
	socket_printf(config.fd, "\n");
	gxemul_wait_until_prompt(0);

	return fd;
}

int gxemul_system(const char *cmd)
{
	char tmp[130];
	if (cmd[0]=='!') {
		socket_printf(config.fd, cmd+1);
		socket_printf(config.fd, "\n");
		// TODO: print out data
		gxemul_wait_until_prompt(1);
	} else
	if (!strcmp(cmd, "help")) {
		cons_printf("GxEmul Debugger help\n"
		" !step [N]     steps one or N instructions\n"
		" !regs[*]      show or flag registers\n"
		" !!cmd         execute a gxemul command\n"
		" !!help        gxemul help\n");
	} else
	if (!strcmp(cmd, "step")) {
		socket_printf(config.fd, "step\n");
		gxemul_wait_until_prompt(0);
	} else
	if (!memcmp(cmd, "regs",4)) {
		int i;
		unsigned long r[16];
		unsigned long cpsr; // r[16]
		//unsigned long eip, esp,ebp,eflags,eax,ebx,ecx,edx,esi,edi;
		socket_printf(config.fd, "reg\n");
		socket_fgets(tmp, 1024); // dup
		
		tmp[0]='\0';
		socket_fgets(tmp, 128);
		sscanf(tmp, "cpu0:  cpsr = %6s   pc = %08x", &cpsr, &r[15]);
		tmp[0]='\0';
		socket_fgets(tmp, 128);
		sscanf(tmp, "cpu0:  r0 = 0x%08x r1 = %08x  r2 = %08x  r3 = %08x", &r[0], &r[1], &r[2], &r[3]);
		tmp[0]='\0';
		socket_fgets(tmp, 128);
		sscanf(tmp, "cpu0:  r4 = 0x%08x r5 = %08x  r6 = %08x  r7 = %08x", &r[4], &r[5], &r[6], &r[7]);
		tmp[0]='\0';
		socket_fgets(tmp, 128);
		sscanf(tmp, "cpu0:  r8 = 0x%08x r9 = %08x  sl = %08x  fp = %08x", &r[8], &r[9], &r[10], &r[11]);
		tmp[0]='\0';
		socket_fgets(tmp, 128);
		sscanf(tmp, "cpu0:  ip = 0x%08x sp = %08x  lr = %08x", &r[12], &r[13], &r[14]);
		if (cmd[4]=='*') {
			cons_printf("f eip @ 0x%08x\n", r[15]);
			cons_printf("f esp @ 0x%08x\n", r[13]);
			cons_printf("f lr  @ 0x%08x\n", r[14]);
			for(i=0;i<13;i++)
				cons_printf("f r%d @ 0x%08x\n", i, r[i]);
		} else {
			cons_printf("  r0 0x%08x   r4 0x%08x   r8 0x%08x\n", r[0], r[4], r[8]);
			cons_printf("  r1 0x%08x   r5 0x%08x   r9 0x%08x\n", r[1], r[5], r[9]);
			cons_printf("  r2 0x%08x   r6 0x%08x  r10 0x%08x\n", r[2], r[6], r[10]);
			cons_printf("  r3 0x%08x   r7 0x%08x r11(fp)0x%08x\n", r[3], r[7], r[11]);
			cons_printf("  r0.orig   0x%08x   r14 (lr)   0x%08x\n", r[17], r[14]);
			cons_printf("  r12 (ip)  0x%08x   r15 (pc)   0x%08x\n", r[12], r[15]);
			cons_printf("  r13 (sp)  0x%08x   r16 (cpsr) 0x%08x\n", r[13], r[16]);
		}
		gxemul_wait_until_prompt(0);
	}
	return 0;
}

int gxemul_close(int fd)
{
	return close(fd);
}

u64 gxemul_lseek(int fildes, u64 offset, int whence)
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

plugin_t gxemul_plugin = {
	.name = "gxemul",
	.desc = "GxEmul Debugger interface ( gxemul://program.arm )",
	.init = NULL,
	.debug = NULL, // XXX
	.system = gxemul_system,
	.handle_fd = gxemul_handle_fd,
	.handle_open = gxemul_handle_open,
	.open = gxemul_open,
	.read = gxemul_read,
	.write = gxemul_write,
	.lseek = gxemul_lseek,
	.close = gxemul_close
};
