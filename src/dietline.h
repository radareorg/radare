#ifndef _INCLUDE_DIETLINE_H_
#define _INCLUDE_DIETLINE_H_

#include <stdio.h>

#define DL_BUFSIZE 1024
#define DL_HISTSIZE 256
extern int dl_echo;
extern const char *dl_prompt;
extern const char *dl_clipboard;

extern char **dl_history;
extern int dl_histsize;
extern int dl_histidx;
extern int dl_autosave;
extern int dl_disable;


/* history */
extern char **dl_history;
extern int dl_histsize;
extern int dl_histidx;
extern int dl_autosave;
extern int dl_disable;

int dl_init();
int dl_hist_load(const char *file);
extern int dl_readchar();
extern int dl_hist_add(const char *line);
extern char *hist_get_i(int p);
extern void hist_add(char *str, int log);
extern void hist_clean();
int dl_hist_save(const char *file);
extern int hist_show();

extern char **(*dl_callback)(const char *text, int start, int end);

#endif
