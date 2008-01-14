#ifndef WP_H
#define WP_H
#include "parser.h"

typedef struct {
	struct tok *cond;
	char *str_cond;
	int refs;
}WP;

#endif
