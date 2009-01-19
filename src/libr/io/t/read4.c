#include <r_io.h>

int main(int argc, char **argv)
{
	char buf[4];
	int fd;

	r_io_init();

	fd = r_io_open(argc>1?argv[1]:"/etc/issue", R_IO_READ, 0);
	r_io_lseek(fd, 1, R_IO_SEEK_SET);
	memset(buf, '\0', 4);
	r_io_read(fd, buf, 4);
	buf[3]='\0';

	puts(buf);

	return r_io_close(fd);
}
