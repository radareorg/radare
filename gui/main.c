/*
 * Copyright (C) 2007, 2008
 *       pancake <@youterm.com>
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

//#define FONT "-adobe-courier-bold-o-normal--18-180-75-75-m-110-iso8859-15"
#define FONT "Sans Bold 8"

int mon_id = 0;
static char *project_file = NULL;

GtkWidget *term = NULL;
char *filename = NULL;
char *command = NULL;
int is_debugger = 0;
char *font = FONT;

#if _MAEMO_
HildonWindow *w = NULL;
HildonProgram *p = NULL;
#else
GtkWindow *w = NULL;
#endif

// TODO: autodetect project files (rdb)

void show_help_message()
{
	printf("Usage: gradare [-h] [-e command] [-f font] [[-d pid|prg] | file]\n");
	exit(1);
}

GtkWidget *tool;

void init_home_directory()
{
	char buf[4096];
	strcpy(buf, g_get_home_dir());
	strcat(buf, "/.radare/");
	mkdir(buf);
	strcat(buf, "toolbar/");
	mkdir(buf);
}

GtkWidget *acti;
GtkWidget *menu;

GtkClipboard *clip = NULL;

void seek_to()
{
	gchar *str;

	clip = gtk_widget_get_clipboard(term, 1);
	vte_terminal_copy_clipboard(term);
	str = gtk_clipboard_wait_for_text(clip);
	vte_terminal_feed_child(VTE_TERMINAL(term), ":s ", 3);
	vte_terminal_feed_child(VTE_TERMINAL(term), str, strlen(str));
	vte_terminal_feed_child(VTE_TERMINAL(term), "\n\n", 2);
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(combo)) == 1)
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 2);

	gtk_widget_destroy(menu);
}

void copy_to()
{
	gchar *str;

	clip = gtk_widget_get_clipboard(term, 1);
	vte_terminal_copy_clipboard(term);
	gtk_widget_destroy(menu);
}

void breakpoint_drop()
{
	gchar *str;

	clip = gtk_widget_get_clipboard(term, 1);
	vte_terminal_copy_clipboard(term);
	str = gtk_clipboard_wait_for_text(clip);
	vte_terminal_feed_child(VTE_TERMINAL(term), ":!bp -", 6);
	vte_terminal_feed_child(VTE_TERMINAL(term), str, strlen(str));
	vte_terminal_feed_child(VTE_TERMINAL(term), "\n\n", 2);
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(combo)) == 1)
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 2);

	gtk_widget_destroy(menu);
}

void breakpoint_to()
{
	gchar *str;

	clip = gtk_widget_get_clipboard(term, 1);
	vte_terminal_copy_clipboard(term);
	str = gtk_clipboard_wait_for_text(clip);
	vte_terminal_feed_child(VTE_TERMINAL(term), ":!bp ", 5);
	vte_terminal_feed_child(VTE_TERMINAL(term), str, strlen(str));
	vte_terminal_feed_child(VTE_TERMINAL(term), "\n\n", 2);
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(combo)) == 1)
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 2);

	gtk_widget_destroy(menu);
}

void continue_until_here()
{
	gchar *str;

	clip = gtk_widget_get_clipboard(term, 1);
	vte_terminal_copy_clipboard(term);
	str = gtk_clipboard_wait_for_text(clip);
	if (str && str[0]) {
		vte_terminal_feed_child(VTE_TERMINAL(term), ":!cont ", 7);
		vte_terminal_feed_child(VTE_TERMINAL(term), str, strlen(str));
		vte_terminal_feed_child(VTE_TERMINAL(term), "\n\n", 2);
		if (gtk_combo_box_get_active(GTK_COMBO_BOX(combo)) == 1)
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 2);
	}

	gtk_widget_destroy(menu);
}

gboolean popup_context_menu(GtkWidget *tv, GdkEventButton *event, gpointer user_data)
{
	GtkWidget *menu_item;

	if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
		menu = gtk_menu_new();

		gtk_container_set_border_width(menu, 2);

		menu_item = gtk_image_menu_item_new_from_stock("Seek to...", "gtk-find");
		g_signal_connect(menu_item, "button-release-event", G_CALLBACK(seek_to), NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

		menu_item = gtk_image_menu_item_new_from_stock("Copy to clipboard", "gtk-copy");
		g_signal_connect(menu_item, "button-release-event", G_CALLBACK(copy_to), NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

		gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	/* TODO: make these menus optional depending on the mode */
		menu_item = gtk_image_menu_item_new_from_stock("Add breakpoint", "gtk-add");
		g_signal_connect(menu_item, "button-release-event", G_CALLBACK(breakpoint_to), NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

		menu_item = gtk_image_menu_item_new_from_stock("Remove breakpoint", "gtk-remove");
		g_signal_connect(menu_item, "button-release-event", G_CALLBACK(breakpoint_drop), NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

		gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

		menu_item = gtk_image_menu_item_new_from_stock("Continue until here", "gtk-next");
		g_signal_connect(menu_item, "button-release-event", G_CALLBACK(continue_until_here), NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);


		gtk_widget_show_all(menu);
		gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, NULL);
		return 1;
	}

	return 0;
}

