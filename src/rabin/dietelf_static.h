/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

#include "../main.h"
#include "elf.h"


static void  ELF_(aux_swap_endian)(u8 *value, int size);
static int   ELF_(aux_is_encoded)(int encoding, unsigned char c);
static int   ELF_(aux_is_printable)(int c);
static int   ELF_(aux_stripstr_iterate)(const unsigned char *buf, int i, int min, int enc, u64 base, u64 offset, const char *filter, int *cont);
static int   ELF_(aux_stripstr_from_file)(const char *filename, int min, int encoding, u64 base, u64 seek, u64 limit, const char *filter, int *cont);
static char* ELF_(aux_filter_rad_output)(const char *string);
static int   ELF_(do_elf_checks)(ELF_(dietelf_bin_t) *bin);
static u64   ELF_(get_import_addr)(ELF_(dietelf_bin_t) *bin, int fd, int sym);
static int   ELF_(load_section)(char **section, int fd, ELF_(Shdr) *shdr);
