/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

#include "../main.h"
#include "pe.h"

#ifndef _INCLUDE_DIETPE_TYPES_H_
#define _INCLUDE_DIETPE_TYPES_H_

typedef struct {
	u64 rva;
	u64 offset;
} dietpe_entrypoint;

typedef struct {
	u8  name[PE_IMAGE_SIZEOF_SHORT_NAME];
	u64 size;
	u64 vsize;
	u64 rva;
	u64 offset;
	u64 characteristics;
} dietpe_section;

typedef struct {
	u8  name[PE_NAME_LENGTH];
	u64 rva;
	u64 offset;
	u64 hint;
	u64 ordinal;
} dietpe_import;

typedef struct {
	u8  name[PE_NAME_LENGTH];
	u8  forwarder[PE_NAME_LENGTH];
	u64 rva;
	u64 offset;
	u64 ordinal;
} dietpe_export;

typedef struct {
	u64 rva;
	u64 offset;
	u64 size;
	char type;
	char string[PE_STRING_LENGTH];
} dietpe_string;

#endif 
