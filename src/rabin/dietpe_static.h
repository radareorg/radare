/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

#include "pe.h"
#include "dietpe_types.h"

static int PE_(dietpe_aux_stripstr_from_file)(PE_(dietpe_obj) *bin, int min, int encoding, ut64 seek, ut64 limit, const char *filter, int str_limit, dietpe_string *strings);
static PE_DWord PE_(dietpe_aux_rva_to_offset)(PE_(dietpe_obj) *bin, PE_DWord rva);
static PE_DWord PE_(dietpe_aux_offset_to_rva)(PE_(dietpe_obj) *bin, PE_DWord offset);
static int PE_(dietpe_do_checks)(PE_(dietpe_obj) *bin);
static int PE_(dietpe_init)(PE_(dietpe_obj) *bin);
static int PE_(dietpe_init_exports)(PE_(dietpe_obj) *bin);
static int PE_(dietpe_init_imports)(PE_(dietpe_obj) *bin);
static int PE_(dietpe_get_import_dirs_count)(PE_(dietpe_obj) *bin);
static int PE_(dietpe_parse_imports)(PE_(dietpe_obj) *bin, dietpe_import **importp, char *dll_name, PE_DWord OriginalFirstThunk, PE_DWord FirstThunk);
