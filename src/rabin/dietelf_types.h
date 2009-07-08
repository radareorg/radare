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
	ut64 offset;
	ut64 size;
	ut64 align;
	ut32 flags;
	char name[ELF_NAME_LENGTH];
} dietelf_section;

typedef struct {
	ut64 offset;
	char bind[ELF_NAME_LENGTH];
	char type[ELF_NAME_LENGTH];
	char name[ELF_NAME_LENGTH];
} dietelf_import;

typedef struct {
	ut64 offset;
	ut64 size;
	char bind[ELF_NAME_LENGTH];
	char type[ELF_NAME_LENGTH];
	char name[ELF_NAME_LENGTH];
} dietelf_symbol;

typedef struct {
	ut64 offset;
	ut64 size;
	char type;
	char string[ELF_STRING_LENGTH];
} dietelf_string;

typedef struct {
	ut64 offset;
	ut64 vaddr;
	ut64 size;
	ut64 end;
	int type;
	int flags;
	char name[ELF_NAME_LENGTH];
} dietelf_field;

#endif
