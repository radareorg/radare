#ifndef _INCLUDE_FLAGS_H_
#define _INCLUDE_FLAGS_H_

#include "main.h"
#include "radare.h"
#include "list.h"
#define FLAG_BSIZE 40

extern char **environ;

/* radare flag */
typedef struct {
	char name[FLAG_BSIZE];
	u64 offset;
	u64 length;
	print_fmt_t format;
	unsigned char data[FLAG_BSIZE]; // only take a minor part of the data
	struct list_head list;
} flag_t;

void flag_list(char *arg);
void flag_array_clear(const char *name);
void flag_clear(const char *name);
void flag_clear_by_addr(u64 addr);
u64 flag_get_addr(const char *name);
flag_t *flag_get(const char *name);
flag_t *flag_get_i(int id);
flag_t *flag_get_next();
flag_t *flag_get_reset();
int flags_between(u64 from, u64 to);
int flag_is_empty(flag_t *flag);
char *flag_name_by_offset(u64 offset);
int flag_set(const char *name, u64 addr, int dup);
void print_flag_offset(u64 seek);
void flags_setenv();
void flag_list(char *arg);
void flag_help();
int flag_rename_str(char *text);

//extern flag_t **flags;
struct list_head flags;
//extern unsigned int eflags; // number of existent flags
//extern unsigned int nflags; // number of elements in flag array
#endif
