#ifndef _INCLUDE_MACROS_H_
#define _INCLUDE_MACROS_H_

#include "list.h"


extern int macro_counter;
extern u64 *macro_break_value;

void radare_macro_init();
int radare_macro_add(const char *name);
int radare_macro_rm(const char *_name);
int radare_macro_list();
int radare_macro_call(const char *name);
int radare_macro_break(const char *value);

#endif
