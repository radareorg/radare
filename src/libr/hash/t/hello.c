#include <stdio.h>
#include <stdlib.h>
#include "r_io.h"
#include "r_hash.h"

int main(int argc, char **argv)
{
	int fd;
	u8 *buf;
	u64 size;

	if (argc<2) {
		printf("Usage: %s [file]\n", argv[0]);
		return 1;
	}

	r_io_init();

	fd = r_io_open(argv[1], R_IO_READ, 0);
	if (fd == -1) {
		printf("Cannot open file\n");
		return 1;
	}

	/* get file size */
	r_io_lseek(fd, 0, R_IO_SEEK_END);
	size = r_io_lseek(fd, 0, R_IO_SEEK_END);
	r_io_lseek(fd, 0, R_IO_SEEK_SET);

	/* read bytes */
	buf = (u8*) malloc(size);
	if (buf == NULL) {
		printf("Too big file\n");
		return 1;
		r_io_close(fd);
	}

	r_io_read(fd, buf, size);
	printf("----\n%s\n----\n", buf);

	printf("file size = %lld\n", size);
	printf("CRC32: 0x%08x\n", r_hash_crc32(buf, size));

	r_io_close(fd);
	free (buf);
	return 0;
}