struct gradare_mon_t {
	int id;
	GtkWidget *entry;
	GtkWidget *but;
	GtkWidget *term;
};

gboolean monitor_button_clicked(GtkWidget *but, gpointer user_data)
{
	struct gradare_mon_t *mon = user_data;
	char miau[16];
	char buf[1024];
	char *cmd[4]={"/usr/bin/rsc","monitor", &miau, 0};
	sprintf(miau, "mon%d", mon->id);

	snprintf(buf, 1023, "/usr/bin/rsc monitor %s \"%s\"", miau, gtk_entry_get_text(mon->entry));
	system(buf);

	vte_terminal_fork_command(
			VTE_TERMINAL(mon->term),
			cmd[0], cmd, NULL, ".",
			FALSE, FALSE, FALSE);

	return 0;
}

void gradare_save_project_as()
{
	char buf[1024];
        GtkWidget *fcd;

        fcd = gtk_file_chooser_dialog_new (
                "Save project as...", NULL, // parent
                GTK_FILE_CHOOSER_ACTION_SAVE,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                NULL);

        gtk_window_set_position( GTK_WINDOW(fcd), GTK_WIN_POS_CENTER);
        if ( gtk_dialog_run(GTK_DIALOG(fcd)) == GTK_RESPONSE_ACCEPT ) {
                const char *filename = (char *)gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fcd));
		free(project_file);
		project_file = strdup(filename);
		gradare_save_project();
	}
	gtk_widget_destroy(fcd);
}

void gradare_save_project()
{
	char buf[1024];
	if (project_file) {
		snprintf(buf, 4095, ":Ps %s\n\n", project_file);
		vte_terminal_feed_child(VTE_TERMINAL(term), buf, strlen(buf));
#if _MAEMO_
		hildon_banner_show_information(GTK_WIDGET(w), NULL, "Project saved...");
#endif
	} else
		gradare_save_project_as();
	
}

void gradare_open_project()
{
	char buf[1024];
        GtkWidget *fcd;

        fcd = gtk_file_chooser_dialog_new (
                "Open project file...", NULL, // parent
                GTK_FILE_CHOOSER_ACTION_OPEN,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                NULL);

        gtk_window_set_position( GTK_WINDOW(fcd), GTK_WIN_POS_CENTER);
        if ( gtk_dialog_run(GTK_DIALOG(fcd)) == GTK_RESPONSE_ACCEPT )
        {
                char cmd[4096];
                const char *filename = (const char *)gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fcd));

		sprintf(buf, ":Po %s\n\n", filename);
		vte_terminal_feed_child(VTE_TERMINAL(term), buf, strlen(buf));
	}

	gtk_widget_destroy(fcd);
}

void gradare_new_graph()
{
	vte_terminal_feed_child(VTE_TERMINAL(term), ":ag\n\n", 5);
}

