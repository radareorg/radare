#include <stdio.h>

extern unsigned long len_shcode;
extern char process_syscall;

int main(int argc, char **argv)
{
	char *l = &process_syscall + len_shcode;
	char *p = &process_syscall;

	for(; p != l; p++)
		printf("%c", (unsigned char)*p);

	return 0;
}
