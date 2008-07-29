#include "elf.h"

#undef ELF_

#ifdef DIETELF64
    #define ELF_(name) Elf64_##name 
#else       
    #define ELF_(name) Elf32_##name 
#endif      

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


void  ELF_(aux_swap_endian)(u8 *value, int size);
int   ELF_(aux_is_encoded)(int encoding, unsigned char c);
int   ELF_(aux_is_printable)(int c);
int   ELF_(aux_stripstr_iterate)(const unsigned char *buf, int i, int min, int enc, u64 base, u64 offset);
int   ELF_(aux_stripstr_from_file)(const char *filename, int min, int encoding, u64 base, u64 seek, u64 limit);
char* ELF_(aux_filter_rad_output)(const char *string);
int   ELF_(do_elf_checks)(ELF_(dietelf_bin_t) *bin);
u64   ELF_(get_import_addr)(ELF_(dietelf_bin_t) *bin, int fd, int sym);
int   ELF_(load_section)(char **section, int fd, ELF_(Shdr) *shdr);
u64   ELF_(dietelf_get_section_index)(ELF_(dietelf_bin_t) *bin, int fd, const char *section_name);
u64   ELF_(dietelf_get_section_offset)(ELF_(dietelf_bin_t) *bin, int fd, const char *section_name);
int   ELF_(dietelf_get_section_size)(ELF_(dietelf_bin_t) *bin, int fd, const char *section_name);
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
int   ELF_(dietelf_list_exports)(ELF_(dietelf_bin_t) *bin, int fd);
int   ELF_(dietelf_list_others)(ELF_(dietelf_bin_t) *bin, int fd);
int   ELF_(dietelf_list_strings)(ELF_(dietelf_bin_t) *bin, int fd);
int   ELF_(dietelf_open)(ELF_(dietelf_bin_t) *bin, int fd);
int   ELF_(dietelf_new)(ELF_(dietelf_bin_t) *bin, const char *name);
