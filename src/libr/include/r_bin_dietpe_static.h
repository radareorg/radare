/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

#ifndef _INCLUDE_DIETPE_STATIC_H_
#define _INCLUDE_DIETPE_STATIC_H_

static PE_DWord dietpe_aux_rva_to_offset(dietpe_bin*, PE_DWord);
static PE_DWord dietpe_aux_offset_to_rva(dietpe_bin*, PE_DWord);
static int dietpe_aux_is_encoded(int encoding, unsigned char c);
static int dietpe_aux_is_printable(int c);
static int dietpe_aux_stripstr_from_file(dietpe_bin*, int, int, PE_DWord, PE_DWord, const char*, int, dietpe_string *strings);
static int dietpe_do_checks(dietpe_bin*);
static int dietpe_get_delay_import_dirs_count(dietpe_bin*);
static int dietpe_get_import_dirs_count(dietpe_bin*);
static int dietpe_init(dietpe_bin*);
static int dietpe_init_exports(dietpe_bin*);
static int dietpe_init_imports(dietpe_bin*);
static int dietpe_parse_imports(dietpe_bin*, dietpe_import**, char*, PE_DWord, PE_DWord);

#endif
