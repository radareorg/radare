/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

#ifndef _INCLUDE_DIETELF_TYPES_H_
#define _INCLUDE_DIETELF_TYPES_H_

#include "../main.h"
#include "elf.h"

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

typedef struct {
	u64 offset;
	u64 vaddr;
	char name[ELF_NAME_LENGTH];
} dietelf_field;

#endif
