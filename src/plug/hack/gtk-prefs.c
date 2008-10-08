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
#include <gtk/gtk.h>

static GtkWidget *pw = NULL;
static int pw_opened = 0;

#include <gtk/gtk.h>
#include "plugin.h"

extern int radare_plugin_type;
extern struct plugin_hack_t radare_plugin;
static GtkWidget *my_widget = NULL;

// TODO.add a column to specify if toggle or string one
static struct {
	int panel_id;
	const char *label;
	const char *name;
	int value;
} toggles[22] = {
	{ 0, "Show bytes",  "asm.bytes",  1 },
	{ 0, "Show offset", "asm.offset", 1 },
	{ 0, "Show lines",  "asm.lines",  1 },
	{ 0, "Split by rets",  "asm.split",  1 },
	{ 0, "Split all blocks",  "asm.splitall",  0 },
	{ 0, "Show relative address",  "asm.reladdr",  1 },
	{ 0, "Lines out of block",  "asm.linesout",  0 },
	{ 0, "Show comments",  "asm.comments",  1 },

	{ 1, "Endian (set = big, unset little)",  "cfg.bigendian",  0 },
	{ 1, "Color",  "scr.color",  1 },
	{ 1, "Buffered output",  "scr.buf",  0 },
	{ 1, "Inverse block",  "cfg.inverse",  0 },

	{ 2, "Split code blocks",  "graph.split",  0 },
	{ 2, "Split blocks by calls",  "graph.callblocks",  0 },
	{ 2, "Split blocks by jumps",  "graph.jmpblocks",  1 },
	{ 2, "Show code offsets",  "graph.offset",  0 },

	{ 3, "Load symbols",  "dbg.syms",  1 },
	{ 3, "Use DWARF",  "dbg.dwarf",  1 },
	{ 3, "Handle memory maps",  "dbg.maps",  1 },
	{ 3, "Show backtrace in !contsc",  "dbg.contscbt",  1 },
	{ 3, "Show complete backtrace",  "dbg.fullbt",  0 },
	NULL
};

//void prefs_close(void *widget, void *data, void *user)
static gint prefs_close(void *item, GdkEvent *event, gpointer data)
{
	if (pw) {
		gtk_widget_hide(GTK_WIDGET(pw));
		gtk_widget_destroy(GTK_WIDGET(pw));
	}
	pw_opened = 0;

	return TRUE;
}

	int (*r)(const char *cmd, int log);
static GtkWidget *lines;
static void toggle_changed(void *foo, void *data)
{
	char buf[1024];
	int i = (int)data;

	toggles[i].value = gtk_toggle_button_get_active(foo);

	sprintf(buf, ":eval %s=%d", toggles[i].name, toggles[i].value);
	//vte_terminal_feed_child(VTE_TERMINAL(term), buf, strlen(buf));
	r(buf, 0);
	
}

static GtkWidget *draw_toggles_for(int panel_id)
{
	GtkWidget *bytes;
	GtkWidget *hbbox;
	GtkVBox *vbox = gtk_vbox_new(FALSE, 5);
	int i;
	
	for(i=0;toggles[i].label!=NULL;i++) {
		if (toggles[i].panel_id != panel_id)
			continue;
		bytes = gtk_check_button_new_with_label(toggles[i].label);
		gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(bytes), FALSE, FALSE, 2); 
		gtk_toggle_button_set_active(bytes, toggles[i].value);
		g_signal_connect(GTK_COMBO_BOX(bytes), "clicked", GTK_SIGNAL_FUNC(toggle_changed), i);
	}

	return vbox;
}

static GtkWidget *prefs_open()
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
#if 0
	pw = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(pw), "gradare preferences");
	g_signal_connect(pw, "destroy", G_CALLBACK(prefs_close), 0);
#endif

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(pw), vbox);

	nb = gtk_notebook_new();
	gtk_notebook_append_page(nb, draw_toggles_for(0), gtk_label_new("Disassembly"));
	gtk_notebook_append_page(nb, draw_toggles_for(1), gtk_label_new("Global"));
	gtk_notebook_append_page(nb, draw_toggles_for(2), gtk_label_new("Graphs"));
	gtk_notebook_append_page(nb, draw_toggles_for(3), gtk_label_new("Debugger"));
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

	return vbox;

//	gtk_widget_show_all(pw);
}
/*---*/

static int my_hack(char *input)
{
	static int dry = 0;

	my_widget = prefs_open();
	r = radare_plugin.resolve("radare_cmd");
	if (r != NULL)
		return 1;

	if (dry) return 0; dry=1;

	return 0;
}

int radare_plugin_type = PLUGIN_TYPE_GUI;
struct plugin_hack_t radare_plugin = {
	.name = "gtk-prefs",
	.desc = "GTK preferences menu",
	.callback = &my_hack,
	.widget = &my_widget
};
