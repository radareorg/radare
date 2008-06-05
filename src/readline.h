#ifndef _INCLUDE_READLINE_H_
#define _INCLUDE_READLINE_H_

#include "dietline.h"

int iswhitespace(char ch);
char **radare_dl_autocompletion(const char *text, int start, int end);

#if HAVE_LIB_READLINE
#include <readline/history.h>
#include <readline/readline.h>

void rad_readline_init();
void rad_readline_finish();
#endif

#endif
