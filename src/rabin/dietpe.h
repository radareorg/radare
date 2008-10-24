/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

#ifndef _INCLUDE_DIETPE_H_
#define _INCLUDE_DIETPE_H_

#include "dietpe_types.h"

#define PE_SCN_IS_SHAREABLE(x)       x & PE_IMAGE_SCN_MEM_SHARED
#define PE_SCN_IS_EXECUTABLE(x)      x & PE_IMAGE_SCN_MEM_EXECUTE
#define PE_SCN_IS_READABLE(x)        x & PE_IMAGE_SCN_MEM_READ
#define PE_SCN_IS_WRITABLE(x)        x & PE_IMAGE_SCN_MEM_WRITE

int dietpe_close(int);
int dietpe_get_arch(dietpe_bin*, char*);
int dietpe_get_class(dietpe_bin*, char*);
int dietpe_get_entrypoint(dietpe_bin*, dietpe_entrypoint*);
int dietpe_get_exports(dietpe_bin*, int, dietpe_export*);
int dietpe_get_exports_count(dietpe_bin*, int);
int dietpe_get_file_alignment(dietpe_bin*);
PE_DWord dietpe_get_image_base(dietpe_bin*);
int dietpe_get_image_size(dietpe_bin*);
int dietpe_get_imports(dietpe_bin*, int, dietpe_import*);
int dietpe_get_imports_count(dietpe_bin*, int);
int dietpe_get_libs(dietpe_bin*, int, int, dietpe_string*);
int dietpe_get_machine(dietpe_bin*, char*);
int dietpe_get_os(dietpe_bin*, char*);
int dietpe_get_section_alignment(dietpe_bin*);
int dietpe_get_sections(dietpe_bin*, dietpe_section*);
int dietpe_get_sections_count(dietpe_bin*);
int dietpe_get_subsystem(dietpe_bin*, char*);
int dietpe_is_dll(dietpe_bin*);
int dietpe_is_big_endian(dietpe_bin*);
int dietpe_is_stripped_relocs(dietpe_bin*);
int dietpe_is_stripped_line_nums(dietpe_bin*);
int dietpe_is_stripped_local_syms(dietpe_bin*);
int dietpe_is_stripped_debug(dietpe_bin*);
int dietpe_open(dietpe_bin*, const char*);

#endif
