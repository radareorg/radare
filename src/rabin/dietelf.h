
#ifndef _INCLUDE_DIETELF_H_
#define _INCLUDE_DIETELF_H_

#include <elf.h>

typedef struct {
    Elf32_Ehdr      ehdr;
    Elf32_Phdr*     phdr;
    Elf32_Shdr*     shdr;
    int		    plen;
    char**	    section;
    char*	    string;
    int		    bss;
    unsigned long long base_addr;
    const char*	    file;
} dietelf_bin_t;

int   aux_is_encoded(int encoding, unsigned char c);
int   aux_is_printable (int c);
int   aux_stripstr_iterate(const unsigned char *buf, int i, int min, int enc, u64 offset);
int   aux_stripstr_from_file(const char *filename, int min, int encoding, u64 seek, u64 limit);
char* aux_filter_rad_output(const char *string);
void  do_elf_checks(dietelf_bin_t *bin);
u64   get_import_addr(int fd, dietelf_bin_t *bin, int sym);
void  load_section(char **section, int fd, Elf32_Shdr *shdr);
u64   dietelf_get_section_offset(int fd, dietelf_bin_t *bin, const char *section_name);
int   dietelf_get_section_size(int fd, dietelf_bin_t *bin, const char *section_name);
u64   dietelf_get_entry_addr(dietelf_bin_t *bin);
u64   dietelf_get_base_addr(dietelf_bin_t *bin);
int   dietelf_list_sections(int fd, dietelf_bin_t *bin);
int   dietelf_list_imports(int fd, dietelf_bin_t *bin);
int   dietelf_list_exports(int fd, dietelf_bin_t *bin);
int   dietelf_list_others(int fd, dietelf_bin_t *bin);
int   dietelf_list_strings(int fd, dietelf_bin_t *bin);
void  dietelf_open(int fd, dietelf_bin_t *bin);
int   dietelf_new(const char *name, dietelf_bin_t *bin);

#endif