void gradare_new_monitor()
{
	GtkWindow *w;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *hpan;
	struct gradare_mon_t *mon;
	mon = (struct gradare_mon_t *)malloc(sizeof(struct gradare_mon_t));
	mon->id = mon_id++;

	w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	w->allow_shrink=TRUE;
#if _MAEMO_
	hildon_program_add_window(p, w);
#endif
	gtk_window_resize(GTK_WINDOW(w), 600,400);
	gtk_window_set_title(GTK_WINDOW(w), "gradare monitor");
	// XXX memleak !!! should be passed to a destroyer function for 'mon'
	g_signal_connect(w, "destroy", G_CALLBACK(gtk_widget_hide), mon);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(w), vbox);

	hbox = gtk_hbox_new(FALSE, 3);
	mon->entry = gtk_entry_new();
	mon->but = gtk_button_new_with_label("Go");
	// mouse
	g_signal_connect(mon->but, "released", (gpointer)monitor_button_clicked, (gpointer)mon);
	// keyboard
	g_signal_connect(mon->but, "activate", (gpointer)monitor_button_clicked, (gpointer)mon);
	g_signal_connect(mon->entry, "activate", (gpointer)monitor_button_clicked, (gpointer)mon);
	gtk_container_add(GTK_CONTAINER(hbox), mon->entry);
	gtk_box_pack_start(GTK_CONTAINER(hbox), mon->but, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_CONTAINER(vbox), hbox, FALSE, FALSE, 5);

	mon->term = vte_terminal_new();
	//vte_terminal_set_background_transparent(term, TRUE);
	//vte_terminal_set_opacity(term, 10);
	//vte_terminal_set_color_background(term, (GdkColor));
	//vte_terminal_set_color_foreground(term, (GdkColor));
	vte_terminal_set_cursor_blinks((VteTerminal*)mon->term, TRUE);
	vte_terminal_set_mouse_autohide((VteTerminal*)mon->term, TRUE);
	vte_terminal_set_scrollback_lines((VteTerminal*)mon->term, 3000);
	vte_terminal_set_font_from_string_full((VteTerminal*)mon->term, font, VTE_ANTI_ALIAS_FORCE_DISABLE);
	//(VTE_TERMINAL_CLASS(term))->increase_font_size(term);
	g_signal_connect (mon->term, "button-press-event", G_CALLBACK (popup_context_menu), NULL);

	/*
	   vte_terminal_fork_command(
	   VTE_TERMINAL(term),
	   cmd[0], cmd, NULL, ".",
	   FALSE, FALSE, FALSE);

*/
	gtk_container_add(GTK_CONTAINER(vbox), mon->term);
	gtk_widget_show_all(GTK_WIDGET(w));
}

static int fontsize=8;
static int fontalias=1;
static int fontbold=0;
static int console_font_size(int newsize)
{
	char buf[128];
	if (newsize<4)
		newsize = 4;
	if (newsize>72)
		newsize = 72;

	sprintf(buf, "Sans %s%d", fontbold?"bold ":"", fontsize);
	vte_terminal_set_font_from_string_full((VteTerminal*)term, buf, fontalias?VTE_ANTI_ALIAS_FORCE_DISABLE:0);
	return newsize;
}


static int fs = 0;
void toggle_fullscreen()
{
	if (fs) gtk_window_unfullscreen(w);
	else gtk_window_fullscreen(w);
	fs^=1;
}

