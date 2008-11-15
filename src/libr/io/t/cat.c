#include "r_io.h"

int main(int argc, char **argv)
{
	char buf[4096];
	int fd;

	r_io_init();

	fd = r_io_open(argc>1?argv[1]:"/etc/issue", R_IO_READ, 0);
	memset(buf, '\0', 4096);
	r_io_read(fd, buf, 4095);

	puts(buf);

	return r_io_close(fd);
}
