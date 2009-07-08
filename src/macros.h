#ifndef _INCLUDE_MACROS_H_
#define _INCLUDE_MACROS_H_

#include "list.h"

#define MACRO_LIMIT 4096

struct macro_t {
	char *name;
	char *code;
	int nargs;
	struct list_head list;
};

#define MAX_LABELS 20
struct macro_label_t {
  char name[80];
  char *ptr;
};


extern int macro_counter;
extern ut64 *macro_break_value;

void radare_macro_init();
int radare_macro_add(const char *name);
int radare_macro_rm(const char *_name);
int radare_macro_list();
int radare_macro_call(const char *name);
int radare_macro_break(const char *value);

#endif
