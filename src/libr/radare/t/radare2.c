#include "r_radare.h"

struct r_radare_t r;

int main(int argc, char **argv)
{
	int fd;

	if (argc<2) {
		printf("Usage: radare2 [file]\n");
		return 1;
	}
	r_radare_init(&r);

	fd = r_radare_file_open(&r, argv[1], R_IO);
	if (fd == NULL

	return 0;
}
