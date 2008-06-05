/* lua script editor in GTK */
/* TODO: make it work :) */

#include <gtk/gtk.h>
#include "plugin.h"

extern int radare_plugin_type;
extern struct plugin_hack_t radare_plugin;
static GtkWidget *my_widget = NULL;
static GtkButton *but = NULL;
static GtkHButtonBox *hbb = NULL;
static const char *filename = NULL;
static GtkWidget *text = NULL;
static GtkLabel *filename_w = NULL;
static int (*r)(const char *cmd, int log) = NULL;

static void execute()
{
	char buf[4096];
	//r = radare_plugin.resolve("radare_cmd");

	if (filename == NULL)
		return;
	sprintf(buf, "H lua %s", filename);
	//r(buf, 0);
}

static void execute_line()
{
	char buf[4096];
	//r = radare_plugin.resolve("radare_cmd");

	if (filename == NULL)
		return;
	sprintf(buf, "H lua %s", filename);
	//r(buf, 0);
}

static void luaedit_open()
{
	GtkWidget *fcd = gtk_file_chooser_dialog_new (
		"Select script...", NULL, // parent
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
		NULL);

	gtk_window_set_position( GTK_WINDOW(fcd), GTK_WIN_POS_CENTER);
	if ( gtk_dialog_run(GTK_DIALOG(fcd)) == GTK_RESPONSE_ACCEPT )
	{
		char *file= (char *)gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fcd));
		free(filename);
		filename = strdup(file);
		gtk_label_set_text(filename_w, filename);
	}
}

int my_hack(char *input)
{
	static int dry = 0;
	if (dry) return 0; dry=1;

	my_widget = gtk_vbox_new(FALSE, 3);
	/* filename:
          [  text area   ]
          open, save button, run button */

	filename_w = gtk_label_new("");
	gtk_box_pack_start(my_widget, filename_w, FALSE, FALSE, 0);
{
	text= gtk_text_view_new();
	gtk_container_add(my_widget, text);
}

	hbb = gtk_hbutton_box_new();

	but = gtk_button_new_from_stock("gtk-open");
	g_signal_connect(but, "released", G_CALLBACK(luaedit_open), NULL);
	gtk_box_pack_end(GTK_CONTAINER(hbb), but, FALSE, FALSE, 0);

	but = gtk_button_new_from_stock("gtk-save");
	g_signal_connect(but, "released", G_CALLBACK(luaedit_open), NULL);
	gtk_box_pack_end(GTK_CONTAINER(hbb), but, FALSE, FALSE, 0);

	but = gtk_button_new_from_stock("gtk-execute");
	g_signal_connect(but, "released", G_CALLBACK(luaedit_open), NULL);
	gtk_box_pack_end(GTK_CONTAINER(hbb), but, FALSE, FALSE, 0);

	gtk_box_pack_end(GTK_CONTAINER(my_widget), hbb, FALSE, FALSE, 0);

	return 0;
}

int radare_plugin_type = PLUGIN_TYPE_GUI;
struct plugin_hack_t radare_plugin = {
	.name = "luaedit",
	.desc = "LUA script editor in GTK",
	.callback = &my_hack,
	.widget = &my_widget
};
