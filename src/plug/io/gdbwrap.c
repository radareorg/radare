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
#include <socket.h>
#include "libgdbwrap/include/gdbwrapper.h"

static u64 off = 0;
static gdbwrap_t *desc = NULL;
static int _fd = -1;
static int pid = 0;

ssize_t r_gdbwrap_write(int fd, const u8 *buf, size_t count)
{
	gdbwrap_writemem(desc, (la32)off, buf, count);
	return count;
}

ssize_t r_gdbwrap_read(int fd, unsigned char *buf, size_t count)
{
	int i=0;
	if (r_gdbwrap_handle_fd(fd)) {
		unsigned char *ptr = gdbwrap_readmem(desc, (la32)off, count);
		if (ptr == NULL)
			return -1;
		memset(buf, '\0', count);
		hexstr2binstr(ptr, buf); //const char *in, unsigned char *out) // 0A 3B 4E A0
		return count;
	}

        return -1;
}

int r_gdbwrap_handle_fd(int fd)
{
	return (_fd == fd);
}

int r_gdbwrap_open(char *file, int mode, int flags)
{
	char *port;
	if (!r_gdbwrap_handle_open(file)) {
		fprintf(stderr, "Invalid uri\n");
		return -1;
	}
	file = file + 10;
	port = strchr(file,':');
	if (port) {
		port[0]='\0';
		_fd = socket_connect(file, atoi(port + 1));
		desc = NULL;
		if (_fd == -1)
			fprintf(stderr, "Cannot connect\n");
		desc = gdbwrap_init(_fd);
		return _fd;
	} else {
		fprintf(stderr, "No port specified.\n ATM only gdbserver connection is supported\n");
		return -1;
	}
}

int r_gdbwrap_handle_open(const char *file)
{
	if (!strncmp("gdbwrap://", file, 10))
		return 1;
	return 0;
}

int r_gdbwrap_seek(int fd, u64 offset, int whence)
{
	if (r_gdbwrap_handle_fd(fd)) {
		switch(whence) {
		case SEEK_SET:
			return off = offset;
		case SEEK_CUR:
			return off = (u64)((unsigned long long)off+(unsigned long long)offset);
		case SEEK_END:
			return off = (u64)((unsigned long long)(-1));
		}
	}

	return -1;
}

