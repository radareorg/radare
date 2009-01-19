#include "r_core.h"
#include "r_io.h"
#include <stdio.h>

struct r_core_t r;

int main(int argc, char **argv)
{
	struct r_core_file_t *fh;

	if (argc<2) {
		printf("Usage: radare2 [file]\n");
		return 1;
	}
	r_core_init(&r);


	fh = r_core_file_open(&r, argv[1], R_IO_READ);
	if (fh == NULL) {
		fprintf(stderr, "Cannot open file '%s'\n", argv[1]);
		return 1;
	}

	if (0) {
		const char *str = r_core_cmd_str(&r, "p8 4 @ 0x1");
		printf("==(%s)==\n", str);
		free((void *)str);
	}

	while(r_core_prompt(&r) != -1);

	return r_core_file_close(&r, fh);
}
