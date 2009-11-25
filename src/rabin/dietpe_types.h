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
	ut64 rva;
	ut64 offset;
} dietpe_entrypoint;

typedef struct {
	u8  name[PE_IMAGE_SIZEOF_SHORT_NAME];
	ut64 size;
	ut64 vsize;
	ut64 rva;
	ut64 offset;
	ut64 characteristics;
} dietpe_section;

typedef struct {
	u8  name[PE_NAME_LENGTH];
	ut64 rva;
	ut64 offset;
	ut64 hint;
	ut64 ordinal;
} dietpe_import;

typedef struct {
	u8  name[PE_NAME_LENGTH];
	u8  forwarder[PE_NAME_LENGTH];
	ut64 rva;
	ut64 offset;
	ut64 ordinal;
} dietpe_export;

typedef struct {
	ut64 rva;
	ut64 offset;
	ut64 size;
	char type;
	char section[PE_STRING_LENGTH];
	char string[PE_STRING_LENGTH];
} dietpe_string;

#endif 
