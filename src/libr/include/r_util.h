#ifndef _INCLUDE_UTIL_R_
#define _INCLUDE_UTIL_R_

/* TODO */
#define eprintf(x,y...) fprintf(stderr,x,##y)
#define strnull(x) (!x||!*x)

int strhash(const char *str);
char *strclean(char *str);
char *estrdup(char *ptr, const char *string);
void efree(void **ptr);

#endif
