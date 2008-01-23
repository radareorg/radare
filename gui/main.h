#ifndef _INCLUDE_MAIN_H_
#define _INCLUDE_MAIN_H_

#include <gtk/gtk.h>
#include <vte/vte.h>

extern GtkWidget *term;
extern GtkWidget *tool;
extern GtkWidget *combo;
extern GtkWidget *entry;
extern GtkWidget *catact;
extern char *filename;
extern int is_debugger;

void gradare_new();
int execute_command(char *cmd);
GtkWidget *gradare_actions_new();
GtkWidget *gradare_toolbar_new();
GtkWidget *gradare_sidebar_new();
void prefs_open();
void gradare_open();
void gradare_open_program();
void gradare_open_process();
void gradare_new_monitor();
void gradare_help();
void gradare_refresh();
void toggle_toolbar();
#endif