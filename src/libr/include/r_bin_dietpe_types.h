/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

#ifndef _INCLUDE_DIETPE_TYPES_H_
#define _INCLUDE_DIETPE_TYPES_H_

typedef struct {
	pe_image_dos_header             *dos_header;
	pe_image_nt_headers			    *nt_headers;
	pe_image_section_header         *section_header;
	pe_image_export_directory       *export_directory;
	pe_image_import_directory       *import_directory;
	pe_image_delay_import_directory *delay_import_directory;
    const char* file;
	int fd;
} dietpe_bin;

typedef struct {
	PE_DWord rva;
	PE_DWord offset;
} dietpe_entrypoint;

typedef struct {
	PE_Byte  name[PE_IMAGE_SIZEOF_SHORT_NAME];
	PE_DWord size;
	PE_DWord vsize;
	PE_DWord rva;
	PE_DWord offset;
	PE_DWord characteristics;
} dietpe_section;

typedef struct {
	PE_Byte  name[PE_NAME_LENGTH];
	PE_DWord rva;
	PE_DWord offset;
	PE_Word hint;
	PE_Word ordinal;
} dietpe_import;

typedef struct {
	PE_Byte  name[PE_NAME_LENGTH];
	PE_Byte  forwarder[PE_NAME_LENGTH];
	PE_DWord rva;
	PE_DWord offset;
	int      ordinal;
} dietpe_export;

typedef struct {
	PE_DWord rva;
	PE_DWord offset;
	PE_DWord size;
	char     type;
	char     string[PE_STRING_LENGTH];
} dietpe_string;

#endif
