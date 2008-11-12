#ifndef _INCLUDE_MACROS_H_
#define _INCLUDE_MACROS_H_

#include "list.h"

/* MACROS */
struct macro_t {
	char *name;
	char *code;
	int nargs;
	struct list_head list;
};

void radare_macro_init();
int radare_macro_add(const char *name);
int radare_macro_rm(const char *_name);
int radare_macro_list();
int radare_cmd_args(const char *ptr, const char *args, int nargs);
int radare_macro_call(const char *name);

#endif