/* Callback for hardware keys */
gboolean key_press_cb(GtkWidget * widget, GdkEventKey * event,
		GtkWindow * window)
{
	switch (event->keyval) {
#if 0
		case GDK_Up:
			//        hildon_banner_show_information(GTK_WIDGET(window), NULL, "Navigation Key Up");
			vte_terminal_feed_child(VTE_TERMINAL(term), "k", 1);
			return TRUE;

		case GDK_Down:
			//       hildon_banner_show_information(GTK_WIDGET(window), NULL, "Navigation Key Down");
			vte_terminal_feed_child(VTE_TERMINAL(term), "j", 1);
			return TRUE;

		case GDK_Left:
			//       hildon_banner_show_information(GTK_WIDGET(window), NULL, "Navigation Key Left");
			vte_terminal_feed_child(VTE_TERMINAL(term), "h", 1);
			return TRUE;

		case GDK_Right:
			//hildon_banner_show_information(GTK_WIDGET(window), NULL, "Navigation Key Right");
			vte_terminal_feed_child(VTE_TERMINAL(term), "l", 1);
			return TRUE;

		case GDK_Return:
			//hildon_banner_show_information(GTK_WIDGET(window), NULL, "Navigation Key select");
			return TRUE;
#endif

		case GDK_F6:
			//      hildon_banner_show_information(GTK_WIDGET(window), NULL, "Full screen");
			toggle_fullscreen();
			return TRUE;

			// TODO: FONT SIZE HERE!
#if _MAEMO_
		case GDK_F7:
#else
		case GDK_KP_Add:
#endif
			//hildon_banner_show_information(GTK_WIDGET(window), NULL, "Increase (zoom in)");
			fontsize = console_font_size(++fontsize);
			return TRUE;

		case GDK_F5:
			gradare_refresh();
			return TRUE;
#if _MAEMO_
		case GDK_F8:
#else
		case GDK_KP_Subtract:
#endif
			//hildon_banner_show_information(GTK_WIDGET(window), NULL, "Decrease (zoom out)");
			fontsize = console_font_size(--fontsize);
			return TRUE;
			//#endif

		case GDK_Escape:
			//hildon_banner_show_information(GTK_WIDGET(window), NULL, "Cancel/Close");
			//gtk_window_unfullscreen(window);
			gtk_widget_grab_focus(term);
			return TRUE;
	}

	return FALSE;
}

int main(int argc, const char **argv, char **envp)
{
	int c;
	char str[1024];
	GtkWidget *chos;
	GtkWidget *vbox;
	GtkWidget *hpan;

	while ((c = getopt(argc, argv, "e:hf:d")) != -1) {
		switch( c ) {
			case 'd': {
					  // XXX : overflowable, must use strcatdup or stgh like that
					  int pid = atoi(argv[optind]);
					  char buf[4096];
					  char buf2[4096];
					  buf[0]='\0';

					  /* by process-id */
					  if (pid > 0) {
						  sprintf(buf2, "pid://%d", pid);
						  //plugin_load();
						  //return radare_go();
						  filename = strdup(buf2);
					  } else {
						  /* by program path */
						  for(c=optind;argv[c];c++) {
							  strcat(buf, argv[c]);
							  if (argv[c+1])
								  strcat(buf, " ");
						  }
						  sprintf(buf2, "dbg://%s", buf);
						  filename = strdup(buf2);
					  }}
					  break;
			case 'h':
					  show_help_message();
					  break;
			case 'e':
					  command = optarg;
					  break;
			case 'f':
					  font = optarg;
					  break;
		}
	}

	if (filename == NULL && optind<argc)
		filename = argv[optind];

	gtk_init(&argc, &argv);
	init_home_directory();

	g_set_application_name("gradare");
#if _MAEMO_
	p = HILDON_PROGRAM(hildon_program_get_instance());
	w = HILDON_WINDOW(hildon_window_new());
	hildon_program_add_window(p, w);
#else
	w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	w->allow_shrink=TRUE;
#endif
	g_signal_connect(G_OBJECT(w), "key_press_event", G_CALLBACK(key_press_cb), w);

	gtk_window_resize(GTK_WINDOW(w), 800,600);
	gtk_window_set_title(GTK_WINDOW(w), "gradare");
	g_signal_connect(w, "destroy", G_CALLBACK(gtk_main_quit), 0);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(w), vbox);
#if _MAEMO_
	{
		GtkMenu *menu = GTK_MENU(gradare_menubar_hildon_new(w));
		hildon_window_set_menu(HILDON_WINDOW(w), menu);
		hildon_program_set_common_menu(HILDON_PROGRAM(p), menu);
	}
