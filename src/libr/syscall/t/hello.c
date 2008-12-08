#include <stdio.h>
#include "r_syscall.h"

int main()
{
	char buf[1024];

	r_syscall_setup("x86", "linux");

	printf("4 = %s\n", r_syscall_get_i(4, -1));
	printf("write = %d\n", r_syscall_get("write"));

	return 0;
}
