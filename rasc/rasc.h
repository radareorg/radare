#include <stdio.h>

struct shellcode_t {
	char *name;
	char *desc;
	unsigned char *data;
	int len;
	/* flagz */
	int cmd;
	int host;
	int port;
};

extern struct shellcode_t shellcodes[];
void process_syscall();
int test();
