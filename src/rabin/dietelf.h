
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
} dietelf_bin_t;

void do_elf_checks(Elf32_Ehdr *ehdr);
int  dietelf_get_base_addr(Elf32_Phdr *phdr);
u64  get_syms_addr(int fd, const char *string, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr, int sym);
void dietelf_list_sections(int fd, const char *string, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr);
void dietelf_list_imports(int fd, const char *bstring, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr);
void dietelf_open(int fd, dietelf_bin_t *bin);
void load_section(char **section, int fd, Elf32_Shdr *shdr);
int dietelf_new(const char *name, dietelf_bin_t *bin);

#endif
