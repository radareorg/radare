#ifndef _INCLUDE_UTIL_R_
#define _INCLUDE_UTIL_R_

#include "r_types.h"

/* numbers */
struct r_num_t {
	u64 (*callback)(void *userptr, const char *str, int *ok);
	void *userptr;
};

u64 r_num_math(struct r_num_t *num, const char *str);
u64 r_num_get(struct r_num_t *num, const char *str);
struct r_num_t *r_num_new(u64 (*cb)(void*,const char *,int*), void *ptr);
int r_num_init(struct r_num_t *num);

/* strings */

/* TODO */
#define eprintf(x,y...) fprintf(stderr,x,##y)
#define strnull(x) (!x||!*x)
// XXX
#define iswhitechar(x) (x==' '||x=='\t'||x=='\n'||x=='\r')
#define iswhitespace(x) (x==' '||x=='\t')

int strhash(const char *str);
char *strclean(char *str);
char *estrdup(char *ptr, const char *string);
void efree(void **ptr);

/* stabilized */
int str_word_count(const char *string);
int str_word_set0(char *str);
const char *str_word_get0(const char *str, int idx);

char *str_clean(char *str);
int str_nstr(char *from, char *to, int size);
char *str_lchr(char *str, char chr);
int str_ccmp(char *dst, char *orig, int ch);
int str_ccpy(char *dst, char *orig, int ch);
char *str_slurp(const char *str);
const char *str_get(const char *str);
char *str_dup(char *ptr, const char *string);
void *str_free(void *ptr);
int str_inject(char *begin, char *end, char *str, int maxlen);

#endif
