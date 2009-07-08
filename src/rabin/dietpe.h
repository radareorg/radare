/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

#include "pe.h"
#include "dietpe_types.h"

#ifndef _INCLUDE_DIETPE_H_
#define _INCLUDE_DIETPE_H_

#define PE_SCN_IS_SHAREABLE(x)       x & PE_IMAGE_SCN_MEM_SHARED
#define PE_SCN_IS_EXECUTABLE(x)      x & PE_IMAGE_SCN_MEM_EXECUTE
#define PE_SCN_IS_READABLE(x)        x & PE_IMAGE_SCN_MEM_READ
#define PE_SCN_IS_WRITABLE(x)        x & PE_IMAGE_SCN_MEM_WRITE

#endif

typedef struct {
	PE_(image_dos_header)             *dos_header;
	PE_(image_nt_headers)			  *nt_headers;
	PE_(image_section_header)         *section_header;
	PE_(image_export_directory)       *export_directory;
	PE_(image_import_directory)       *import_directory;
	PE_(image_delay_import_directory) *delay_import_directory;
    const char* file;
	int fd;
} PE_(dietpe_obj);

int PE_(dietpe_close)(PE_(dietpe_obj) *bin);
int PE_(dietpe_get_arch)(PE_(dietpe_obj) *bin, char *str);
int PE_(dietpe_get_entrypoint)(PE_(dietpe_obj) *bin, dietpe_entrypoint *entrypoint);
int PE_(dietpe_get_exports)(PE_(dietpe_obj) *bin, dietpe_export *export);
int PE_(dietpe_get_exports_count)(PE_(dietpe_obj) *bin);
int PE_(dietpe_get_file_alignment)(PE_(dietpe_obj) *bin);
ut64 PE_(dietpe_get_image_base)(PE_(dietpe_obj) *bin);
int PE_(dietpe_get_imports)(PE_(dietpe_obj) *bin, dietpe_import *import);
int PE_(dietpe_get_imports_count)(PE_(dietpe_obj) *bin);
int PE_(dietpe_get_libs)(PE_(dietpe_obj) *bin, int limit, dietpe_string *strings);
int PE_(dietpe_get_image_size)(PE_(dietpe_obj) *bin);
int PE_(dietpe_get_machine)(PE_(dietpe_obj) *bin, char *str);
int PE_(dietpe_get_os)(PE_(dietpe_obj) *bin, char *str);
int PE_(dietpe_get_class)(PE_(dietpe_obj) *bin, char *str);
int PE_(dietpe_get_section_alignment)(PE_(dietpe_obj) *bin);
int PE_(dietpe_get_sections)(PE_(dietpe_obj) *bin, dietpe_section *section);
int PE_(dietpe_get_sections_count)(PE_(dietpe_obj) *bin);
int PE_(dietpe_get_strings)(PE_(dietpe_obj) *bin, int verbose, int str_limit, dietpe_string *strings);
int PE_(dietpe_get_subsystem)(PE_(dietpe_obj) *bin, char *str);
int PE_(dietpe_is_dll)(PE_(dietpe_obj) *bin);
int PE_(dietpe_is_big_endian)(PE_(dietpe_obj) *bin);
int PE_(dietpe_is_stripped_relocs)(PE_(dietpe_obj) *bin);
int PE_(dietpe_is_stripped_line_nums)(PE_(dietpe_obj) *bin);
int PE_(dietpe_is_stripped_local_syms)(PE_(dietpe_obj) *bin);
int PE_(dietpe_is_stripped_debug)(PE_(dietpe_obj) *bin);
int PE_(dietpe_open)(PE_(dietpe_obj) *bin, const char *file);
