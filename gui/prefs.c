/*
 * Copyright (C) 2007
 *       pancake <pancake@phreaker.net>
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
#include <gtk/gtk.h>

GtkWidget *pw = NULL;
int pw_opened = 0;

//void prefs_close(void *widget, void *data, void *user)
gint prefs_close(void *item, GdkEvent *event, gpointer data)
{
	if (pw) {
		gtk_widget_hide(GTK_WIDGET(pw));
		gtk_widget_destroy(GTK_WIDGET(pw));
	}
	pw_opened = 0;

	if (data) {
		gradare_refresh();
	}
	return TRUE;
}

static GtkWidget *bytes;
static void bytes_changed(void *foo, void *data)
{
	if (gtk_toggle_button_get_active(bytes)) {
		vte_terminal_feed_child(VTE_TERMINAL(term), ":eval asm.bytes=1\n\n", 19);
	} else
		vte_terminal_feed_child(VTE_TERMINAL(term), ":eval asm.bytes=0\n\n", 19);
}

static GtkWidget *lines;
static void lines_changed(void *foo, void *data)
{
	if (gtk_toggle_button_get_active(data)) {
		vte_terminal_feed_child(VTE_TERMINAL(term), ":eval asm.lines=1\n\n", 19);
	} else
		vte_terminal_feed_child(VTE_TERMINAL(term), ":eval asm.lines=0\n\n", 19);
}

GtkWidget *prefs_disassembly()
{
	GtkWidget *offset;
	GtkWidget *hbbox;
	GtkVBox *vbox = gtk_vbox_new(FALSE, 5);
	
	bytes = gtk_check_button_new_with_label("Show bytes");
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(bytes), FALSE, FALSE, 2); 
 	g_signal_connect(GTK_COMBO_BOX(bytes), "clicked", GTK_SIGNAL_FUNC(bytes_changed), bytes);


	offset = gtk_check_button_new_with_label("Show offset");
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(offset), FALSE, FALSE, 2); 

	lines = gtk_check_button_new_with_label("Show lines");
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(lines), FALSE, FALSE, 2); 
 	g_signal_connect(GTK_COMBO_BOX(lines), "clicked", GTK_SIGNAL_FUNC(lines_changed), lines);
	
	
	return vbox;
}

void prefs_open()
{
	GtkWidget *vbox;
	GtkWidget *hbbox;
	GtkWidget *ok, *cancel;
	GtkWidget *nb;

	if (pw_opened) {
		puts("dry!");
		// focus the window
	//	gtk_widget_set_focus(GTK_WIDGET(pw));
		return;
	}

	pw_opened = 1;
	pw = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(pw), "gradare preferences");
	g_signal_connect(pw, "destroy", G_CALLBACK(prefs_close), 0);

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(pw), vbox);

	nb = gtk_notebook_new();
	gtk_notebook_append_page(nb, prefs_disassembly(), gtk_label_new("Disassembly"));
	gtk_notebook_append_page(nb, gtk_label_new("TODO"), gtk_label_new("Global"));
	gtk_notebook_append_page(nb, gtk_label_new("TODO"), gtk_label_new("Startup"));
	gtk_container_add(GTK_CONTAINER(vbox), nb);
	//gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("TODO O:)"), FALSE, FALSE, 5); 

	hbbox = gtk_hbutton_box_new();
	gtk_container_set_border_width(GTK_CONTAINER(hbbox), 5);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbbox), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(hbbox), 5);

	cancel = gtk_button_new_from_stock("gtk-cancel");
	g_signal_connect(cancel, "button-release-event",
		(gpointer)prefs_close, (gpointer)NULL);
	gtk_container_add(GTK_CONTAINER(hbbox), cancel);

	ok = gtk_button_new_from_stock("gtk-ok");
	g_signal_connect(ok, "button-release-event",
		(gpointer)prefs_close, (gpointer)1);
	gtk_container_add(GTK_CONTAINER(hbbox), ok);

	gtk_box_pack_end(GTK_BOX(vbox), GTK_WIDGET(hbbox), FALSE, FALSE, 5); 

	gtk_widget_show_all(pw);
}
