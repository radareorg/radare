/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

typedef struct {
    ELF_(Ehdr)  ehdr;
    ELF_(Phdr)* phdr;
    ELF_(Shdr)* shdr;
    int         plen;
    char**      section;
    char*       string;
    int         bss;
    u64         base_addr;
    const char* file;
	int			fd;
} ELF_(dietelf_bin_t);

#ifndef _DIETELF_TYPES_H_
#define _DIETELF_TYPES_H_

typedef struct {
	u64 offset;
	u64 size;
	u64 align;
	u32 flags;
	char name[ELF_NAME_LENGTH];
} dietelf_section;

typedef struct {
	u64 offset;
	char bind[ELF_NAME_LENGTH];
	char type[ELF_NAME_LENGTH];
	char name[ELF_NAME_LENGTH];
} dietelf_import;

typedef struct {
	u64 offset;
	u64 size;
	char bind[ELF_NAME_LENGTH];
	char type[ELF_NAME_LENGTH];
	char name[ELF_NAME_LENGTH];
} dietelf_symbol;

typedef struct {
	u64 offset;
	u64 size;
	char type;
	char string[ELF_STRING_LENGTH];
} dietelf_string;

#endif
