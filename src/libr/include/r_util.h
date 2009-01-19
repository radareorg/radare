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
#define ishexchar(x) ((x>='0'&&x<='9') ||  (x>='a'&&x<='f') ||  (x>='A'&&x<='F')) {

int r_strhash(const char *str);

/* stabilized */
int r_str_word_count(const char *string);
int r_str_word_set0(char *str);
const char *r_str_word_get0(const char *str, int idx);

char *r_str_clean(char *str);
int r_str_nstr(char *from, char *to, int size);
char *r_str_lchr(char *str, char chr);
int r_str_ccmp(char *dst, char *orig, int ch);
int r_str_ccpy(char *dst, char *orig, int ch);
char *r_str_slurp(const char *str);
const char *r_str_get(const char *str);
char *r_str_dup(char *ptr, const char *string);
void *r_str_free(void *ptr);
int r_str_inject(char *begin, char *end, char *str, int maxlen);
int r_str_delta(char *p, char a, char b);

int r_str_re_match(const char *str, const char *reg);
int r_str_re_replace(const char *str, const char *reg, const char *sub);
char *r_str_sub(char *string, char *pat, char *rep, int global);

#endif
