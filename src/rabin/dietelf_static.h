/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

#include "../main.h"
#include "dietelf_types.h"
#include "elf.h"


static int   ELF_(aux_is_encoded)(int, unsigned char);
static int   ELF_(aux_is_printable)(int);
static int   ELF_(aux_stripstr_from_file)(const char*, int, int, u64, u64, u64, const char*, int*);
static int   ELF_(aux_stripstr_iterate)(const unsigned char*, int, int, int, u64, u64, const char*, int*);
static void  ELF_(aux_swap_endian)(u8*, int);
static int   ELF_(dietelf_init)(ELF_(dietelf_bin_t)*, int);
static int   ELF_(do_elf_checks)(ELF_(dietelf_bin_t)*);
static u64   ELF_(get_import_addr)(ELF_(dietelf_bin_t)*, int, int);
static int   ELF_(load_section)(char**, int, ELF_(Shdr)*);
