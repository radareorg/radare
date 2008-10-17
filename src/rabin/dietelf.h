/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

#include "../main.h"
#include "elf.h"

#undef ELF_
	
#ifdef DIETELF64
    #define ELF_(name) Elf64_##name 
#else       
    #define ELF_(name) Elf32_##name 
#endif      

#define ELF_ADDR_MASK 0xffffffffffff8000LL

typedef struct {
    ELF_(Ehdr)    ehdr;
    ELF_(Phdr)*   phdr;
    ELF_(Shdr)*   shdr;
    int		  plen;
    char**	  section;
    char*	  string;
    int		  bss;
    u64		  base_addr;
    const char*	  file;
} ELF_(dietelf_bin_t);


u64   ELF_(dietelf_get_section_index)(ELF_(dietelf_bin_t) *bin, int fd, const char *section_name);
u64   ELF_(dietelf_get_section_offset)(ELF_(dietelf_bin_t) *bin, int fd, const char *section_name);
int   ELF_(dietelf_get_section_size)(ELF_(dietelf_bin_t) *bin, int fd, const char *section_name);
int   ELF_(dietelf_is_big_endian)(ELF_(dietelf_bin_t) *bin);
int   ELF_(dietelf_get_arch)(ELF_(dietelf_bin_t) *bin);
u64   ELF_(dietelf_get_entry_addr)(ELF_(dietelf_bin_t) *bin);
u64   ELF_(dietelf_get_base_addr)(ELF_(dietelf_bin_t) *bin);
int   ELF_(dietelf_get_stripped)(ELF_(dietelf_bin_t) *bin);
int   ELF_(dietelf_get_static)(ELF_(dietelf_bin_t) *bin);
char* ELF_(dietelf_get_data_encoding)(ELF_(dietelf_bin_t) *bin);
char* ELF_(dietelf_get_machine_name)(ELF_(dietelf_bin_t) *bin);
char* ELF_(dietelf_get_file_type)(ELF_(dietelf_bin_t) *bin);
char* ELF_(dietelf_get_elf_class)(ELF_(dietelf_bin_t) *bin);
char* ELF_(dietelf_get_osabi_name)(ELF_(dietelf_bin_t) *bin);
int   ELF_(dietelf_list_sections)(ELF_(dietelf_bin_t) *bin, int fd);
int   ELF_(dietelf_list_imports)(ELF_(dietelf_bin_t) *bin, int fd);
int   ELF_(dietelf_list_symbols)(ELF_(dietelf_bin_t) *bin, int fd);
int   ELF_(dietelf_list_strings)(ELF_(dietelf_bin_t) *bin, int fd);
int   ELF_(dietelf_list_libs)(ELF_(dietelf_bin_t) *bin, int fd);
int   ELF_(dietelf_open)(ELF_(dietelf_bin_t) *bin, int fd);
int   ELF_(dietelf_new)(ELF_(dietelf_bin_t) *bin, const char *name);
