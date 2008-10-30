/*
 * Copyright (C) 2007, 2008
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

#include "plugin.h"
#define GRSCDIR "/usr/share/radare/gradare"

#include "main.h"
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

static int (*r)(const char *cmd, int log);
extern struct plugin_hack_t radare_plugin;
static GtkWidget *my_widget = NULL;
static GtkWidget *sc_cat; /* scrolled window */
static GtkWidget *mo_cat; /* model categories */
static GtkWidget *tv_cat; /* treeview categories */
static GtkWidget *sc_act;
static GtkWidget *mo_act;
static GtkWidget *tv_act;
static GtkCellRenderer *render_text;
static gchar *cat = NULL;

static void gradare_fill_model(GtkWidget *model, char *path)
{
	GDir *dir;
	gchar *name;
	GtkTreeIter iter;

	dir = g_dir_open (path, 0, NULL);
	if (!dir) return;

	name = (gchar *)g_dir_read_name (dir);
	while (name != NULL) {
		if (strcmp(name, "Shell")) {
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
			gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, name, -1);
		}
		name = (gchar *)g_dir_read_name (dir);
	}
}

static void execute_command(const char *buffer)
{
	printf("EXECUTE_COMMAND(%s)\n", buffer);
	r(buffer,0);
}
/*
void action_activated(
	GtkTreeView       *tree_view,
	GtkTreePath       *path,
	GtkTreeViewColumn *column,
	gpointer           user_data)
	*/
gchar *name = NULL;
static gint action_activated(void *item, GdkEvent *event, gpointer data)
{
	char buffer[1024];
	GtkTreeIter iter;
	GtkTreeSelection *sel;
	GtkTreeModel *model;

	// 0x100 is left click
	// 0x400 is right click
	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tv_act));
	if (gtk_tree_selection_count_selected_rows(sel )  == 1) {
		gtk_tree_selection_get_selected(GTK_TREE_SELECTION(sel), (GtkTreeModel **)&mo_act, &iter);
		g_free (name);
		gtk_tree_model_get (GTK_TREE_MODEL(mo_act), &iter, 0, &name, -1);

		if (event->button.state == 0x100) {
			sprintf(buffer, "sh "GRSCDIR"/%s/%s", cat, name);
			printf("exec: %s\n", buffer);
			execute_command(buffer);
		}
	}
}

static void action_category_activated(
	GtkTreeView       *tree_view,
	GtkTreePath       *path,
	GtkTreeViewColumn *column,
	gpointer           user_data)
{
	char buffer[1024];
	GtkTreeIter iter;
	GtkTreeSelection *sel;
	GtkTreeModel *model;
	gchar *name = NULL;

	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tv_cat));
	if (gtk_tree_selection_count_selected_rows(sel )  == 1) {
		gtk_tree_selection_get_selected(GTK_TREE_SELECTION(sel), (GtkTreeModel **)&mo_cat, &iter);
		gtk_tree_model_get(GTK_TREE_MODEL(mo_cat), &iter, 0, &name, -1);
		sprintf(buffer, GRSCDIR"/%s", name);

		gtk_list_store_clear(GTK_LIST_STORE(mo_act));
		gradare_fill_model(mo_act, buffer);
		g_free(cat);
		cat = name;
	}
}


/*************************************/
/*************************************/
/*************************************/
static GtkWidget *wt = NULL;
static GtkWidget *icon = NULL;
static GtkWidget *entry = NULL;
static int wt_opened = 0;

enum {
	PIXBUF_COL,
	TEXT_COL
};

static gchar *stock_id[39] = {
	GTK_STOCK_ABOUT,
	GTK_STOCK_GO_UP,
	GTK_STOCK_GO_DOWN,
	GTK_STOCK_GO_BACK,
	GTK_STOCK_GO_FORWARD,
	GTK_STOCK_FIND,
	GTK_STOCK_JUMP_TO,
	GTK_STOCK_MISSING_IMAGE,
	GTK_STOCK_FIND_AND_REPLACE,
	GTK_STOCK_HOME,
	GTK_STOCK_ADD,
	GTK_STOCK_NETWORK,
	GTK_STOCK_REMOVE,
	GTK_STOCK_APPLY,
	GTK_STOCK_PROPERTIES,
	GTK_STOCK_COLOR_PICKER,
	GTK_STOCK_FILE,
	GTK_STOCK_DIRECTORY,
	GTK_STOCK_EXECUTE,
	GTK_STOCK_CLEAR,
	GTK_STOCK_OPEN,
	GTK_STOCK_CLOSE,
	GTK_STOCK_STOP,
	GTK_STOCK_INFO,
	GTK_STOCK_GOTO_TOP,
	GTK_STOCK_COPY,
	GTK_STOCK_CUT,
	GTK_STOCK_PASTE,
	GTK_STOCK_CONVERT,
	GTK_STOCK_DELETE,
	GTK_STOCK_UNDELETE,
	GTK_STOCK_CDROM,
	GTK_STOCK_HARDDISK,
	GTK_STOCK_FLOPPY,
	GTK_STOCK_OK,
	GTK_STOCK_CANCEL,
	GTK_STOCK_MEDIA_PREVIOUS,
	GTK_STOCK_MEDIA_NEXT,
	GTK_STOCK_MEDIA_RECORD
};

