#ifndef PARSER_H
#define PARSER_H
#include "../list.h"

struct regs_off {
	char *reg;
	int  off;
};

/* token types */
enum {
	GROUP_TOK = 0,
	REG_TOK,
	MEM_TOK
};

/* token structure */
struct tok {
	unsigned long off;
	int type;
	int op, log_op;
	char *val;
	int len;

	struct list_head list;
	struct list_head next;
};

/* arithmetical operations */
enum {
	OP_LT = 1,
	OP_LE,
	OP_EQ,
	OP_NE,
	OP_GE,
	OP_GT
};

/* logical operations */
enum {
	LOG_OR = 1,
	LOG_AND
}; 

char skip_chars(char **c);
inline struct tok* parse_cond(char *cond);
int eval_cond(struct tok *group);
void free_cond(struct tok *group);


#endif
