/* radare 2008 GPL -- pancake <youterm.com> */

#include "r_types.h"
#include "r_syscall.h"
#include <stdio.h>
#include <string.h>

extern struct syscall_t syscalls_netbsd_x86[];
extern struct syscall_t syscalls_linux_x86[];
extern struct syscall_t syscalls_freebsd_x86[];
extern struct syscall_t syscalls_darwin_x86[];

static struct syscall_t *sysptr = syscalls_linux_x86;

/* TODO: move to default debug_os namespace */
int r_syscall_setup(const char *arch, const char *os)
{
	if (!strcmp(os, "linux"))
		sysptr = syscalls_linux_x86;
	else
	if (!strcmp(os, "netbsd"))
		sysptr = syscalls_netbsd_x86;
	else
	if (!strcmp(os, "openbsd"))
		sysptr = syscalls_netbsd_x86; // XXX
	else
	if (!strcmp(os, "freebsd"))
		sysptr = syscalls_freebsd_x86;
	else
	if (!strcmp(os, "darwin"))
		sysptr = syscalls_darwin_x86;
	else {
		fprintf(stderr, "Unknown/unhandled OS in asm.os for arch_print_syscall()\n");
		return 1;
	}
	return 0;
}

int r_syscall_setup_file(const char *path)
{
	
}

int r_syscall_get(const char *str)
{
	int i;
	for(i=0;sysptr[i].num;i++)
		if (!strcmp(str, sysptr[i].name))
			return sysptr[i].num;

	return 0;
}

struct syscall_t *r_syscall_get_n(int n)
{
	int i;
	for(i=0;sysptr[i].num && i!=n;i++)
		return &sysptr[i];
	return NULL;
}

const char *r_syscall_get_i(int num, int swi)
{
	int i;
	for(i=0;sysptr[i].num;i++)
		if (num == sysptr[i].num && (swi == -1 || swi == sysptr[i].swi))
			return sysptr[i].name;
	return NULL;
}

void r_syscall_list()
{
	int i;
	for(i=0;sysptr[i].num;i++) {
		printf("%02x: %d = %s\n",
			sysptr[i].swi, sysptr[i].num, sysptr[i].name);
	}
}
