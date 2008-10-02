#ifndef _INCLUDE_DIETPE_TYPES_H_
#define _INCLUDE_DIETPE_TYPES_H_

#include "pe.h"

typedef struct {
	PE_Byte  name[PE_IMAGE_SIZEOF_SHORT_NAME];
	PE_DWord size;
	PE_DWord offset;
	PE_DWord characteristics;
} dietpe_section;

typedef struct {
	PE_Byte  name[PE_NAME_LENGTH];
	PE_DWord offset;
	PE_DWord ilt_offset;
	PE_Word hint;
	PE_Word ordinal;
} dietpe_import;

typedef struct {
	PE_Byte  name[PE_NAME_LENGTH];
	PE_Byte  forwarder[PE_NAME_LENGTH];
	PE_DWord offset;
	int      ordinal;
} dietpe_export;

#endif