void r_gdbwrap_system(char *str)
{
	char *ptr;
#if 0
	if (!memcmp(str, "vmware",6)) {
		gdbwrap_vmwareinit(desc);
	} else
#endif
	if (!memcmp(str, "step",4)) {
		gdbwrap_stepi(desc);
	} else
	if (!memcmp(str, "cont", 4)) {
		gdbwrap_continue(desc);
	} else
	if (!memcmp(str, "info", 4)) {
		printf("Last signal : %d\n", gdbwrap_lastsignal(desc));
		printf("Active: %d\n", gdbwrap_is_active(desc));
		gdbwrap_reason_halted(desc); // XXX showing info .. where?!?
	} else
	if (!memcmp(str, "help", 4)) {
		printf("gdbwrap help:\n"
		" !vm          ; run vmware initialization stuff (???)\n"
		" !step        ; perform a step into\n"
		" !cont        ; continue\n"
		" !stop        ; stop running process\n"
		" !cr0         ; show value of cr0 register\n"
		" !bp 0x804800 ; add breakpoint\n"
		" !bp-0x804800 ; remove breakpoint\n"
		" !stat        ; show process status (halted why)\n"
		" !info        ; show status of process\n"
		" !pid         ; set or list pid (hacky, only local procs)\n"
		" !maps        ; show process memory maps (hacky, needs !pid)\n"
		" !kill 9      ; send signal to the process\n"
		" !reg eax=33  ; show register values\n"
		" !reg[*]      ; show register values\n");
	} else
	if (!memcmp(str, "pid ", 4)) {
		pid = atoi(str+4);
	} else
	if (!memcmp(str, "pid", 3)) {
		if (pid == 0) {
			eprintf("No pid defined. Use !pid:\n");
			system("cat /proc/*/stat 2>/dev/null | grep ' T ' | cut -d ' ' -f 1,2");
		} else eprintf("%d\n", pid);
	} else
	if (!memcmp(str, "stop", 4)) {
		gdbwrap_ctrl_c(desc);
	} else
	if (!memcmp(str, "stat", 4)) {
		gdbwrap_reason_halted(desc);
	} else
	if (!memcmp(str, "kill ", 5)) {
		gdbwrap_signal(desc, atoi(str+5));
	} else
	if (!memcmp(str, "maps", 4)) {
		char buf[128];
		if (pid == 0) {
			eprintf("No pid defined. Use !pid:\n");
			system("cat /proc/*/stat 2>/dev/null | grep ' T ' | cut -d ' ' -f 1,2");
		} else {
			eprintf("Maps of process: %d\n", pid);
			snprintf(buf, 127, "cat /proc/%d/maps", pid);
			system(buf);
		}
	} else
	if (!memcmp(str, "bp ", 3)) {
		unsigned int addr = get_offset(str+3);
		printf("Add bp\n");
		gdbwrap_simplesetbp(desc, addr);
	} else
	if (!memcmp(str, "bp-", 3)) {
		unsigned int addr = get_offset(str+3);
		printf("Delete bp\n");
		gdbwrap_simpledelbp(desc, addr);
	} else
	if (!memcmp(str, "reg",3)) {
		gdbwrap_gdbreg32 *reg = gdbwrap_readgenreg(desc);
		if (reg == NULL)
			return;
		ptr = strstr(str, "=");
		if (ptr) {
			u32 value;
			ptr[0]='\0';
			memcpy(&desc->reg32, reg, sizeof(reg));
			reg = &desc->reg32;
			value = (u32) get_math(ptr+1);
			if (strstr(str, "eax")) reg->eax = value; else
			if (strstr(str, "ebx")) reg->ebx = value; else
			if (strstr(str, "ecx")) reg->ecx = value; else
			if (strstr(str, "edx")) reg->edx = value; else
			if (strstr(str, "esp")) reg->esp = value; else
			if (strstr(str, "esi")) reg->esi = value; else
			if (strstr(str, "edi")) reg->edi = value; else
			if (strstr(str, "ebp")) reg->ebp = value; else
			if (strstr(str, "eip")) reg->eip = value; else
			if (strstr(str, "eflags")) reg->eflags = value; else
			if (strstr(str, "cs")) reg->cs = value; else
			if (strstr(str, "ds")) reg->ds = value; else
			if (strstr(str, "ss")) reg->ss = value; else
			if (strstr(str, "es")) reg->es = value; else
			if (strstr(str, "fs")) reg->fs = value; else
			if (strstr(str, "gs")) reg->gs = value;
			gdbwrap_shipallreg(desc);
		} else {
			if (strchr(str,'*')) {
				printf("f eip @ 0x%08x\n", reg->eip);
				printf("f eax @ 0x%08x\n", reg->eax);
				printf("f ebx @ 0x%08x\n", reg->ebx);
				printf("f ecx @ 0x%08x\n", reg->ecx);
				printf("f edx @ 0x%08x\n", reg->edx);
				printf("f esi @ 0x%08x\n", reg->esi);
				printf("f edi @ 0x%08x\n", reg->edi);
				printf("f esp @ 0x%08x\n", reg->esp);
				printf("f ebp @ 0x%08x\n", reg->ebp);
				printf("f eflags @ 0x%04x\n", reg->eflags);
			} else {
				printf("eax 0x%08x   esi 0x%08x   eip 0x%08x\n", reg->eax, reg->esi, reg->eip);
				printf("ebx 0x%08x   edi 0x%08x   eflags 0x%04x\n", reg->ebx, reg->edi, reg->eflags);
				printf("ecx 0x%08x   esp 0x%08x  cs=0x%04x  ds=0x%04x\n", reg->ebx, reg->esp, reg->cs, reg->ds);
				printf("edx 0x%08x   ebp 0x%08x  ss=0x%04x  es=0x%04x\n", reg->edx, reg->ebp, reg->ss, reg->es);
			}
		}
	}
}

void r_gdbwrap_init()
{
}

void r_gdbwrap_close()
{
	close(config.fd);
}

plugin_t gdbwrap_plugin = {
	.name = "gdbwrap",
	.desc = "Connect to remote GDB using eresi's gdbwrap (gdbwrap://host:port)",
	.init = r_gdbwrap_init,
	.debug = 1,
	.system = r_gdbwrap_system,
	.handle_fd = r_gdbwrap_handle_fd,
	.handle_open = r_gdbwrap_handle_open,
	.open = r_gdbwrap_open,
	.read = r_gdbwrap_read,
	.write = r_gdbwrap_write,
	.lseek = r_gdbwrap_seek,
	.close = r_gdbwrap_close
};
