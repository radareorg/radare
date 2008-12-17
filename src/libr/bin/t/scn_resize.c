#include <stdio.h>

#include "r_types.h"
#include "r_bin.h"

int main(int argc, char *argv[])
{
	r_bin_obj bin;
	int fd;
	char *file, *section;
	u32 size, new_size, delta;
	u64 baddr, rva;
	
	if (argc != 4) {
		fprintf(stderr, "Usage: %s elf32_file section_name new_size\n", argv[0]);
		return 1;
	}

	file = argv[1];
	section = argv[2];
	new_size = atoi(argv[3]);

	if ((fd = r_bin_open(&bin, file)) == -1) {
		fprintf(stderr, "Cannot open file\n");
		return 1;
	}

	if ((baddr = r_bin_get_baddr(&bin)) == -1) {
		fprintf(stderr, "Cannot get baddr\n");
		return 1;
	}

	if ((rva = r_bin_get_section_rva(&bin, section)) == -1) {
		fprintf(stderr, "Unknown section\n");
		return 1;
	}

	if ((size = r_bin_get_section_size(&bin, section)) == -1) {
		fprintf(stderr, "Unknown section\n");
		return 1;
	}

	delta = new_size - size;
	printf("old size = %d\n", size);
	printf("new size = %d\n", new_size);
	printf("size delta = %d\n", delta);
	printf("section address = 0x%08llx\n", baddr + rva);

	r_bin_close(&bin);

	return 0;
}