static gchar * strip_underscore (const gchar *text)
{
	gchar *p, *q;
	gchar *result;

	result = g_strdup (text);
	p = q = result;
	while (*p) {
		if (*p != '_') { *q = *p; q++; }
		p++;
	}
	*q = '\0';

	return result;
}

static GtkTreeModel *create_stock_icon_store (void)
{
	GtkStockItem item;
	GdkPixbuf *pixbuf;
	GtkWidget *cellview;
	GtkTreeIter iter;
	GtkListStore *store;
	gchar *label;
	gint i;

	cellview = gtk_cell_view_new ();

	store = gtk_list_store_new (2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

	for (i = 0; i < G_N_ELEMENTS (stock_id); i++)
	{
		if (stock_id[i])
		{
			pixbuf = gtk_widget_render_icon (cellview, stock_id[i],
					GTK_ICON_SIZE_BUTTON, NULL);
			gtk_stock_lookup (stock_id[i], &item);
			label = strip_underscore (item.label);
			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter,
					PIXBUF_COL, pixbuf,
					TEXT_COL, label,
					-1);
			g_object_unref (pixbuf);
			g_free (label);
		} /* else {
			// TODO: why doesn't works? check against gtk-demo code
			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter,
					PIXBUF_COL, NULL,
					TEXT_COL, "separator",
					-1);
		}
		*/
	}

	gtk_widget_destroy (cellview);

	return GTK_TREE_MODEL (store);
}

/* A GtkTreeViewRowSeparatorFunc that demonstrates how rows can be
 * rendered as separators. This particular function does nothing 
 * useful and just turns the fourth row into a separator.
 */
static gboolean is_separator (GtkTreeModel *model, GtkTreeIter  *iter, gpointer data)
{
	GtkTreePath *path;
	gboolean result;

	path = gtk_tree_model_get_path (model, iter);
	result = gtk_tree_path_get_indices (path)[0] == 4;
	gtk_tree_path_free (path);

	return result;
}

/*
static void set_sensitive (
	GtkCellLayout   *cell_layout,
	GtkCellRenderer *cell,
	GtkTreeModel    *tree_model,
	GtkTreeIter     *iter,
	gpointer         data)
{
	GtkTreePath *path;
	gint *indices;
	gboolean sensitive;

	path = gtk_tree_model_get_path (tree_model, iter);
	indices = gtk_tree_path_get_indices (path);
	sensitive = indices[0] != 1;
	gtk_tree_path_free (path);

	g_object_set (cell, "sensitive", sensitive, NULL);
}
*/

static gint toolbar_noadd_close(void *item, GdkEvent *event, gpointer data)
{
	gtk_widget_destroy(wt);
	wt_opened = 0;
	//gradare_refresh();
	printf("TODO: refresh here\n");

	return 1;
}

static gint toolbar_add_close(void *item, GdkEvent *event, gpointer data)
{
	char *home = getenv("HOME");
	char *alias = NULL;
	char cmd[1024];
	int icon_id;

	gtk_widget_destroy(wt);
	wt_opened = 0;

	if (data == 0)
		return 1;

	if (((int)data) == -1)
		return 1;

	if (home == NULL) {
		fprintf(stderr, "No HOME?\n");
		return 1;
	}
	// XXX ugly hack joke
	if (strlen(home)>128) {
		fprintf(stderr, "Too large home\n");
		return 1;
	}

	alias = (char *)gtk_entry_get_text(GTK_ENTRY(entry));
	if (alias == NULL) // catch teh trap!
		return 1;

	sprintf(cmd, "ln -s '"GRSCDIR"/%s/%s' '%s/.radare/toolbar/%s'", cat, name, home, alias);
	system(cmd);

	icon_id = gtk_combo_box_get_active(GTK_COMBO_BOX(icon));
	sprintf(cmd, "echo '%s' > '%s/.radare/toolbar/.%s'", stock_id[icon_id], home, alias);
	system(cmd);

	//gradare_refresh();

	return 1;
}

static void toolbar_add()
{
	GtkWidget *vbox;
	GtkTreeModel *model;
	GtkWidget *hbox;
	  GtkCellRenderer *renderer;
	GtkWidget *hbbox;
	GtkWidget *ok, *cancel;

	if (wt_opened) {
		fprintf(stderr, "dry!\n");
		return;
	}
	if (name == NULL)
		return;

	wt_opened = 1;

	wt = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(wt), "Add icon to the toolbar");
	gtk_widget_show_all(wt);
	g_signal_connect(wt, "destroy", G_CALLBACK(toolbar_add_close), (gpointer)-1);

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(wt), vbox);

	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("~/.radare/toolbar"), FALSE, FALSE, 5);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("Icon:"), FALSE, FALSE, 5);