#else
	gtk_box_pack_start(GTK_BOX(vbox),
			GTK_WIDGET(gradare_menubar_new(w)), FALSE, FALSE, 0);
#endif

	tool = gradare_toolbar_new(NULL);
#if _MAEMO_
	hildon_window_add_toolbar(HILDON_WINDOW(w), GTK_TOOLBAR(tool));
	//	hildon_program_set_common_toolbar(p, tool);
#else
	gtk_box_pack_start(GTK_BOX(vbox), tool, FALSE, FALSE, 0);
#endif
	chos = gradare_topbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), chos, FALSE, FALSE, 0);

	//hpan = gtk_hpaned_new();
	hpan = gtk_hbox_new(FALSE, 3);
	gtk_container_add(GTK_CONTAINER(vbox), hpan);

	acti = gradare_actions_new();
	//gtk_paned_pack1(GTK_PANED(hpan), acti, TRUE, TRUE);
	gtk_box_pack_start(GTK_HBOX(hpan), acti, TRUE, TRUE,0);

	term = vte_terminal_new();
	//vte_terminal_set_background_transparent(term, TRUE);
	//vte_terminal_set_opacity(term, 10);
	//vte_terminal_set_color_background(term, (GdkColor));
	//vte_terminal_set_color_foreground(term, (GdkColor));
	vte_terminal_set_cursor_blinks((VteTerminal*)term, TRUE);
	vte_terminal_set_mouse_autohide((VteTerminal*)term, TRUE);
	vte_terminal_set_scrollback_lines((VteTerminal*)term, 3000);
	vte_terminal_set_font_from_string_full((VteTerminal*)term, font, VTE_ANTI_ALIAS_FORCE_DISABLE);
	//(VTE_TERMINAL_CLASS(term))->increase_font_size(term);
	g_signal_connect (term, "button-press-event",
			G_CALLBACK (popup_context_menu), NULL);


	//gtk_paned_pack2(GTK_PANED(hpan), term, TRUE, TRUE);
	gtk_container_add(GTK_HBOX(hpan),term); //, term, TRUE, TRUE);
	gtk_widget_show_all(GTK_WIDGET(w));

	setenv("BEP", "entry", 1); // force debugger to stop at entry point
	if (command) {
		char *arg[2] = { command, NULL};
		vte_terminal_fork_command(
				VTE_TERMINAL(term),
				command, arg, envp, ".",
				FALSE, FALSE, FALSE);
	} else {
		if (filename) {
#if 0
			char *arg[6] = { "/usr/bin/radare", "-escr.color=true", "-b", "1024", filename, NULL};

			vte_terminal_fork_command(
					VTE_TERMINAL(term),
					arg[0], arg, envp, ".",
					FALSE, FALSE, FALSE);
			vte_terminal_feed_child(VTE_TERMINAL(term), "V\n", 2);
			sprintf(str, "radare -c -b 1024 - %s", filename);
#endif
			char *arg[6] = { GRSCDIR"/Shell", filename, NULL};
			sprintf(str, "gradare: %s", filename);
			gtk_window_set_title(GTK_WINDOW(w), str);
			vte_terminal_fork_command(
					VTE_TERMINAL(term),
					GRSCDIR"/Shell", arg, envp, ".",
					FALSE, FALSE, FALSE);
		} else {
			sprintf(str, "gradare: (no file)");
			gtk_window_set_title(GTK_WINDOW(w), str);
			vte_terminal_fork_command(
					VTE_TERMINAL(term),
					GRSCDIR"/Shell", NULL, envp, ".",
					FALSE, FALSE, FALSE);
		}
	}

	if (filename && (strstr(filename, "dbg://") || strstr(filename, "pid://"))) {
		is_debugger = 1;
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 1);
	}

	gtk_widget_show_all(GTK_WIDGET(w));
	gtk_widget_grab_focus(term);

#if 0
	printf("Terminal size: %dx%d\n",
			vte_terminal_get_char_width(term),
			vte_terminal_get_char_height(term));
#endif

	gtk_main();
}
