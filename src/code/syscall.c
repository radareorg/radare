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

#include <sys/syscall.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#if __x86_64__
#define ARCH_X86_64 1
#endif

#if 0
// TODO: use it!
int arch_print_syscall_mips64()
{
	unsigned int sc;
	int i,j;

	/* read 4 previous bytes to ARM_pc and get syscall number from there */
	debug_read_at(ps.tid, &sc, 4, arch_pc()-4);
	sc<<=8; // drop opcode
	for(i=0;syscalls_linux_mips64[i].num;i++) {
		if (sc == 0x900000 + syscalls_linux_mips64[i].num) {
			printf("%s ( ", syscalls_linux_mips64[i].name);
			j = syscalls_linux_mips64[i].args;
			/*
			if (j>0) printf("0x%08x ", R_EBX(regs));
			if (j>1) printf("0x%08x ", R_ECX(regs));
			if (j>2) printf("0x%08x ", R_EDX(regs));
			if (j>3) printf("0x%08x ", R_ESI(regs));
			if (j>4) printf("0x%08x ", R_EDI(regs));
			*/
			break;
		}
	}
	return sc-0x900000;
}
#endif


/* TODO: give me the arch too! */
struct syscall_t *syscall_by_os(const char *os, const char *arch)
{
	struct syscall_t *sysptr;

	if (!strcmp(os, "linux"))
		sysptr = &syscalls_linux_x86;
	else
	if (!strcmp(os, "netbsd"))
		sysptr = &syscalls_netbsd_x86;
	else
	if (!strcmp(os, "openbsd"))
		sysptr = &syscalls_netbsd_x86; // XXX
	else
	if (!strcmp(os, "freebsd"))
		sysptr = &syscalls_freebsd_x86;
	else
	if (!strcmp(os, "darwin"))
		sysptr = &syscalls_darwin_x86;
	else {
		eprintf("Unknown/unhandled OS in asm.os for arch_print_syscall()\n");
		return -1;
	}
	return sysptr;
}

int syscall_name_to_int(const char *str)
{
	int i;
	struct syscall_t *sysptr = syscall_by_os(config_get("asm.os"));

	for(i=0;sysptr[i].num;i++)
		if (!strcmp(str, sysptr[i].name))
			return sysptr[i].num;

	return 0;
}

void debug_os_syscall_list()
{
	int i;
	struct syscall_t *sysptr = syscall_by_os(config_get("asm.os"));

	for(i=0;sysptr[i].num;i++) {
		cons_printf("%d = %s\n",
			sysptr[i].num,
			sysptr[i].name);
	}
}

int arch_print_syscall()
{
#if __ARM__
	return arch_arm_print_syscall();
#elif __linux__ || __NetBSD__ || __FreeBSD__ || __OpenBSD__
	int i,j,ret;
	regs_t regs;
	struct syscall_t *sysptr;

	sysptr = syscall_by_os(config_get("asm.os"));

	ret = debug_getregs(ps.tid, &regs);
	if (ret < 0) {
		perror("getregs");
		return -1;
	}
#if __x86_64__
	cons_printf("0x%08llx syscall(%d) ", (u64)R_RIP(regs), (int)R_ORAX(regs));
	
	for(i=0;sysptr[i].num;i++) {
		if(R_ORAX(regs) == sysptr[i].num) {
			cons_printf("%s ( ", sysptr[i].name);
			j = sysptr[i].args;
			if (j>0) cons_printf("0x%08llx ", (u64) R_RBX(regs));
			if (j>1) cons_printf("0x%08llx ", (u64) R_RCX(regs));
			if (j>2) cons_printf("0x%08llx ", (u64) R_RDX(regs));
			if (j>3) cons_printf("0x%08llx ", (u64) R_RSI(regs));
			if (j>4) cons_printf("0x%08llx ", (u64) R_RDI(regs));
			break;
		}
	}

	cons_printf(") = 0x%08llx\n", R_RAX(regs));
	return (int)R_ORAX(regs);
#elif __i386__
	cons_printf("0x%08x syscall(%d) ", R_EIP(regs), R_OEAX(regs), R_EAX(regs));

	for(i=0;sysptr[i].num;i++) {
		if (R_OEAX(regs) == sysptr[i].num) {
			cons_printf("%s ( ", sysptr[i].name);
			j = sysptr[i].args;
			if (j>0) cons_printf("0x%08x ", R_EBX(regs));
			if (j>1) cons_printf("0x%08x ", R_ECX(regs));
			if (j>2) cons_printf("0x%08x ", R_EDX(regs));
			if (j>3) cons_printf("0x%08x ", R_ESI(regs));
			if (j>4) cons_printf("0x%08x ", R_EDI(regs));
			break;
		}
	}
	cons_printf(") = 0x%08x\n", R_EAX(regs));
	return (int)R_OEAX(regs);
#else
#warning Unknown arch for arch_print_syscall
	eprintf("arch_print_syscall: not implemented for this platform\n");
#endif
#endif
	return -1;
}
