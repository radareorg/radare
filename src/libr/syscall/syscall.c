/* radare 2008 GPL -- pancake <youterm.com> */

#include "r_types.h"
#include "r_syscall.h"
#include <stdio.h>
#include <string.h>

extern struct r_syscall_list_t syscalls_netbsd_x86[];
extern struct r_syscall_list_t syscalls_linux_x86[];
extern struct r_syscall_list_t syscalls_freebsd_x86[];
extern struct r_syscall_list_t syscalls_darwin_x86[];

// XXX move into r_r_syscall_list_t
/* TODO: move to default debug_os namespace */
int r_syscall_setup(struct r_syscall_t *ctx, const char *arch, const char *os)
{
	if (!strcmp(os, "linux"))
		ctx->sysptr = syscalls_linux_x86;
	else
	if (!strcmp(os, "netbsd"))
		ctx->sysptr = syscalls_netbsd_x86;
	else
	if (!strcmp(os, "openbsd"))
		ctx->sysptr = syscalls_netbsd_x86; // XXX
	else
	if (!strcmp(os, "freebsd"))
		ctx->sysptr = syscalls_freebsd_x86;
	else
	if (!strcmp(os, "darwin"))
		ctx->sysptr = syscalls_darwin_x86;
	else {
		fprintf(stderr, "Unknown/unhandled OS in asm.os for arch_print_syscall()\n");
		return 1;
	}
	return 0;
}

int r_syscall_setup_file(struct r_syscall_t *ctx, const char *path)
{
	// TODO
	return 0;
}

int r_syscall_get(struct r_syscall_t *ctx, const char *str)
{
	int i;
	for(i=0;ctx->sysptr[i].num;i++)
		if (!strcmp(str, ctx->sysptr[i].name))
			return ctx->sysptr[i].num;
	return 0;
}

struct r_syscall_list_t *r_syscall_get_n(struct r_syscall_t *ctx, int n)
{
	int i;
	for(i=0;ctx->sysptr[i].num && i!=n;i++)
		return &ctx->sysptr[i];
	return NULL;
}

const char *r_syscall_get_i(struct r_syscall_t *ctx, int num, int swi)
{
	int i;
	for(i=0;ctx->sysptr[i].num;i++)
		if (num == ctx->sysptr[i].num && (swi == -1 || swi == ctx->sysptr[i].swi))
			return ctx->sysptr[i].name;
	return NULL;
}

void r_syscall_list(struct r_syscall_t *ctx)
{
	int i;
	for(i=0;ctx->sysptr[i].num;i++) {
		printf("%02x: %d = %s\n",
			ctx->sysptr[i].swi, ctx->sysptr[i].num, ctx->sysptr[i].name);
	}
}
