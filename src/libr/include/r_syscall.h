#ifndef _INCLUDE_R_SYSCALL_H_
#define _INCLUDE_R_SYSCALL_H_

#include "r_types.h"

struct r_syscall_list_t {
	const char *name;
	int swi;
	int num;
	int args;
};

// TODO: use this as arg to store state :)
struct r_syscall_t {
	int arch; // XXX char *??
	int os;
	struct syscall_list_t *sysptr;
};

int r_syscall_setup(const char *arch, const char *os);
int r_syscall_setup_file(const char *path);
int r_syscall_get(const char *str);
struct r_syscall_list_t *r_syscall_get_n(int n);
const char *r_syscall_get_i(int num, int swi);
void r_syscall_list();

#endif

