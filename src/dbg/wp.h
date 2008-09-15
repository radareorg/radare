#ifndef WP_H
#define WP_H
#include "parser.h"

typedef struct {
	struct tok *cond;
	char *str_cond;
	int refs;
} WP;

int debug_wp_match();
int debug_wp_add(const char *str);
int debug_wp_rm(int i);
void debug_wp_list();

#endif
