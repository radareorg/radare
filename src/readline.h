#ifndef _INCLUDE_READLINE_H_
#define _INCLUDE_READLINE_H_

int iswhitespace(char ch);

#if HAVE_LIB_READLINE
#include <readline/history.h>
#include <readline/readline.h>

void rad_readline_init();
void rad_readline_finish();
#endif

#endif
