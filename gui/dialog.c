/*
 * Copyright (C) 2007
 *       pancake <youterm.com>
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
#include <string.h>
#include <gtk/gtk.h>
#include <glib/glist.h>
#include <glib/glist.h>

int dialog_yes_no(const char *question, int two)
{
        GtkDialog *d = (GtkDialog *)gtk_dialog_new();
        int ret;

        gtk_window_set_title(GTK_WINDOW(d), "CanoeQuestion");
//      gtk_window_set_default_size(GTK_WINDOW(d), 300,140);
        gtk_window_set_position(GTK_WINDOW(d), GTK_WIN_POS_CENTER);
        if (two)
        gtk_dialog_add_button(d, "gtk-cancel", FALSE);
        gtk_dialog_add_button(d, "gtk-ok", TRUE);
        gtk_dialog_set_default_response(d, FALSE);
        gtk_window_set_modal(GTK_WINDOW(d),TRUE);
        // XXX no access to w pointer
        //gtk_window_set_transient_for(GTK_WINDOW(d), GTK_WINDOW(w->window));
        gtk_container_add(
                GTK_CONTAINER(d->vbox),
                gtk_label_new(question));
        gtk_widget_show_all(GTK_WIDGET(d));

        ret = gtk_dialog_run(d);
        gtk_widget_destroy(GTK_WIDGET(d));

        return ret;
}

int dialog_error(const char *msg)
{
	return dialog_yes_no(msg, 0);
}