// COMBO
	model = create_stock_icon_store();
	icon = gtk_combo_box_new_with_model(model);
	renderer = gtk_cell_renderer_pixbuf_new ();
        gtk_cell_layout_pack_start(GTK_CELL_LAYOUT (icon), renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT (icon), renderer,
			"pixbuf", PIXBUF_COL, 
			NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT (icon), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT (icon), renderer,
			"text", TEXT_COL, NULL);

	gtk_combo_box_set_active (GTK_COMBO_BOX (icon), 0);
	//g_object_unref (model);
	gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 5);
// COMBO

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("Alias:"), FALSE, FALSE, 5);

	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), name);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 5);

	hbbox = gtk_hbutton_box_new();
	gtk_container_set_border_width(GTK_CONTAINER(hbbox), 5);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbbox), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(hbbox), 5);

	cancel = gtk_button_new_from_stock("gtk-cancel");
	g_signal_connect(cancel, "button-release-event",
		G_CALLBACK(toolbar_noadd_close), (gpointer)0);
	gtk_container_add(GTK_CONTAINER(hbbox), cancel);

	ok = gtk_button_new_from_stock("gtk-ok");
	g_signal_connect(ok, "button-release-event",
		G_CALLBACK(toolbar_add_close), (gpointer)1);
	gtk_container_add(GTK_CONTAINER(hbbox), ok);

	gtk_box_pack_end(GTK_BOX(vbox), hbbox, FALSE, FALSE, 5); 
	gtk_widget_show_all(wt);
}
/*************************************/
/*************************************/
/*************************************/

static GtkWidget *catact = NULL;

static GtkWidget *gradare_actions_new()
{
//	GtkWidget *exp =gtk_expander_new("");
	GtkWidget *vpan = gtk_vpaned_new();
	GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
	GtkWidget *exec_button;
	GtkWidget *add;
	GtkTreeViewColumn *col;

	render_text = gtk_cell_renderer_text_new();

	sc_cat = gtk_scrolled_window_new(NULL, (GtkAdjustment*)gtk_adjustment_new(0, 0, 100, 1, 10, 10));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sc_cat), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	tv_cat = gtk_tree_view_new();
	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(tv_cat), TRUE);
	//gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(tv_cat), TRUE);
	mo_cat = (GtkWidget *)gtk_list_store_new(1, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(tv_cat), GTK_TREE_MODEL(mo_cat));
	col = gtk_tree_view_column_new_with_attributes("Category", render_text, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tv_cat), col);
	g_signal_connect (tv_cat, "cursor-changed", G_CALLBACK(action_category_activated), NULL);
	gtk_container_add(GTK_CONTAINER(sc_cat), tv_cat);
	gtk_paned_pack1(GTK_PANED(vpan), sc_cat, TRUE, TRUE);
	catact = tv_cat;
	gradare_fill_model(mo_cat, GRSCDIR);

	////////////////

	sc_act = gtk_scrolled_window_new(NULL, (GtkAdjustment*)gtk_adjustment_new(0,0,100,1,10,10));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sc_act), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	tv_act = gtk_tree_view_new();
	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(tv_act), TRUE);
	//gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(tv_act), TRUE);
	mo_act = (GtkWidget *)gtk_list_store_new(1, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(tv_act), GTK_TREE_MODEL(mo_act));
	col = gtk_tree_view_column_new_with_attributes("Action", render_text, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tv_act), col);
	g_signal_connect (tv_act, "button-release-event", G_CALLBACK(action_activated), NULL);
	gtk_container_add(GTK_CONTAINER(sc_act), tv_act);
	gtk_paned_pack2(GTK_PANED(vpan), sc_act, TRUE, TRUE);
	cat = NULL;

	gtk_paned_set_position(GTK_PANED(vpan), 140);

	gtk_container_add(GTK_CONTAINER(vbox), vpan);

	add = gtk_button_new_from_stock("gtk-add");
	g_signal_connect(add, "button-release-event", toolbar_add, NULL);
	gtk_box_pack_end(GTK_BOX(vbox), GTK_WIDGET(add), FALSE, FALSE, 0);

	//return vbox;
	//gtk_container_add(exp,vbox);
	return vbox;
}

/*---*/

static int my_hack(char *input)
{
	static int dry = 0;

	if (dry) return 0; dry=1;

	my_widget = gradare_actions_new();
	r = radare_plugin.resolve("radare_cmd");
	if (r != NULL)
		return 1;

	return 0;
}

int radare_plugin_type = PLUGIN_TYPE_GUI;
struct plugin_hack_t radare_plugin = {
	.name = "gtk-actions",
	.desc = "GTK actions dialog",
	.callback = &my_hack,
	.widget = &my_widget
};
