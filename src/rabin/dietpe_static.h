/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

#ifndef _INCLUDE_DIETPE_STATIC_H_
#define _INCLUDE_DIETPE_STATIC_H_

#include "dietpe_types.h"

static PE_DWord dietpe_aux_rva_to_offset(dietpe_bin*, PE_DWord);
static int dietpe_get_import_dirs_count(dietpe_bin*);
static int dietpe_init(dietpe_bin*, int);
static int dietpe_init_exports(dietpe_bin*, int);
static int dietpe_init_imports(dietpe_bin*, int);

#endif
