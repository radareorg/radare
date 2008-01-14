#ifndef _INCLUDE_FLAGS_H_
#define _INCLUDE_FLAGS_H_

#include "radare.h"
#include "main.h"
#define FLAG_BSIZE 40

extern char **environ;

/* radare flag */
typedef struct {
	char name[FLAG_BSIZE];
	off_t offset;
	off_t length;
	print_fmt_t format;
	unsigned char data[FLAG_BSIZE]; // only take a minor part of the data
} rad_flag_t;

void flag_list(char *arg);
void flag_array_clear(const char *name);
void flag_clear(const char *name);
rad_flag_t *flag_get(const char *name);
rad_flag_t *flag_get_i(int id);
rad_flag_t *flag_get_next();
int flag_is_empty(rad_flag_t *flag);
char *flag_name_by_offset(off_t offset);
int flag_set(const char *name, off_t addr, int dup);
void print_flag_offset(off_t seek);
void flags_setenv();

extern rad_flag_t **flags;
extern unsigned int eflags; // number of existent flags
extern unsigned int nflags; // number of elements in flag array
#endif
