#include <stdio.h>

#include "r_types.h"
#include "r_asm.h"
#include "r_bin.h"
#include "r_util.h"

int main(int argc, char *argv[])
{
	r_bin_obj bin;
	struct r_asm_t a;
	struct r_asm_realloc_t r;
	r_bin_section *sections;
	char *file, *section;
	u8 *buf;
	u64 size = 0, idx = 0, len = 0;
	u32 ret = 0, i = 0;
	
	if (argc != 4) {
		fprintf(stderr, "Usage: %s elf_file section_name new_size\n", argv[0]);
		return 1;
	}

	file = argv[1];
	section = argv[2];
	size = r_num_math(NULL, argv[3]);

	if (r_bin_open(&bin, file, 1) == -1) {
		fprintf(stderr, "Cannot open file\n");
		return 1;
	}

	/* Resize sections */
	if ((r.delta = r_bin_resize_section(&bin, section, size)) == 0) {
		fprintf(stderr, "Delta = 0\n");
		return 1;
	}

	r.offset = r_bin_get_section_rva(&bin, section) + r_bin_get_baddr(&bin);
	sections = r_bin_get_sections(&bin);

	/* Parse executable sections */
	r_asm_init(&a);
	r_asm_set_arch(&a, R_ASM_ARCH_X86);
	r_asm_set_bits(&a, 32);
	r_asm_set_big_endian(&a, FALSE);
	r_asm_set_syntax(&a, R_ASM_SYN_INTEL);

	for (i=0; !sections[i].last; i++)
		if (R_BIN_SCN_EXECUTABLE(sections[i].characteristics)) {
			if ((buf = malloc(sections[i].size)) == NULL){
				fprintf(stderr, "Malloc error\n");
				return 1;
			}
			if (lseek(bin.fd, sections[i].offset, SEEK_SET) < 0) {
				perror("lseek");
				return 1;
			}
			if (read(bin.fd, buf, sections[i].size) != sections[i].size) {
				perror("read");
				return 1;
			}
			idx = 0; len = sections[i].size;
			while (idx < len) {
				r_asm_set_pc(&a, sections[i].rva + idx);

				ret = r_asm_disasm(&a, buf+idx, len-idx);
				r_asm_set_parser(&a, R_ASM_PAR_REALLOC, &r);
				r_asm_parse(&a);
				printf("0x%llx... %s -> %s\n", r.offset, a.buf_asm, a.buf_par);

				idx += ret;
			}
			free(buf);
		}

	r_bin_close(&bin);

	return 0;
}
